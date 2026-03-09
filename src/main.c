#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/display.h>
#include <zephyr/logging/log.h>
#include <lvgl.h>

#include "sgp30.h"
#include "ui/ui.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

/* lvgl_init() is defined in zephyr/modules/lvgl/lvgl.c — not static,
   not exported in a public header, so we declare it here directly.   */
extern int lvgl_init(void);

static const struct gpio_dt_spec backlight =
    GPIO_DT_SPEC_GET(DT_NODELABEL(bl), gpios);

#define SENSOR_INTERVAL_MS 1000
#define LVGL_INTERVAL_MS 5

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

  /* ── LVGL — manual init after display is confirmed ready ── */
  int ret = lvgl_init();
  struct display_capabilities caps;
  display_get_capabilities(display_dev, &caps);
  printk("pixel_format: %d\n", caps.current_pixel_format);
  if (ret != 0)
  {
    printk("ERROR: lvgl_init failed: %d\n", ret);
    return ret;
  }
  printk("lvgl_init done\n");

  lv_obj_t *scr = lv_scr_act();
  printk("lv_scr_act: %p\n", (void *)scr);
  if (scr == NULL)
  {
    printk("ERROR: still no active screen after lvgl_init\n");
    return -ENODEV;
  }

  display_blanking_off(display_dev);

  ui_init();
  printk("UI init done\n");

  /* Force an immediate redraw so the first frame is flushed */
  lv_refr_now(NULL);
  printk("First refresh done\n");

  /* Direct display write test — fills top-left 10x10 pixels red
     If this shows but LVGL doesn't, the SPI path works but LVGL flush is broken
     If this also shows nothing, the display hardware/wiring needs checking */
  struct display_buffer_descriptor desc = {
      .buf_size = 10 * 10 * 2,
      .width = 10,
      .height = 10,
      .pitch = 10,
  };
  static uint8_t test_buf[10 * 10 * 2];
  /* RGB565 red = 0xF800, big-endian on wire = 0xF8, 0x00 */
  for (int i = 0; i < sizeof(test_buf); i += 2)
  {
    test_buf[i] = 0xF8;
    test_buf[i + 1] = 0x00;
  }
  int dret = display_write(display_dev, 0, 0, &desc, test_buf);
  printk("display_write test: %d\n", dret);

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
    if (sensor_ok && (now - last_sensor_ms >= SENSOR_INTERVAL_MS))
    {
      last_sensor_ms = now;
      uint16_t eco2, tvoc;
      if (sgp30_measure(&sgp30, &eco2, &tvoc) == 0)
      {
        LOG_INF("eCO2: %4u ppm  |  TVOC: %4u ppb", eco2, tvoc);
      }
    }

    k_msleep(MIN(sleep_ms, LVGL_INTERVAL_MS));
  }

  return 0;
}