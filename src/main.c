/*
 * Air Quality Monitor — main.c
 *
 * Stage 3: sensor reading moved to a dedicated thread so the LVGL
 * timer handler is never stalled by the SGP30's 12 ms I2C delay.
 *
 * Threading model
 * ───────────────
 *  sensor_thread  (priority 5)
 *      Owns the SGP30 handle.  Reads eCO2/TVOC every SENSOR_INTERVAL_MS
 *      and pushes a sensor_reading into sensor_msgq.  Makes NO LVGL calls.
 *
 *  main thread  (priority 0)
 *      Runs lv_timer_handler() every LVGL_INTERVAL_MS, drains sensor_msgq,
 *      and calls ui_update().  All LVGL access is confined to this thread.
 *
 * Touch debug
 * ───────────
 *  Set TOUCH_DEBUG to 1 to log every press event coordinates to the
 *  console.  Use this to verify / tune invert-x, invert-y, swap-xy
 *  in the overlay's lvgl_pointer node, then set it back to 0.
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/display.h>
#include <zephyr/logging/log.h>
#include <lvgl.h>
#include <stdio.h>

#include "sgp30.h"
#include "ui/ui.h"
#include "ui/screens.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

extern int lvgl_init(void);

/* ── Hardware ────────────────────────────────────────────── */

static const struct gpio_dt_spec backlight =
    GPIO_DT_SPEC_GET(DT_NODELABEL(bl), gpios);

/* ── Timing ──────────────────────────────────────────────── */

#define SENSOR_INTERVAL_MS 1000
#define LVGL_INTERVAL_MS 5

/* ── Sensor ranges ───────────────────────────────────────── */

#define ECO2_MIN 400
#define ECO2_MAX 2000
#define TVOC_MIN 0
#define TVOC_MAX 500

/* ── Chart ───────────────────────────────────────────────── */

#define CHART_POINTS 60 /* 1 point/s → 60 s history */

static lv_chart_series_t *chart_ser_eco2;
static lv_chart_series_t *chart_ser_tvoc;

/* ── Touch debug ─────────────────────────────────────────── */

#define TOUCH_DEBUG 0 /* set to 1 to log touch coordinates */

/* ── Inter-thread communication ──────────────────────────── */

struct sensor_reading
{
  uint16_t eco2;
  uint16_t tvoc;
};

/* 4-slot queue: sensor writes at 1 Hz, main drains at ≥200 Hz */
K_MSGQ_DEFINE(sensor_msgq, sizeof(struct sensor_reading), 4, 4);

/* ── Air-quality classification ──────────────────────────── */

static const char *quality_label(uint16_t eco2)
{
  if (eco2 < 800)
    return "GOOD";
  if (eco2 < 1200)
    return "MODERATE";
  if (eco2 < 1600)
    return "POOR";
  return "DANGER";
}

static lv_color_t quality_color(uint16_t eco2)
{
  if (eco2 < 800)
    return lv_color_hex(0x00C800); /* green      */
  if (eco2 < 1200)
    return lv_color_hex(0xFFA000); /* orange     */
  if (eco2 < 1600)
    return lv_color_hex(0xFF4000); /* red        */
  return lv_color_hex(0xFF0000);   /* bright red */
}

/* ── UI helpers ──────────────────────────────────────────── */

static void ui_setup_chart(void)
{
  lv_obj_t *chart = objects.chart_history;

  lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
  lv_chart_set_point_count(chart, CHART_POINTS);
  lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, ECO2_MIN, ECO2_MAX);
  lv_chart_set_range(chart, LV_CHART_AXIS_SECONDARY_Y, TVOC_MIN, TVOC_MAX);
  lv_obj_set_style_size(chart, 0, 0, LV_PART_INDICATOR); /* hide dots */

  chart_ser_eco2 = lv_chart_add_series(chart,
                                       lv_color_hex(0x00BFFF),
                                       LV_CHART_AXIS_PRIMARY_Y);
  chart_ser_tvoc = lv_chart_add_series(chart,
                                       lv_color_hex(0xFFA500),
                                       LV_CHART_AXIS_SECONDARY_Y);

  /* Pre-fill with NONE so the line only draws once real data arrives */
  lv_chart_set_all_value(chart, chart_ser_eco2, LV_CHART_POINT_NONE);
  lv_chart_set_all_value(chart, chart_ser_tvoc, LV_CHART_POINT_NONE);
}

static void ui_update(uint16_t eco2, uint16_t tvoc, int64_t uptime_s)
{
  char buf[32];

  /* eCO2 */
  snprintf(buf, sizeof(buf), "%u", eco2);
  lv_label_set_text(objects.label_eco2_value, buf);
  lv_label_set_text(objects.label_eco2_unit, "ppm");
  lv_bar_set_range(objects.bar_eco2, ECO2_MIN, ECO2_MAX);
  lv_bar_set_value(objects.bar_eco2, eco2, LV_ANIM_ON);

  /* TVOC */
  snprintf(buf, sizeof(buf), "%u", tvoc);
  lv_label_set_text(objects.label_tvoc_value, buf);
  lv_label_set_text(objects.label_tvoc_unit, "ppb");
  lv_bar_set_range(objects.bar_tvoc, TVOC_MIN, TVOC_MAX);
  lv_bar_set_value(objects.bar_tvoc, tvoc, LV_ANIM_ON);

  /* Status */
  lv_label_set_text(objects.label_status, quality_label(eco2));
  lv_obj_set_style_text_color(objects.label_status,
                              quality_color(eco2), LV_PART_MAIN);

  /* Chart */
  lv_chart_set_next_value(objects.chart_history, chart_ser_eco2, eco2);
  lv_chart_set_next_value(objects.chart_history, chart_ser_tvoc, tvoc);
  lv_chart_refresh(objects.chart_history);

  /* Uptime */
  int h = (int)(uptime_s / 3600);
  int m = (int)((uptime_s % 3600) / 60);
  int s = (int)(uptime_s % 60);
  snprintf(buf, sizeof(buf), "%02d:%02d:%02d", h, m, s);
  lv_label_set_text(objects.label_uptime, buf);
}

/* ── Touch coordinate logger (only compiled when TOUCH_DEBUG=1) ── */

#if TOUCH_DEBUG
/*
 * Walk LVGL's indev chain looking for the pointer device registered
 * by the Zephyr LVGL glue (CONFIG_LV_Z_POINTER_INPUT).  Log its
 * coordinates every time it is pressed.
 *
 * Use the output to tune the overlay's lvgl_pointer node:
 *
 *   Symptom                       Fix in overlay
 *   ─────────────────────────────────────────────
 *   x/y swapped                   add swap-xy;
 *   x inverted (mirror horizontal) add invert-x;
 *   y inverted (mirror vertical)   add/remove invert-y;
 *
 * Remove or set TOUCH_DEBUG 0 once calibration is confirmed.
 */
static void touch_debug_log(void)
{
  lv_indev_t *indev = lv_indev_get_next(NULL);
  while (indev != NULL)
  {
    if (lv_indev_get_type(indev) == LV_INDEV_TYPE_POINTER)
    {
      lv_point_t p;
      lv_indev_get_point(indev, &p);
      if (lv_indev_get_state(indev) == LV_INDEV_STATE_PRESSED)
      {
        LOG_INF("TOUCH x=%-4d y=%d", p.x, p.y);
      }
    }
    indev = lv_indev_get_next(indev);
  }
}
#endif /* TOUCH_DEBUG */

/* ── Sensor thread ───────────────────────────────────────── */

/*
 * Runs at priority 5 (below main at 0).  Owns the SGP30 handle
 * exclusively — no other code touches i2c0 or the sgp30_t struct.
 *
 * The 10 ms init delay and 12 ms measurement delay stay here,
 * keeping them entirely off the LVGL thread.
 */
static void sensor_thread_fn(void *p1, void *p2, void *p3)
{
  ARG_UNUSED(p1);
  ARG_UNUSED(p2);
  ARG_UNUSED(p3);

  const struct device *i2c = DEVICE_DT_GET(DT_NODELABEL(i2c0));
  sgp30_t sgp30 = {0};
  bool sensor_ok = (sgp30_init(&sgp30, i2c) == 0);

  if (!sensor_ok)
  {
    LOG_WRN("SGP30 init failed — sending safe defaults (400 ppm / 0 ppb)");
  }

  while (1)
  {
    struct sensor_reading r = {.eco2 = 400, .tvoc = 0};

    if (sensor_ok)
    {
      if (sgp30_measure(&sgp30, &r.eco2, &r.tvoc) == 0)
      {
        LOG_INF("eCO2: %4u ppm  |  TVOC: %4u ppb", r.eco2, r.tvoc);
      }
      else
      {
        LOG_WRN("SGP30 measurement failed — keeping previous values");
      }
    }

    /*
     * K_NO_WAIT: if the queue is full (main is falling behind)
     * we drop the oldest slot and write the fresh reading.
     */
    if (k_msgq_put(&sensor_msgq, &r, K_NO_WAIT) != 0)
    {
      k_msgq_purge(&sensor_msgq);
      k_msgq_put(&sensor_msgq, &r, K_NO_WAIT);
    }

    k_sleep(K_MSEC(SENSOR_INTERVAL_MS));
  }
}

/* ── Sensor thread ───────────────────────────────────────── */

#define SENSOR_THREAD_STACK_SIZE 2048

K_THREAD_DEFINE(sensor_tid,
                SENSOR_THREAD_STACK_SIZE,
                sensor_thread_fn,
                NULL, NULL, NULL,
                5,  /* priority — below main (0), above idle */
                0,  /* no special flags */
                0); /* start immediately */

/* ── main ────────────────────────────────────────────────── */

int main(void)
{
  printk("### main() reached ###\n");

  /* ── Backlight ─────────────────────────────────────── */
  if (!gpio_is_ready_dt(&backlight))
  {
    printk("ERROR: backlight GPIO not ready\n");
    return -ENODEV;
  }
  gpio_pin_configure_dt(&backlight, GPIO_OUTPUT_ACTIVE);
  gpio_pin_set_dt(&backlight, 1);
  printk("Backlight on\n");

  /* ── Display ───────────────────────────────────────── */
  const struct device *display_dev =
      DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

  if (!device_is_ready(display_dev))
  {
    printk("ERROR: display not ready\n");
    return -ENODEV;
  }
  printk("Display ready\n");

  /* ── LVGL ──────────────────────────────────────────── */
  int ret = lvgl_init();
  if (ret != 0)
  {
    printk("ERROR: lvgl_init failed: %d\n", ret);
    return ret;
  }
  printk("lvgl_init done\n");

  display_blanking_off(display_dev);
  ui_init();
  ui_setup_chart();
  printk("UI init done\n");

  lv_refr_now(NULL);
  printk("First refresh done\n");

  /* ── Main loop (LVGL thread) ───────────────────────── */
  while (1)
  {
    uint32_t sleep_ms = lv_timer_handler();
    ui_tick();

#if TOUCH_DEBUG
    touch_debug_log();
#endif

    /* Drain any fresh sensor readings from the queue */
    struct sensor_reading r;
    while (k_msgq_get(&sensor_msgq, &r, K_NO_WAIT) == 0)
    {
      ui_update(r.eco2, r.tvoc, k_uptime_get() / 1000);
    }

    k_msleep(MIN(sleep_ms, LVGL_INTERVAL_MS));
  }

  return 0;
}