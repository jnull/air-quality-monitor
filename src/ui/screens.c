#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"

#include <string.h>

objects_t objects;

//
// Event handlers
//

lv_obj_t *tick_value_change_obj;

//
// Screens
//

void create_screen_main() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.main = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 240);
    {
        lv_obj_t *parent_obj = obj;
        {
            // main_header
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.main_header = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 320, 40);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // btn_main_main
                    lv_obj_t *obj = lv_button_create(parent_obj);
                    objects.btn_main_main = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 80, 40);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "Main");
                        }
                    }
                }
                {
                    // btn_main_chart
                    lv_obj_t *obj = lv_button_create(parent_obj);
                    objects.btn_main_chart = obj;
                    lv_obj_set_pos(obj, 80, 0);
                    lv_obj_set_size(obj, 80, 40);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff83c8fe), LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.obj0 = obj;
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "Graph");
                        }
                    }
                }
                {
                    // btn_main_network
                    lv_obj_t *obj = lv_button_create(parent_obj);
                    objects.btn_main_network = obj;
                    lv_obj_set_pos(obj, 160, 0);
                    lv_obj_set_size(obj, 80, 40);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff83c8fe), LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.obj1 = obj;
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "Network");
                        }
                    }
                }
                {
                    // btn_main_settings
                    lv_obj_t *obj = lv_button_create(parent_obj);
                    objects.btn_main_settings = obj;
                    lv_obj_set_pos(obj, 240, 0);
                    lv_obj_set_size(obj, 80, 40);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff83c8fe), LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.obj2 = obj;
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "Settings");
                        }
                    }
                }
            }
        }
        {
            // main_body
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.main_body = obj;
            lv_obj_set_pos(obj, 0, 40);
            lv_obj_set_size(obj, 320, 200);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // main_body_left
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    objects.main_body_left = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 160, 69);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // label_eco2_titel
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.label_eco2_titel = obj;
                            lv_obj_set_pos(obj, 61, 4);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "eCO2");
                        }
                        {
                            // label_eco2_value
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.label_eco2_value = obj;
                            lv_obj_set_pos(obj, 47, 20);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "Text");
                        }
                        {
                            // label_eco2_unit
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.label_eco2_unit = obj;
                            lv_obj_set_pos(obj, 81, 20);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "Text");
                        }
                        {
                            // bar_eco2
                            lv_obj_t *obj = lv_bar_create(parent_obj);
                            objects.bar_eco2 = obj;
                            lv_obj_set_pos(obj, 5, 41);
                            lv_obj_set_size(obj, 150, 10);
                            lv_bar_set_value(obj, 25, LV_ANIM_OFF);
                        }
                    }
                }
                {
                    // main_body_right
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    objects.main_body_right = obj;
                    lv_obj_set_pos(obj, 160, 0);
                    lv_obj_set_size(obj, 160, 69);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // label_tvoc_titel
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.label_tvoc_titel = obj;
                            lv_obj_set_pos(obj, 60, 4);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "TVOC");
                        }
                        {
                            // label_tvoc_value
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.label_tvoc_value = obj;
                            lv_obj_set_pos(obj, 46, 20);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "Text");
                        }
                        {
                            // label_tvoc_unit
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.label_tvoc_unit = obj;
                            lv_obj_set_pos(obj, 81, 20);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "Text");
                        }
                        {
                            // bar_tvoc
                            lv_obj_t *obj = lv_bar_create(parent_obj);
                            objects.bar_tvoc = obj;
                            lv_obj_set_pos(obj, 5, 41);
                            lv_obj_set_size(obj, 150, 10);
                            lv_bar_set_value(obj, 25, LV_ANIM_OFF);
                        }
                    }
                }
                {
                    // obj_status
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    objects.obj_status = obj;
                    lv_obj_set_pos(obj, 0, 69);
                    lv_obj_set_size(obj, 320, 39);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // label_status_titel
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.label_status_titel = obj;
                            lv_obj_set_pos(obj, 106, 9);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "Status:");
                        }
                        {
                            // label_status
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.label_status = obj;
                            lv_obj_set_pos(obj, 165, 9);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "Text");
                        }
                    }
                }
                {
                    // main_footer
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    objects.main_footer = obj;
                    lv_obj_set_pos(obj, 0, 108);
                    lv_obj_set_size(obj, 320, 92);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // chart_history
                            lv_obj_t *obj = lv_chart_create(parent_obj);
                            objects.chart_history = obj;
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, 320, 92);
                        }
                    }
                }
            }
        }
    }
    
    tick_screen_main();
}

void tick_screen_main() {
}

void create_screen_network() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.network = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 240);
    {
        lv_obj_t *parent_obj = obj;
        {
            // network_header
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.network_header = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 320, 40);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // btn_network_main
                    lv_obj_t *obj = lv_button_create(parent_obj);
                    objects.btn_network_main = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 80, 40);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff83c8fe), LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.obj3 = obj;
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "Main");
                        }
                    }
                }
                {
                    // btn_network_chart
                    lv_obj_t *obj = lv_button_create(parent_obj);
                    objects.btn_network_chart = obj;
                    lv_obj_set_pos(obj, 80, 0);
                    lv_obj_set_size(obj, 80, 40);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff83c8fe), LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.obj4 = obj;
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "Graph");
                        }
                    }
                }
                {
                    // btn_network_network
                    lv_obj_t *obj = lv_button_create(parent_obj);
                    objects.btn_network_network = obj;
                    lv_obj_set_pos(obj, 160, 0);
                    lv_obj_set_size(obj, 80, 40);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "Network");
                        }
                    }
                }
                {
                    // btn_network_settings
                    lv_obj_t *obj = lv_button_create(parent_obj);
                    objects.btn_network_settings = obj;
                    lv_obj_set_pos(obj, 240, 0);
                    lv_obj_set_size(obj, 80, 40);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff83c8fe), LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.obj5 = obj;
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "Settings");
                        }
                    }
                }
            }
        }
        {
            // network_body
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.network_body = obj;
            lv_obj_set_pos(obj, 0, 40);
            lv_obj_set_size(obj, 320, 200);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // label_thread_role
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.label_thread_role = obj;
                    lv_obj_set_pos(obj, 9, 8);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "label_thread_role");
                }
                {
                    // label_thread_ipv6
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.label_thread_ipv6 = obj;
                    lv_obj_set_pos(obj, 9, 30);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "label_thread_ipv6");
                }
                {
                    // label_mqtt_state
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.label_mqtt_state = obj;
                    lv_obj_set_pos(obj, 9, 52);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "label_mqtt_state");
                }
                {
                    // label_mqtt_last
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.label_mqtt_last = obj;
                    lv_obj_set_pos(obj, 9, 74);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "label_mqtt_last");
                }
                {
                    // label_rssi
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.label_rssi = obj;
                    lv_obj_set_pos(obj, 9, 96);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "label_rssi");
                }
            }
        }
    }
    
    tick_screen_network();
}

void tick_screen_network() {
}

void create_screen_histogram() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.histogram = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 240);
    {
        lv_obj_t *parent_obj = obj;
        {
            // histogram_header
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.histogram_header = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 320, 40);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // btn_chart_main
                    lv_obj_t *obj = lv_button_create(parent_obj);
                    objects.btn_chart_main = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 80, 40);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff83c8fe), LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.obj6 = obj;
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "Main");
                        }
                    }
                }
                {
                    // btn_chart_chart
                    lv_obj_t *obj = lv_button_create(parent_obj);
                    objects.btn_chart_chart = obj;
                    lv_obj_set_pos(obj, 80, 0);
                    lv_obj_set_size(obj, 80, 40);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_grad_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.obj7 = obj;
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_bg_grad_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "Graph");
                        }
                    }
                }
                {
                    // btn_chart_network
                    lv_obj_t *obj = lv_button_create(parent_obj);
                    objects.btn_chart_network = obj;
                    lv_obj_set_pos(obj, 160, 0);
                    lv_obj_set_size(obj, 80, 40);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff83c8fe), LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.obj8 = obj;
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "Network");
                        }
                    }
                }
                {
                    // btn_chart_settings
                    lv_obj_t *obj = lv_button_create(parent_obj);
                    objects.btn_chart_settings = obj;
                    lv_obj_set_pos(obj, 240, 0);
                    lv_obj_set_size(obj, 80, 40);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff83c8fe), LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.obj9 = obj;
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "Settings");
                        }
                    }
                }
            }
        }
        {
            // histogram_body
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.histogram_body = obj;
            lv_obj_set_pos(obj, 0, 40);
            lv_obj_set_size(obj, 320, 200);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // chart_fullscreen
                    lv_obj_t *obj = lv_chart_create(parent_obj);
                    objects.chart_fullscreen = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 320, 200);
                }
            }
        }
    }
    
    tick_screen_histogram();
}

void tick_screen_histogram() {
}

void create_screen_settings() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.settings = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 240);
    {
        lv_obj_t *parent_obj = obj;
        {
            // settings_header
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.settings_header = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 320, 40);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // btn_settings_main
                    lv_obj_t *obj = lv_button_create(parent_obj);
                    objects.btn_settings_main = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 80, 40);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff83c8fe), LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.obj10 = obj;
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "Main");
                        }
                    }
                }
                {
                    // btn_settings_chart
                    lv_obj_t *obj = lv_button_create(parent_obj);
                    objects.btn_settings_chart = obj;
                    lv_obj_set_pos(obj, 80, 0);
                    lv_obj_set_size(obj, 80, 40);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff83c8fe), LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.obj11 = obj;
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "Graph");
                        }
                    }
                }
                {
                    // btn_settings_network
                    lv_obj_t *obj = lv_button_create(parent_obj);
                    objects.btn_settings_network = obj;
                    lv_obj_set_pos(obj, 160, 0);
                    lv_obj_set_size(obj, 80, 40);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff83c8fe), LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.obj12 = obj;
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "Network");
                        }
                    }
                }
                {
                    // btn_settings_settings
                    lv_obj_t *obj = lv_button_create(parent_obj);
                    objects.btn_settings_settings = obj;
                    lv_obj_set_pos(obj, 240, 0);
                    lv_obj_set_size(obj, 80, 40);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "Settings");
                        }
                    }
                }
            }
        }
        {
            // settings_body
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.settings_body = obj;
            lv_obj_set_pos(obj, 0, 40);
            lv_obj_set_size(obj, 320, 200);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // sw_dark_mode
                    lv_obj_t *obj = lv_switch_create(parent_obj);
                    objects.sw_dark_mode = obj;
                    lv_obj_set_pos(obj, 215, 6);
                    lv_obj_set_size(obj, 50, 25);
                }
                {
                    // label_dark_mode
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.label_dark_mode = obj;
                    lv_obj_set_pos(obj, 4, 11);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "Dark Mode");
                }
                {
                    // label_uptime_title
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.label_uptime_title = obj;
                    lv_obj_set_pos(obj, 4, 46);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "Uptime");
                }
                {
                    // label_uptime
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.label_uptime = obj;
                    lv_obj_set_pos(obj, 210, 46);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "00:00:00");
                }
            }
        }
    }
    
    tick_screen_settings();
}

void tick_screen_settings() {
}

typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_main,
    tick_screen_network,
    tick_screen_histogram,
    tick_screen_settings,
};
void tick_screen(int screen_index) {
    tick_screen_funcs[screen_index]();
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen_funcs[screenId - 1]();
}

//
// Fonts
//

ext_font_desc_t fonts[] = {
#if LV_FONT_MONTSERRAT_8
    { "MONTSERRAT_8", &lv_font_montserrat_8 },
#endif
#if LV_FONT_MONTSERRAT_10
    { "MONTSERRAT_10", &lv_font_montserrat_10 },
#endif
#if LV_FONT_MONTSERRAT_12
    { "MONTSERRAT_12", &lv_font_montserrat_12 },
#endif
#if LV_FONT_MONTSERRAT_14
    { "MONTSERRAT_14", &lv_font_montserrat_14 },
#endif
#if LV_FONT_MONTSERRAT_16
    { "MONTSERRAT_16", &lv_font_montserrat_16 },
#endif
#if LV_FONT_MONTSERRAT_18
    { "MONTSERRAT_18", &lv_font_montserrat_18 },
#endif
#if LV_FONT_MONTSERRAT_20
    { "MONTSERRAT_20", &lv_font_montserrat_20 },
#endif
#if LV_FONT_MONTSERRAT_22
    { "MONTSERRAT_22", &lv_font_montserrat_22 },
#endif
#if LV_FONT_MONTSERRAT_24
    { "MONTSERRAT_24", &lv_font_montserrat_24 },
#endif
#if LV_FONT_MONTSERRAT_26
    { "MONTSERRAT_26", &lv_font_montserrat_26 },
#endif
#if LV_FONT_MONTSERRAT_28
    { "MONTSERRAT_28", &lv_font_montserrat_28 },
#endif
#if LV_FONT_MONTSERRAT_30
    { "MONTSERRAT_30", &lv_font_montserrat_30 },
#endif
#if LV_FONT_MONTSERRAT_32
    { "MONTSERRAT_32", &lv_font_montserrat_32 },
#endif
#if LV_FONT_MONTSERRAT_34
    { "MONTSERRAT_34", &lv_font_montserrat_34 },
#endif
#if LV_FONT_MONTSERRAT_36
    { "MONTSERRAT_36", &lv_font_montserrat_36 },
#endif
#if LV_FONT_MONTSERRAT_38
    { "MONTSERRAT_38", &lv_font_montserrat_38 },
#endif
#if LV_FONT_MONTSERRAT_40
    { "MONTSERRAT_40", &lv_font_montserrat_40 },
#endif
#if LV_FONT_MONTSERRAT_42
    { "MONTSERRAT_42", &lv_font_montserrat_42 },
#endif
#if LV_FONT_MONTSERRAT_44
    { "MONTSERRAT_44", &lv_font_montserrat_44 },
#endif
#if LV_FONT_MONTSERRAT_46
    { "MONTSERRAT_46", &lv_font_montserrat_46 },
#endif
#if LV_FONT_MONTSERRAT_48
    { "MONTSERRAT_48", &lv_font_montserrat_48 },
#endif
};

//
// Color themes
//

uint32_t active_theme_index = 0;

//
//
//

void create_screens() {

// Set default LVGL theme
    lv_display_t *dispp = lv_display_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
    lv_display_set_theme(dispp, theme);
    
    // Initialize screens
    // Create screens
    create_screen_main();
    create_screen_network();
    create_screen_histogram();
    create_screen_settings();
}