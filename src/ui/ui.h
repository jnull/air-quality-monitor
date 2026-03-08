#ifndef APP_UI_H
#define APP_UI_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * @brief Initialise the LVGL screen and create all widgets.
   *
   * Must be called after the display device is ready and lv_init() has been
   * called (Zephyr's LVGL shim calls lv_init() automatically during
   * driver init, so just call this from main after device_is_ready()).
   */
  void ui_init(void);

  /**
   * @brief Update both gauges with fresh sensor data.
   *
   * @param eco2_ppm  eCO2 concentration in ppm  (400–8192)
   * @param tvoc_ppb  TVOC concentration in ppb  (0–60000)
   * @param valid     false while sensor is still warming up
   */
  void ui_update(uint16_t eco2_ppm, uint16_t tvoc_ppb, bool valid);

  /**
   * @brief Drive lv_timer_handler().
   *
   * Call periodically from the main loop at the rate set by
   * CONFIG_APP_UI_REFRESH_MS.
   */
  void ui_tick(void);

#ifdef __cplusplus
}
#endif
#endif /* APP_UI_H */