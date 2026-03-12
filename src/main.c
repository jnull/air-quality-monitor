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

static const struct gpio_dt_spec backlight =
    GPIO_DT_SPEC_GET(DT_NODELABEL(bl), gpios);

#define SENSOR_INTERVAL_MS 1000
#define LVGL_INTERVAL_MS 5

/* eCO2 range: 400–2000 ppm, TVOC range: 0–500 ppb */
#define ECO2_MIN 400
#define ECO2_MAX 2000
#define TVOC_MIN 0
#define TVOC_MAX 500

/* Chart history — one point per second, 60 points = 1 minute */
#define CHART_POINTS 60

static lv_chart_series_t *chart_ser_eco2;
static lv_chart_series_t *chart_ser_tvoc;

/* Classify air quality based on eCO2 ppm */
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
    return lv_color_hex(0x00C800); /* green  */
  if (eco2 < 1200)
    return lv_color_hex(0xFFA000); /* orange */
  if (eco2 < 1600)
    return lv_color_hex(0xFF4000); /* red    */
  return lv_color_hex(0xFF0000);   /* bright red */
}

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

  /* Pre-fill with LV_CHART_POINT_NONE so the line only draws
     once real data arrives */
  lv_chart_set_all_value(chart, chart_ser_eco2, LV_CHART_POINT_NONE);
  lv_chart_set_all_value(chart, chart_ser_tvoc, LV_CHART_POINT_NONE);
}

static void ui_update(uint16_t eco2, uint16_t tvoc, int64_t uptime_s)
{
  char buf[32];

  /* eCO2 value + bar */
  snprintf(buf, sizeof(buf), "%u", eco2);
  lv_label_set_text(objects.label_eco2_value, buf);
  lv_label_set_text(objects.label_eco2_unit, "ppm");
  lv_bar_set_range(objects.bar_eco2, ECO2_MIN, ECO2_MAX);
  lv_bar_set_value(objects.bar_eco2, eco2, LV_ANIM_ON);

  /* TVOC value + bar */
  snprintf(buf, sizeof(buf), "%u", tvoc);
  lv_label_set_text(objects.label_tvoc_value, buf);
  lv_label_set_text(objects.label_tvoc_unit, "ppb");
  lv_bar_set_range(objects.bar_tvoc, TVOC_MIN, TVOC_MAX);
  lv_bar_set_value(objects.bar_tvoc, tvoc, LV_ANIM_ON);

  /* Status label */
  lv_label_set_text(objects.label_status, quality_label(eco2));
  lv_obj_set_style_text_color(objects.label_status,
                              quality_color(eco2), LV_PART_MAIN);

  /* Chart — push new points (scrolls automatically) */
  lv_chart_set_next_value(objects.chart_history, chart_ser_eco2, eco2);
  lv_chart_set_next_value(objects.chart_history, chart_ser_tvoc, tvoc);
  lv_chart_refresh(objects.chart_history);

  /* Uptime HH:MM:SS */
  int h = uptime_s / 3600;
  int m = (uptime_s % 3600) / 60;
  int s = uptime_s % 60;
  snprintf(buf, sizeof(buf), "%02d:%02d:%02d", h, m, s);
  lv_label_set_text(objects.label_uptime, buf);
}

int main(void)
{
  printk("### main() reached ###\n");

  /* ── Backlight ───────────────────────────────────── */
  if (!gpio_is_ready_dt(&backlight))
  {
    printk("ERROR: backlight GPIO not ready\n");
    return -ENODEV;
  }
  gpio_pin_configure_dt(&backlight, GPIO_OUTPUT_ACTIVE);
  gpio_pin_set_dt(&backlight, 1);
  printk("Backlight on\n");

  /* ── Display ─────────────────────────────────────── */
  const struct device *display_dev =
      DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

  if (!device_is_ready(display_dev))
  {
    printk("ERROR: display not ready\n");
    return -ENODEV;
  }
  printk("Display ready\n");

  /* ── LVGL ─────────────────────────────────────────── */
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

  /* ── SGP30 ───────────────────────────────────────── */
  const struct device *i2c = DEVICE_DT_GET(DT_NODELABEL(i2c0));
  sgp30_t sgp30 = {0};
  bool sensor_ok = (sgp30_init(&sgp30, i2c) == 0);
  if (!sensor_ok)
  {
    printk("WARNING: SGP30 init failed, continuing without sensor\n");
  }

  /* ── Main loop ───────────────────────────────────── */
  int64_t last_sensor_ms = k_uptime_get();

  while (1)
  {
    uint32_t sleep_ms = lv_timer_handler();
    ui_tick();

    int64_t now = k_uptime_get();
    if (now - last_sensor_ms >= SENSOR_INTERVAL_MS)
    {
      last_sensor_ms = now;

      uint16_t eco2 = 400, tvoc = 0; /* safe defaults if sensor absent */
      if (sensor_ok)
      {
        if (sgp30_measure(&sgp30, &eco2, &tvoc) == 0)
        {
          LOG_INF("eCO2: %4u ppm  |  TVOC: %4u ppb", eco2, tvoc);
        }
      }
      ui_update(eco2, tvoc, now / 1000);
    }

    k_msleep(MIN(sleep_ms, LVGL_INTERVAL_MS));
  }

  return 0;
}