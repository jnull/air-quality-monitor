/*
 * src/main.c
 *
 * Air Quality Monitor — ESP32-C6 / Zephyr RTOS
 *
 * Startup sequence:
 *   1. Wait for USB CDC-ACM console to be enumerated (optional, 2 s timeout).
 *   2. Verify the display device is ready.
 *   3. Turn on the backlight (GPIO11).
 *   4. Initialise LVGL and build the UI.
 *   5. Start the SGP30 reader thread.
 *   6. Enter the main loop: fetch latest sensor data → update UI → sleep.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#include <lvgl.h>

#include "ui/ui.h"
#include "sensors/sgp30.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

/* ── Device tree references ──────────────────────────────────────────────── */

#define DISPLAY_NODE DT_CHOSEN(zephyr_display)
#define BACKLIGHT_GPIO_NODE DT_NODELABEL(gpio0)
#define BACKLIGHT_GPIO_PIN 11 /* GPIO11 → display LED */

/* ── Backlight ───────────────────────────────────────────────────────────── */

static int backlight_on(void)
{
  const struct device *gpio = DEVICE_DT_GET(BACKLIGHT_GPIO_NODE);

  if (!device_is_ready(gpio))
  {
    LOG_ERR("GPIO0 device not ready");
    return -ENODEV;
  }

  int ret = gpio_pin_configure(gpio, BACKLIGHT_GPIO_PIN,
                               GPIO_OUTPUT_ACTIVE);
  if (ret < 0)
  {
    LOG_ERR("Backlight GPIO configure failed: %d", ret);
    return ret;
  }

  ret = gpio_pin_set(gpio, BACKLIGHT_GPIO_PIN, 1);
  if (ret < 0)
  {
    LOG_ERR("Backlight GPIO set failed: %d", ret);
    return ret;
  }

  LOG_INF("Display backlight ON");
  return 0;
}

/* ── Main ────────────────────────────────────────────────────────────────── */

int main(void)
{
  int ret;

  LOG_INF("=== Air Quality Monitor booting ===");
  LOG_INF("Zephyr " STRINGIFY(BUILD_VERSION));

  /* 1. Check display device */
  const struct device *display_dev = DEVICE_DT_GET(DISPLAY_NODE);
  if (!device_is_ready(display_dev))
  {
    LOG_ERR("Display device not ready — halting");
    return -ENODEV;
  }
  LOG_INF("Display ready: %s", display_dev->name);

  /* 3. Backlight */
  ret = backlight_on();
  if (ret < 0)
  {
    LOG_WRN("Could not turn on backlight: %d (continuing)", ret);
  }

  /* 4. LVGL init + UI construction
   *    lv_init() is called by Zephyr's LVGL driver during device init,
   *    so we only need to set up our screen here.
   */
  display_blanking_off(display_dev);
  ui_init();
  LOG_INF("UI ready");

  /* 5. Start SGP30 reader */
  ret = sgp30_reader_init();
  if (ret < 0)
  {
    LOG_ERR("SGP30 init failed: %d — check I2C wiring", ret);
    /*
     * Continue anyway so the display still shows something;
     * the UI will stay in warmup mode indefinitely.
     */
  }

  /* 6. Main loop */
  LOG_INF("Entering main loop");

  struct sgp30_data sensor_data = {0};

  while (1)
  {
    sgp30_get_latest(&sensor_data);
    ui_update(sensor_data.eco2_ppm,
              sensor_data.tvoc_ppb,
              sensor_data.valid);
    ui_tick();

    k_sleep(K_MSEC(CONFIG_APP_UI_REFRESH_MS));
  }

  /* unreachable */
  return 0;
}