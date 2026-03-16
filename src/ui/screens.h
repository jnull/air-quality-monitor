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
    SCREEN_ID_NETWORK = 2,
    SCREEN_ID_HISTOGRAM = 3,
    SCREEN_ID_SETTINGS = 4,
    _SCREEN_ID_LAST = 4
};

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *network;
    lv_obj_t *histogram;
    lv_obj_t *settings;
    lv_obj_t *main_header;
    lv_obj_t *btn_main_main;
    lv_obj_t *btn_main_chart;
    lv_obj_t *obj0;
    lv_obj_t *btn_main_network;
    lv_obj_t *obj1;
    lv_obj_t *btn_main_settings;
    lv_obj_t *obj2;
    lv_obj_t *main_body;
    lv_obj_t *main_body_left;
    lv_obj_t *label_eco2_titel;
    lv_obj_t *label_eco2_value;
    lv_obj_t *label_eco2_unit;
    lv_obj_t *bar_eco2;
    lv_obj_t *main_body_right;
    lv_obj_t *label_tvoc_titel;
    lv_obj_t *label_tvoc_value;
    lv_obj_t *label_tvoc_unit;
    lv_obj_t *bar_tvoc;
    lv_obj_t *obj_status;
    lv_obj_t *label_status_titel;
    lv_obj_t *label_status;
    lv_obj_t *main_footer;
    lv_obj_t *chart_history;
    lv_obj_t *network_header;
    lv_obj_t *btn_network_main;
    lv_obj_t *obj3;
    lv_obj_t *btn_network_chart;
    lv_obj_t *obj4;
    lv_obj_t *btn_network_network;
    lv_obj_t *btn_network_settings;
    lv_obj_t *obj5;
    lv_obj_t *network_body;
    lv_obj_t *label_thread_role;
    lv_obj_t *label_thread_ipv6;
    lv_obj_t *label_mqtt_state;
    lv_obj_t *label_mqtt_last;
    lv_obj_t *label_rssi;
    lv_obj_t *histogram_header;
    lv_obj_t *btn_chart_main;
    lv_obj_t *obj6;
    lv_obj_t *btn_chart_chart;
    lv_obj_t *obj7;
    lv_obj_t *btn_chart_network;
    lv_obj_t *obj8;
    lv_obj_t *btn_chart_settings;
    lv_obj_t *obj9;
    lv_obj_t *histogram_body;
    lv_obj_t *chart_fullscreen;
    lv_obj_t *settings_header;
    lv_obj_t *btn_settings_main;
    lv_obj_t *obj10;
    lv_obj_t *btn_settings_chart;
    lv_obj_t *obj11;
    lv_obj_t *btn_settings_network;
    lv_obj_t *obj12;
    lv_obj_t *btn_settings_settings;
    lv_obj_t *settings_body;
    lv_obj_t *sw_dark_mode;
    lv_obj_t *label_dark_mode;
    lv_obj_t *label_uptime_title;
    lv_obj_t *label_uptime;
} objects_t;

extern objects_t objects;

void create_screen_main();
void tick_screen_main();

void create_screen_network();
void tick_screen_network();

void create_screen_histogram();
void tick_screen_histogram();

void create_screen_settings();
void tick_screen_settings();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/