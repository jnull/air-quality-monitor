/*
 * Air Quality Monitor — main.c  (Stage 4)
 *
 * Navigation model
 * ────────────────
 *  Carousel order (swipe left = forward, swipe right = backward):
 *    Main  →  Histogram  →  Network  →  Settings  →  (wraps to Main)
 *
 *  Button taps use the same navigate_to() path, which picks the
 *  shortest-direction animation automatically.
 *
 *  Tapping chart_history on the Main screen jumps directly to Histogram.
 *
 * Threading model (unchanged from Stage 3)
 * ─────────────────────────────────────────
 *  sensor_thread  (priority 5) — owns SGP30, writes to sensor_msgq
 *  main thread    (priority 0) — owns all LVGL calls
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

/* chart_history (Main screen, 320×92) */
static lv_chart_series_t *chart_ser_eco2;
static lv_chart_series_t *chart_ser_tvoc;

/* chart_fullscreen (Histogram screen, 320×200) */
static lv_chart_series_t *chart_full_ser_eco2;
static lv_chart_series_t *chart_full_ser_tvoc;

/* ── Inter-thread communication ──────────────────────────── */

struct sensor_reading
{
  uint16_t eco2;
  uint16_t tvoc;
};

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

/* ══════════════════════════════════════════════════════════
 *  NAVIGATION
 * ══════════════════════════════════════════════════════════ */

/*
 * Carousel order.
 *   Swipe left  (dx < -SWIPE_THRESHOLD) → forward  → next index
 *   Swipe right (dx >  SWIPE_THRESHOLD) → backward → prev index
 */
#define CAROUSEL_SIZE 4
static const enum ScreensEnum carousel[CAROUSEL_SIZE] = {
    SCREEN_ID_MAIN,
    SCREEN_ID_HISTOGRAM,
    SCREEN_ID_NETWORK,
    SCREEN_ID_SETTINGS,
};

static enum ScreensEnum current_screen = SCREEN_ID_MAIN;

/* Return the root lv_obj_t* for a given screen ID. */
static lv_obj_t *screen_obj(enum ScreensEnum id)
{
  switch (id)
  {
  case SCREEN_ID_MAIN:
    return objects.main;
  case SCREEN_ID_NETWORK:
    return objects.network;
  case SCREEN_ID_HISTOGRAM:
    return objects.histogram;
  case SCREEN_ID_SETTINGS:
    return objects.settings;
  default:
    return objects.main;
  }
}

/* Return the position of id in the carousel array. */
static int carousel_pos(enum ScreensEnum id)
{
  for (int i = 0; i < CAROUSEL_SIZE; i++)
  {
    if (carousel[i] == id)
      return i;
  }
  return 0;
}

/*
 * Navigate to target screen using a directional slide animation.
 * Direction is chosen by taking the shorter path around the carousel ring:
 *   forward distance 1-2 → MOVE_LEFT  (go forward)
 *   forward distance 3   → MOVE_RIGHT (going backward is shorter)
 */
static void navigate_to(enum ScreensEnum target)
{
  if (target == current_screen)
  {
    return;
  }

  int cur = carousel_pos(current_screen);
  int tgt = carousel_pos(target);
  int fwd = (tgt - cur + CAROUSEL_SIZE) % CAROUSEL_SIZE;

  lv_scr_load_anim_t anim = (fwd <= CAROUSEL_SIZE / 2)
                                ? LV_SCR_LOAD_ANIM_MOVE_LEFT
                                : LV_SCR_LOAD_ANIM_MOVE_RIGHT;

  current_screen = target;
  ui_set_screen(target);
  lv_scr_load_anim(screen_obj(target), anim, 200, 0, false);
}

/* ══════════════════════════════════════════════════════════
 *  EVENT CALLBACKS
 * ══════════════════════════════════════════════════════════ */

/* ── Header button navigation ────────────────────────────── */

/*
 * Single callback shared by every nav button across all screens.
 * user_data carries the target ScreensEnum cast to (void *).
 */
static void nav_btn_cb(lv_event_t *e)
{
  enum ScreensEnum target =
      (enum ScreensEnum)(uintptr_t)lv_event_get_user_data(e);
  navigate_to(target);
}

/* ── Swipe gesture ───────────────────────────────────────── */

/* Minimum horizontal travel in pixels to count as a swipe. */
#define SWIPE_THRESHOLD 50

static int32_t swipe_start_x;

static void swipe_press_cb(lv_event_t *e)
{
  lv_indev_t *indev = lv_event_get_indev(e);
  if (!indev)
  {
    return;
  }
  lv_point_t p;
  lv_indev_get_point(indev, &p);
  swipe_start_x = p.x;
}

static void swipe_release_cb(lv_event_t *e)
{
  lv_indev_t *indev = lv_event_get_indev(e);
  if (!indev)
  {
    return;
  }
  lv_point_t p;
  lv_indev_get_point(indev, &p);
  int32_t dx = p.x - swipe_start_x;

  if (dx < -SWIPE_THRESHOLD)
  {
    /* Finger moved left → advance carousel forward */
    int next = (carousel_pos(current_screen) + 1) % CAROUSEL_SIZE;
    navigate_to(carousel[next]);
  }
  else if (dx > SWIPE_THRESHOLD)
  {
    /* Finger moved right → step carousel backward */
    int prev = (carousel_pos(current_screen) - 1 + CAROUSEL_SIZE) % CAROUSEL_SIZE;
    navigate_to(carousel[prev]);
  }
}

/* ── Tap chart_history → jump to fullscreen histogram ───── */

static void chart_tap_cb(lv_event_t *e)
{
  navigate_to(SCREEN_ID_HISTOGRAM);
}

/* ── Dark mode toggle ────────────────────────────────────── */

static void dark_mode_cb(lv_event_t *e)
{
  bool dark = lv_obj_has_state(objects.sw_dark_mode, LV_STATE_CHECKED);
  lv_display_t *disp = lv_display_get_default();
  lv_theme_t *theme = lv_theme_default_init(
      disp,
      lv_palette_main(LV_PALETTE_BLUE),
      lv_palette_main(LV_PALETTE_RED),
      dark,
      LV_FONT_DEFAULT);
  lv_display_set_theme(disp, theme);
  LOG_INF("Dark mode %s", dark ? "on" : "off");
}

/* ── Register all callbacks (called once, after ui_init) ─── */

static void ui_register_callbacks(void)
{
  /* ── Main screen nav buttons ── */
  lv_obj_add_event_cb(objects.btn_main_main,
                      nav_btn_cb, LV_EVENT_CLICKED, (void *)SCREEN_ID_MAIN);
  lv_obj_add_event_cb(objects.btn_main_chart,
                      nav_btn_cb, LV_EVENT_CLICKED, (void *)SCREEN_ID_HISTOGRAM);
  lv_obj_add_event_cb(objects.btn_main_network,
                      nav_btn_cb, LV_EVENT_CLICKED, (void *)SCREEN_ID_NETWORK);
  lv_obj_add_event_cb(objects.btn_main_settings,
                      nav_btn_cb, LV_EVENT_CLICKED, (void *)SCREEN_ID_SETTINGS);

  /* ── Network screen nav buttons ── */
  lv_obj_add_event_cb(objects.btn_network_main,
                      nav_btn_cb, LV_EVENT_CLICKED, (void *)SCREEN_ID_MAIN);
  lv_obj_add_event_cb(objects.btn_network_chart,
                      nav_btn_cb, LV_EVENT_CLICKED, (void *)SCREEN_ID_HISTOGRAM);
  lv_obj_add_event_cb(objects.btn_network_network,
                      nav_btn_cb, LV_EVENT_CLICKED, (void *)SCREEN_ID_NETWORK);
  lv_obj_add_event_cb(objects.btn_network_settings,
                      nav_btn_cb, LV_EVENT_CLICKED, (void *)SCREEN_ID_SETTINGS);

  /* ── Histogram screen nav buttons ── */
  lv_obj_add_event_cb(objects.btn_chart_main,
                      nav_btn_cb, LV_EVENT_CLICKED, (void *)SCREEN_ID_MAIN);
  lv_obj_add_event_cb(objects.btn_chart_chart,
                      nav_btn_cb, LV_EVENT_CLICKED, (void *)SCREEN_ID_HISTOGRAM);
  lv_obj_add_event_cb(objects.btn_chart_network,
                      nav_btn_cb, LV_EVENT_CLICKED, (void *)SCREEN_ID_NETWORK);
  lv_obj_add_event_cb(objects.btn_chart_settings,
                      nav_btn_cb, LV_EVENT_CLICKED, (void *)SCREEN_ID_SETTINGS);

  /* ── Settings screen nav buttons ── */
  lv_obj_add_event_cb(objects.btn_settings_main,
                      nav_btn_cb, LV_EVENT_CLICKED, (void *)SCREEN_ID_MAIN);
  lv_obj_add_event_cb(objects.btn_settings_chart,
                      nav_btn_cb, LV_EVENT_CLICKED, (void *)SCREEN_ID_HISTOGRAM);
  lv_obj_add_event_cb(objects.btn_settings_network,
                      nav_btn_cb, LV_EVENT_CLICKED, (void *)SCREEN_ID_NETWORK);
  lv_obj_add_event_cb(objects.btn_settings_settings,
                      nav_btn_cb, LV_EVENT_CLICKED, (void *)SCREEN_ID_SETTINGS);

  /* ── Swipe on every screen root object ── */
  lv_obj_t *screens[CAROUSEL_SIZE] = {
      objects.main,
      objects.network,
      objects.histogram,
      objects.settings,
  };
  for (int i = 0; i < CAROUSEL_SIZE; i++)
  {
    lv_obj_add_event_cb(screens[i], swipe_press_cb,
                        LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(screens[i], swipe_release_cb,
                        LV_EVENT_RELEASED, NULL);
  }

  /* ── Tap chart_history → Histogram ── */
  lv_obj_add_flag(objects.chart_history, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(objects.chart_history, chart_tap_cb,
                      LV_EVENT_CLICKED, NULL);

  /* ── Dark mode switch ── */
  lv_obj_add_event_cb(objects.sw_dark_mode, dark_mode_cb,
                      LV_EVENT_VALUE_CHANGED, NULL);
}

/* ══════════════════════════════════════════════════════════
 *  UI SETUP AND UPDATE
 * ══════════════════════════════════════════════════════════ */

/*
 * Configure both chart instances identically so they always show the
 * same data.  chart_history (320×92) is the compact overview on Main;
 * chart_fullscreen (320×200) is the detail view on the Histogram screen.
 */
static void ui_setup_chart(void)
{
  lv_obj_t *charts[2] = {objects.chart_history,
                         objects.chart_fullscreen};
  lv_chart_series_t **eco2[2] = {&chart_ser_eco2,
                                 &chart_full_ser_eco2};
  lv_chart_series_t **tvoc[2] = {&chart_ser_tvoc,
                                 &chart_full_ser_tvoc};

  for (int i = 0; i < 2; i++)
  {
    lv_obj_t *c = charts[i];

    lv_chart_set_type(c, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(c, CHART_POINTS);
    lv_chart_set_range(c, LV_CHART_AXIS_PRIMARY_Y, ECO2_MIN, ECO2_MAX);
    lv_chart_set_range(c, LV_CHART_AXIS_SECONDARY_Y, TVOC_MIN, TVOC_MAX);
    lv_obj_set_style_size(c, 0, 0, LV_PART_INDICATOR);

    *eco2[i] = lv_chart_add_series(c,
                                   lv_color_hex(0x00BFFF),
                                   LV_CHART_AXIS_PRIMARY_Y);
    *tvoc[i] = lv_chart_add_series(c,
                                   lv_color_hex(0xFFA500),
                                   LV_CHART_AXIS_SECONDARY_Y);

    lv_chart_set_all_value(c, *eco2[i], LV_CHART_POINT_NONE);
    lv_chart_set_all_value(c, *tvoc[i], LV_CHART_POINT_NONE);
  }
}

/*
 * Push a fresh sensor reading into all live UI elements.
 * Called from the main loop each time a reading arrives from sensor_msgq.
 *
 * label_uptime lives on the Settings screen — updating it here is fine
 * since LVGL only renders it when Settings is active.
 */
static void ui_update(uint16_t eco2, uint16_t tvoc, int64_t uptime_s)
{
  char buf[32];

  /* ── eCO2 ── */
  snprintf(buf, sizeof(buf), "%u", eco2);
  lv_label_set_text(objects.label_eco2_value, buf);
  lv_label_set_text(objects.label_eco2_unit, "ppm");
  lv_bar_set_range(objects.bar_eco2, ECO2_MIN, ECO2_MAX);
  lv_bar_set_value(objects.bar_eco2, eco2, LV_ANIM_ON);

  /* ── TVOC ── */
  snprintf(buf, sizeof(buf), "%u", tvoc);
  lv_label_set_text(objects.label_tvoc_value, buf);
  lv_label_set_text(objects.label_tvoc_unit, "ppb");
  lv_bar_set_range(objects.bar_tvoc, TVOC_MIN, TVOC_MAX);
  lv_bar_set_value(objects.bar_tvoc, tvoc, LV_ANIM_ON);

  /* ── Status label (colour-coded) ── */
  lv_label_set_text(objects.label_status, quality_label(eco2));
  lv_obj_set_style_text_color(objects.label_status,
                              quality_color(eco2), LV_PART_MAIN);

  /* ── Charts — both instances stay in sync ── */
  lv_chart_set_next_value(objects.chart_history,
                          chart_ser_eco2, eco2);
  lv_chart_set_next_value(objects.chart_history,
                          chart_ser_tvoc, tvoc);
  lv_chart_refresh(objects.chart_history);

  lv_chart_set_next_value(objects.chart_fullscreen,
                          chart_full_ser_eco2, eco2);
  lv_chart_set_next_value(objects.chart_fullscreen,
                          chart_full_ser_tvoc, tvoc);
  lv_chart_refresh(objects.chart_fullscreen);

  /* ── Uptime (shown on Settings screen) ── */
  int h = (int)(uptime_s / 3600);
  int m = (int)((uptime_s % 3600) / 60);
  int s = (int)(uptime_s % 60);
  snprintf(buf, sizeof(buf), "%02d:%02d:%02d", h, m, s);
  lv_label_set_text(objects.label_uptime, buf);
}

/* ══════════════════════════════════════════════════════════
 *  SENSOR THREAD
 * ══════════════════════════════════════════════════════════ */

#define SENSOR_THREAD_STACK_SIZE 2048

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
        LOG_WRN("SGP30 measurement failed");
      }
    }

    if (k_msgq_put(&sensor_msgq, &r, K_NO_WAIT) != 0)
    {
      k_msgq_purge(&sensor_msgq);
      k_msgq_put(&sensor_msgq, &r, K_NO_WAIT);
    }

    k_sleep(K_MSEC(SENSOR_INTERVAL_MS));
  }
}

K_THREAD_DEFINE(sensor_tid,
                SENSOR_THREAD_STACK_SIZE,
                sensor_thread_fn,
                NULL, NULL, NULL,
                5,
                0,
                0);

/* ══════════════════════════════════════════════════════════
 *  MAIN
 * ══════════════════════════════════════════════════════════ */

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

  ui_init();               /* create all four screens, load Main */
  ui_register_callbacks(); /* wire touch events */
  ui_setup_chart();        /* configure both chart series */

  printk("UI init done\n");

  lv_refr_now(NULL);
  printk("First refresh done\n");

  /* ── Main loop ─────────────────────────────────────── */
  while (1)
  {
    uint32_t sleep_ms = lv_timer_handler();
    ui_tick();

    struct sensor_reading r;
    while (k_msgq_get(&sensor_msgq, &r, K_NO_WAIT) == 0)
    {
      ui_update(r.eco2, r.tvoc, k_uptime_get() / 1000);
    }

    k_msleep(MIN(sleep_ms, LVGL_INTERVAL_MS));
  }

  return 0;
}