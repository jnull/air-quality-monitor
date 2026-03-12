#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_MAIN = 1,
    _SCREEN_ID_LAST = 1
};

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *obj_header;
    lv_obj_t *label_title;
    lv_obj_t *label_uptime;
    lv_obj_t *obj_center;
    lv_obj_t *obj_left;
    lv_obj_t *label_eco2_titel;
    lv_obj_t *label_eco2_value;
    lv_obj_t *label_eco2_unit;
    lv_obj_t *bar_eco2;
    lv_obj_t *obj_right;
    lv_obj_t *label_tvoc_titel;
    lv_obj_t *label_tvoc_value;
    lv_obj_t *label_tvoc_unit;
    lv_obj_t *bar_tvoc;
    lv_obj_t *obj_status;
    lv_obj_t *label_status_titel;
    lv_obj_t *label_status;
    lv_obj_t *obj_footer;
    lv_obj_t *chart_history;
} objects_t;

extern objects_t objects;

void create_screen_main();
void tick_screen_main();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/
