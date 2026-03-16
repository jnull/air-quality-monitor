#include "ui.h"
#include "screens.h"
#include "images.h"
#include "actions.h"
#include "vars.h"

#include <string.h>

static int16_t currentScreen = -1;

static lv_obj_t *getLvglObjectFromIndex(int32_t index)
{
    if (index == -1)
    {
        return 0;
    }
    return ((lv_obj_t **)&objects)[index];
}

void loadScreen(enum ScreensEnum screenId)
{
    currentScreen = screenId - 1;
    lv_obj_t *screen = getLvglObjectFromIndex(currentScreen);
    lv_scr_load_anim(screen, LV_SCR_LOAD_ANIM_FADE_IN, 200, 0, false);
}

/*
 * Update the internal screen index so ui_tick() ticks the correct screen.
 * Called by navigate_to() in main.c after every directional transition.
 */
void ui_set_screen(enum ScreensEnum screenId)
{
    currentScreen = (int16_t)(screenId - 1);
}

void ui_init()
{
    create_screens();
    loadScreen(SCREEN_ID_MAIN);
}

void ui_tick()
{
    tick_screen(currentScreen);
}