#include "ui.h"
#include <lvgl.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ui, LOG_LEVEL_INF);

void ui_init(void)
{
  printk("ui_init entered\n");

  lv_obj_t *scr = lv_scr_act();
  printk("lv_scr_act done: %p\n", (void *)scr);

  if (scr == NULL)
  {
    printk("ERROR: active screen is NULL\n");
    return;
  }

  /* Bright green background — unmistakable if LVGL is flushing */
  lv_obj_set_style_bg_color(scr, lv_color_hex(0x00FF00), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);

  lv_obj_t *label = lv_label_create(scr);
  lv_label_set_text(label, "HELLO!");
  lv_obj_set_style_text_color(label, lv_color_hex(0xFF0000), LV_PART_MAIN);
  lv_obj_set_style_text_font(label, &lv_font_montserrat_24, LV_PART_MAIN);
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

  printk("ui_init done\n");
}

void ui_tick(void)
{
}