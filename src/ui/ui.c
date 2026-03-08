/*
 * src/ui/ui.c  —  LVGL 9.5 implementation
 *
 * REWRITTEN for LVGL 9 — lv_meter is completely gone.
 *
 * Key API changes from LVGL 8:
 *
 *  LVGL 8                            LVGL 9
 *  ──────────────────────────────    ────────────────────────────────────────
 *  lv_scr_act()                      lv_screen_active()
 *  lv_meter_create()                 lv_scale_create() + LV_SCALE_MODE_ROUND_INNER
 *  lv_meter_add_arc()                lv_scale_add_section() with coloured style
 *  lv_meter_add_needle_line()        lv_line_create() child inside scale
 *  lv_meter_set_indicator_value()    lv_scale_set_line_needle_value()
 *  lv_spinner_create(p, time, arc)   lv_spinner_create(p)
 *                                    + lv_spinner_set_anim_params(p, time, arc)
 *  lv_coord_t                        int32_t  (alias still exists, deprecated)
 *  CONFIG_LV_USE_METER=y             CONFIG_LV_USE_SCALE=y + CONFIG_LV_USE_LINE=y
 *  CONFIG_LV_COLOR_16_SWAP=y         CONFIG_ILI9XXX_RGB565_SWAP=y (driver-side)
 *
 * Screen layout (320x240 landscape):
 *
 *   ┌──────────────────────────────────────────────────────────┐
 *   │           AIR QUALITY MONITOR             <- 28 px header │
 *   ├────────────────────────┬─────────────────────────────────┤
 *   │  lv_scale (ROUND_INNER)│  lv_scale (ROUND_INNER)        │
 *   │  3 coloured sections   │  3 coloured sections            │
 *   │  lv_line needle        │  lv_line needle                 │
 *   │     1234 ppm           │     567 ppb                     │
 *   │      GOOD              │     MODERATE                    │
 *   ├────────────────────────┴─────────────────────────────────┤
 *   │  Updated #n  |  uptime HH:MM:SS           <- 28 px footer │
 *   └──────────────────────────────────────────────────────────┘
 *
 * EEZ Studio note (LVGL 9 edition)
 * ─────────────────────────────────
 * EEZ Studio 0.14+ supports LVGL 9.  When creating your project:
 *   - Set LVGL version to 9.x in project settings.
 *   - Use the "Scale" widget for gauges; EEZ Studio maps it to lv_scale.
 *   - Add an "Line" child object inside the Scale and enable needle mode.
 *   - Export -> "LVGL C code" and drop the generated screens/ files in.
 *   - Keep the ui_init() / ui_update() / ui_tick() interface so main.c
 *     never needs to change.
 * See README.md section 6 for the full walkthrough.
 */

#include "ui.h"
#include <lvgl.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ui, LOG_LEVEL_INF);

/* ── Palette ──────────────────────────────────────────────────────────────── */
#define COLOR_BG lv_color_hex(0x0d1117)
#define COLOR_SURFACE lv_color_hex(0x161b22)
#define COLOR_BORDER lv_color_hex(0x30363d)
#define COLOR_HEADER lv_color_hex(0x1f6feb)
#define COLOR_TEXT_PRI lv_color_hex(0xe6edf3)
#define COLOR_TEXT_SEC lv_color_hex(0x8b949e)
#define COLOR_GOOD lv_color_hex(0x3fb950)     /* green  */
#define COLOR_MODERATE lv_color_hex(0xe3b341) /* amber  */
#define COLOR_POOR lv_color_hex(0xf85149)     /* red    */

/* ── Gauge geometry ───────────────────────────────────────────────────────── */
/*
 * 270-degree sweep, starting at 135 degrees (7:30 clock position).
 * Scale range 0-100; raw sensor values are mapped into this range.
 * Quality zones: GOOD 0-33, MODERATE 33-66, POOR 66-100.
 */
#define GAUGE_ANGLE_RANGE 270
#define GAUGE_START_ANGLE 135
#define SCALE_MIN 0
#define SCALE_MAX 100

/* ── Widget handles ───────────────────────────────────────────────────────── */
static lv_obj_t *eco2_scale;
static lv_obj_t *eco2_needle;
static lv_obj_t *eco2_value_lbl;
static lv_obj_t *eco2_status_lbl;

static lv_obj_t *tvoc_scale;
static lv_obj_t *tvoc_needle;
static lv_obj_t *tvoc_value_lbl;
static lv_obj_t *tvoc_status_lbl;

static lv_obj_t *footer_lbl;
static lv_obj_t *warmup_overlay;

static uint32_t update_count;

/*
 * Section styles MUST live in static memory.
 * lv_scale_section_set_style() stores a pointer, not a copy.
 * One set of three styles is shared across both gauges.
 */
static lv_style_t style_sec_good;
static lv_style_t style_sec_moderate;
static lv_style_t style_sec_poor;
static bool styles_ready;

/* ── Quality helpers ──────────────────────────────────────────────────────── */

typedef enum
{
  QUALITY_GOOD,
  QUALITY_MODERATE,
  QUALITY_POOR
} quality_t;

static quality_t eco2_quality(uint16_t ppm)
{
  if (ppm <= 1000)
    return QUALITY_GOOD;
  if (ppm <= 2000)
    return QUALITY_MODERATE;
  return QUALITY_POOR;
}

static quality_t tvoc_quality(uint16_t ppb)
{
  if (ppb <= 220)
    return QUALITY_GOOD;
  if (ppb <= 660)
    return QUALITY_MODERATE;
  return QUALITY_POOR;
}

/** Map eCO2 (400-2400 ppm) to scale 0-100. */
static int32_t eco2_to_scale(uint16_t ppm)
{
  int32_t v = (int32_t)ppm;
  v = MAX(v, 400);
  v = MIN(v, 2400);
  return ((v - 400) * 100) / 2000;
}

/** Map TVOC (0-800 ppb) to scale 0-100. */
static int32_t tvoc_to_scale(uint16_t ppb)
{
  int32_t v = MIN((int32_t)ppb, 800);
  return (v * 100) / 800;
}

static const char *quality_str(quality_t q)
{
  switch (q)
  {
  case QUALITY_GOOD:
    return "GOOD";
  case QUALITY_MODERATE:
    return "MODERATE";
  case QUALITY_POOR:
    return "POOR";
  default:
    return "---";
  }
}

static lv_color_t quality_color(quality_t q)
{
  switch (q)
  {
  case QUALITY_GOOD:
    return COLOR_GOOD;
  case QUALITY_MODERATE:
    return COLOR_MODERATE;
  case QUALITY_POOR:
    return COLOR_POOR;
  default:
    return COLOR_TEXT_SEC;
  }
}

/* ── Section style init ───────────────────────────────────────────────────── */

/*
 * Each lv_style_t applied to a scale section controls:
 *   arc_color / arc_width  - the arc ring drawn for that zone
 *   line_color             - tick line colour within that zone
 *   text_color             - tick label colour within that zone (labels hidden
 *                            via lv_scale_set_label_show(false) but set anyway)
 *
 * Applied via lv_scale_section_set_style() for:
 *   LV_PART_MAIN      - the arc segment
 *   LV_PART_INDICATOR - major ticks
 *   LV_PART_ITEMS     - minor ticks
 */
static void init_section_styles(void)
{
  if (styles_ready)
  {
    return;
  }

  lv_style_init(&style_sec_good);
  lv_style_set_arc_color(&style_sec_good, COLOR_GOOD);
  lv_style_set_arc_width(&style_sec_good, 8);
  lv_style_set_line_color(&style_sec_good, COLOR_GOOD);
  lv_style_set_text_color(&style_sec_good, COLOR_GOOD);

  lv_style_init(&style_sec_moderate);
  lv_style_set_arc_color(&style_sec_moderate, COLOR_MODERATE);
  lv_style_set_arc_width(&style_sec_moderate, 8);
  lv_style_set_line_color(&style_sec_moderate, COLOR_MODERATE);
  lv_style_set_text_color(&style_sec_moderate, COLOR_MODERATE);

  lv_style_init(&style_sec_poor);
  lv_style_set_arc_color(&style_sec_poor, COLOR_POOR);
  lv_style_set_arc_width(&style_sec_poor, 8);
  lv_style_set_line_color(&style_sec_poor, COLOR_POOR);
  lv_style_set_text_color(&style_sec_poor, COLOR_POOR);

  styles_ready = true;
}

/* ── Gauge card builder ───────────────────────────────────────────────────── */

/*
 * create_gauge_card()
 *
 * LVGL 9 gauge pattern:
 *   1. lv_scale (LV_SCALE_MODE_ROUND_INNER) draws a circular tick ring.
 *      Three lv_scale_section_t zones colour the arc and ticks by quality.
 *   2. lv_line child of the scale acts as the needle.
 *      lv_scale_set_line_needle_value() rotates it to the correct angle.
 *   3. Two lv_label children for the raw numeric value and quality string.
 */
static void create_gauge_card(
    lv_obj_t *parent,
    const char *title,
    const char *unit,
    int32_t x, int32_t y,
    int32_t w, int32_t h,
    lv_obj_t **scale_out,
    lv_obj_t **needle_out,
    lv_obj_t **value_lbl_out,
    lv_obj_t **status_lbl_out)
{
  /* Card container */
  lv_obj_t *card = lv_obj_create(parent);
  lv_obj_set_pos(card, x, y);
  lv_obj_set_size(card, w, h);
  lv_obj_set_scrollbar_mode(card, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_style_bg_color(card, COLOR_SURFACE, 0);
  lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
  lv_obj_set_style_border_color(card, COLOR_BORDER, 0);
  lv_obj_set_style_border_width(card, 1, 0);
  lv_obj_set_style_radius(card, 8, 0);
  lv_obj_set_style_pad_all(card, 4, 0);

  /* Title */
  lv_obj_t *title_lbl = lv_label_create(card);
  lv_label_set_text(title_lbl, title);
  lv_obj_set_style_text_font(title_lbl, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(title_lbl, COLOR_TEXT_SEC, 0);
  lv_obj_align(title_lbl, LV_ALIGN_TOP_MID, 0, 2);

  /* Scale size: fit inside the card leaving room for labels below */
  int32_t sz = MIN(w, h) - 52;
  if (sz < 80)
    sz = 80;

  /* Scale widget */
  lv_obj_t *scale = lv_scale_create(card);
  lv_obj_set_size(scale, sz, sz);
  lv_obj_align(scale, LV_ALIGN_TOP_MID, 0, 18);

  lv_scale_set_mode(scale, LV_SCALE_MODE_ROUND_INNER);
  lv_scale_set_range(scale, SCALE_MIN, SCALE_MAX);
  lv_scale_set_angle_range(scale, GAUGE_ANGLE_RANGE);
  lv_scale_set_rotation(scale, GAUGE_START_ANGLE);
  lv_scale_set_total_tick_count(scale, 21);
  lv_scale_set_major_tick_every(scale, 5);
  lv_scale_set_label_show(scale, false);

  /* Transparent background; sections provide all colour */
  lv_obj_set_style_bg_opa(scale, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(scale, 0, 0);
  /* Hide the default (unstyled) arc that ROUND_INNER draws */
  lv_obj_set_style_arc_opa(scale, LV_OPA_TRANSP, LV_PART_MAIN);

  /* Minor tick default (overridden inside each coloured section) */
  lv_obj_set_style_length(scale, 5, LV_PART_ITEMS);
  lv_obj_set_style_line_color(scale, lv_color_hex(0x444c56), LV_PART_ITEMS);
  lv_obj_set_style_line_width(scale, 2, LV_PART_ITEMS);

  /* Major tick default */
  lv_obj_set_style_length(scale, 10, LV_PART_INDICATOR);
  lv_obj_set_style_line_color(scale, COLOR_TEXT_SEC, LV_PART_INDICATOR);
  lv_obj_set_style_line_width(scale, 2, LV_PART_INDICATOR);

  /*
   * Coloured sections — LVGL 9 API:
   *
   *   lv_scale_section_t *s = lv_scale_add_section(scale);
   *   lv_scale_section_set_range(s, min, max);
   *   lv_scale_section_set_style(s, LV_PART_MAIN,      &style); // arc
   *   lv_scale_section_set_style(s, LV_PART_INDICATOR, &style); // major ticks
   *   lv_scale_section_set_style(s, LV_PART_ITEMS,     &style); // minor ticks
   */
  lv_scale_section_t *sec_good = lv_scale_add_section(scale);
  lv_scale_section_set_range(sec_good, 0, 33);
  lv_scale_section_set_style(sec_good, LV_PART_MAIN, &style_sec_good);
  lv_scale_section_set_style(sec_good, LV_PART_INDICATOR, &style_sec_good);
  lv_scale_section_set_style(sec_good, LV_PART_ITEMS, &style_sec_good);

  lv_scale_section_t *sec_mod = lv_scale_add_section(scale);
  lv_scale_section_set_range(sec_mod, 33, 66);
  lv_scale_section_set_style(sec_mod, LV_PART_MAIN, &style_sec_moderate);
  lv_scale_section_set_style(sec_mod, LV_PART_INDICATOR, &style_sec_moderate);
  lv_scale_section_set_style(sec_mod, LV_PART_ITEMS, &style_sec_moderate);

  lv_scale_section_t *sec_poor = lv_scale_add_section(scale);
  lv_scale_section_set_range(sec_poor, 66, 100);
  lv_scale_section_set_style(sec_poor, LV_PART_MAIN, &style_sec_poor);
  lv_scale_section_set_style(sec_poor, LV_PART_INDICATOR, &style_sec_poor);
  lv_scale_section_set_style(sec_poor, LV_PART_ITEMS, &style_sec_poor);

  /*
   * Needle — lv_line must be a direct child of the scale in LVGL 9.
   * lv_scale_set_line_needle_value() computes start/end points and
   * writes them into the line's point array automatically.
   *
   * needle_length: pixels from the scale centre to the needle tip.
   * Using (sz/2 - 12) leaves a small margin inside the tick ring.
   */
  lv_obj_t *needle = lv_line_create(scale);
  lv_obj_set_style_line_width(needle, 3, 0);
  lv_obj_set_style_line_color(needle, COLOR_TEXT_PRI, 0);
  lv_obj_set_style_line_rounded(needle, true, 0);
  lv_scale_set_line_needle_value(scale, needle, sz / 2 - 12, 0);

  *scale_out = scale;
  *needle_out = needle;

  /* Value label */
  *value_lbl_out = lv_label_create(card);
  lv_label_set_text_fmt(*value_lbl_out, "--- %s", unit);
  lv_obj_set_style_text_font(*value_lbl_out, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(*value_lbl_out, COLOR_TEXT_PRI, 0);
  lv_obj_align_to(*value_lbl_out, scale, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);

  /* Status label */
  *status_lbl_out = lv_label_create(card);
  lv_label_set_text(*status_lbl_out, "---");
  lv_obj_set_style_text_font(*status_lbl_out, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(*status_lbl_out, COLOR_TEXT_SEC, 0);
  lv_obj_align_to(*status_lbl_out, *value_lbl_out,
                  LV_ALIGN_OUT_BOTTOM_MID, 0, 2);
}

/* ── Warmup overlay ───────────────────────────────────────────────────────── */

static void create_warmup_overlay(lv_obj_t *parent)
{
  warmup_overlay = lv_obj_create(parent);
  lv_obj_set_size(warmup_overlay, LV_PCT(100), LV_PCT(100));
  lv_obj_align(warmup_overlay, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_bg_color(warmup_overlay, COLOR_BG, 0);
  lv_obj_set_style_bg_opa(warmup_overlay, LV_OPA_90, 0);
  lv_obj_set_style_border_width(warmup_overlay, 0, 0);
  lv_obj_set_style_radius(warmup_overlay, 0, 0);

  /*
   * LVGL 9 spinner API:
   *   old: lv_spinner_create(parent, anim_time_ms, arc_length_deg)
   *   new: lv_spinner_create(parent)
   *        lv_spinner_set_anim_params(obj, anim_time_ms, arc_length_deg)
   */
  lv_obj_t *spinner = lv_spinner_create(warmup_overlay);
  lv_obj_set_size(spinner, 60, 60);
  lv_obj_align(spinner, LV_ALIGN_CENTER, 0, -20);
  lv_spinner_set_anim_params(spinner, 1000, 60);
  lv_obj_set_style_arc_color(spinner, COLOR_HEADER, LV_PART_INDICATOR);

  lv_obj_t *lbl = lv_label_create(warmup_overlay);
  lv_label_set_text(lbl, "Sensor warming up...");
  lv_obj_set_style_text_color(lbl, COLOR_TEXT_SEC, 0);
  lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, 0);
  lv_obj_align(lbl, LV_ALIGN_CENTER, 0, 30);
}

/* ── Public API ───────────────────────────────────────────────────────────── */

void ui_init(void)
{
  init_section_styles();

  /*
   * LVGL 9: lv_scr_act() renamed to lv_screen_active().
   * lv_scr_act() still compiles as a deprecated alias in 9.x.
   */
  lv_obj_t *scr = lv_screen_active();

  lv_obj_set_style_bg_color(scr, COLOR_BG, 0);
  lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

  /* Header */
  lv_obj_t *header = lv_obj_create(scr);
  lv_obj_set_size(header, 320, 28);
  lv_obj_set_pos(header, 0, 0);
  lv_obj_set_scrollbar_mode(header, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_style_bg_color(header, COLOR_HEADER, 0);
  lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(header, 0, 0);
  lv_obj_set_style_radius(header, 0, 0);
  lv_obj_set_style_pad_all(header, 0, 0);

  lv_obj_t *title_lbl = lv_label_create(header);
  lv_label_set_text(title_lbl, LV_SYMBOL_REFRESH " Air Quality Monitor");
  lv_obj_set_style_text_font(title_lbl, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(title_lbl, lv_color_white(), 0);
  lv_obj_align(title_lbl, LV_ALIGN_CENTER, 0, 0);

  /* Gauge cards */
  create_gauge_card(scr, "eCO\u2082", "ppm",
                    3, 30, 154, 182,
                    &eco2_scale, &eco2_needle,
                    &eco2_value_lbl, &eco2_status_lbl);

  create_gauge_card(scr, "TVOC", "ppb",
                    163, 30, 154, 182,
                    &tvoc_scale, &tvoc_needle,
                    &tvoc_value_lbl, &tvoc_status_lbl);

  /* Footer */
  lv_obj_t *footer = lv_obj_create(scr);
  lv_obj_set_size(footer, 320, 28);
  lv_obj_set_pos(footer, 0, 212);
  lv_obj_set_scrollbar_mode(footer, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_style_bg_color(footer, COLOR_SURFACE, 0);
  lv_obj_set_style_bg_opa(footer, LV_OPA_COVER, 0);
  lv_obj_set_style_border_color(footer, COLOR_BORDER, 0);
  lv_obj_set_style_border_width(footer, 1, 0);
  lv_obj_set_style_border_side(footer, LV_BORDER_SIDE_TOP, 0);
  lv_obj_set_style_radius(footer, 0, 0);
  lv_obj_set_style_pad_ver(footer, 0, 0);
  lv_obj_set_style_pad_hor(footer, 6, 0);

  footer_lbl = lv_label_create(footer);
  lv_label_set_text(footer_lbl, "Waiting for sensor...");
  lv_obj_set_style_text_font(footer_lbl, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(footer_lbl, COLOR_TEXT_SEC, 0);
  lv_obj_align(footer_lbl, LV_ALIGN_LEFT_MID, 0, 0);

  /* Warmup overlay (drawn last = on top) */
  create_warmup_overlay(scr);

  LOG_INF("UI initialised (LVGL %d.%d)",
          LVGL_VERSION_MAJOR, LVGL_VERSION_MINOR);
}

void ui_update(uint16_t eco2_ppm, uint16_t tvoc_ppb, bool valid)
{
  update_count++;

  if (!valid)
  {
    /* Sensor still warming up; keep overlay visible */
    return;
  }

  /* Hide warmup overlay on first valid reading */
  if (warmup_overlay)
  {
    lv_obj_add_flag(warmup_overlay, LV_OBJ_FLAG_HIDDEN);
  }

  /* eCO2 */
  quality_t eq = eco2_quality(eco2_ppm);
  int32_t eco2_nlen = lv_obj_get_width(eco2_scale) / 2 - 12;

  lv_scale_set_line_needle_value(eco2_scale, eco2_needle,
                                 eco2_nlen, eco2_to_scale(eco2_ppm));
  lv_label_set_text_fmt(eco2_value_lbl, "%u ppm", eco2_ppm);
  lv_label_set_text(eco2_status_lbl, quality_str(eq));
  lv_obj_set_style_text_color(eco2_status_lbl, quality_color(eq), 0);

  /* TVOC */
  quality_t tq = tvoc_quality(tvoc_ppb);
  int32_t tvoc_nlen = lv_obj_get_width(tvoc_scale) / 2 - 12;

  lv_scale_set_line_needle_value(tvoc_scale, tvoc_needle,
                                 tvoc_nlen, tvoc_to_scale(tvoc_ppb));
  lv_label_set_text_fmt(tvoc_value_lbl, "%u ppb", tvoc_ppb);
  lv_label_set_text(tvoc_status_lbl, quality_str(tq));
  lv_obj_set_style_text_color(tvoc_status_lbl, quality_color(tq), 0);

  /* Footer uptime */
  uint32_t uptime_s = k_uptime_get_32() / 1000U;

  lv_label_set_text_fmt(footer_lbl,
                        "Updated #%u  |  uptime %02u:%02u:%02u",
                        update_count,
                        uptime_s / 3600,
                        (uptime_s % 3600) / 60,
                        uptime_s % 60);
}

void ui_tick(void)
{
  lv_timer_handler();
}