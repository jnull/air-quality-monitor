#ifndef EEZ_LVGL_UI_GUI_H
#define EEZ_LVGL_UI_GUI_H

#include <lvgl.h>

#include "screens.h"

#ifdef __cplusplus
extern "C"
{
#endif

  void ui_init();
  void ui_tick();

  void loadScreen(enum ScreensEnum screenId);

  /*
   * Update the internal screen-index tracker without triggering an animation.
   * Call this from main.c after every navigate_to() transition so that
   * ui_tick() keeps ticking the correct screen's tick function.
   */
  void ui_set_screen(enum ScreensEnum screenId);

#ifdef __cplusplus
}
#endif

#endif // EEZ_LVGL_UI_GUI_H