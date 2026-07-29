/*
 * SPDX-License-Identifier: MIT
 */

#include <lvgl.h>
#include <stdio.h>

#include <zmk/battery.h>
#include <zmk/display/status_screen.h>
#include <zmk/usb.h>

lv_obj_t *zmk_display_status_screen(void) {
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    uint8_t level = zmk_battery_state_of_charge();
    if (level > 100) {
        level = 100;
    }

    char percent_text[5];
    snprintf(percent_text, sizeof(percent_text), "%u%%", level);

    lv_obj_t *percent_label = lv_label_create(screen);
    lv_label_set_text(percent_label, percent_text);
    lv_obj_set_style_text_color(percent_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(percent_label, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_align(percent_label, LV_ALIGN_TOP_LEFT, 2, 2);

    lv_obj_t *charge_label = lv_label_create(screen);
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
    lv_label_set_text(charge_label, zmk_usb_is_powered() ? LV_SYMBOL_CHARGE : "");
#else
    lv_label_set_text(charge_label, "");
#endif
    lv_obj_set_style_text_color(charge_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(charge_label, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_align(charge_label, LV_ALIGN_TOP_RIGHT, -2, 2);

    lv_obj_t *battery_frame = lv_obj_create(screen);
    lv_obj_set_size(battery_frame, 48, 120);
    lv_obj_align(battery_frame, LV_ALIGN_CENTER, 0, 10);
    lv_obj_set_style_bg_opa(battery_frame, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_color(battery_frame, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_border_width(battery_frame, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(battery_frame, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_all(battery_frame, 0, LV_PART_MAIN);
    lv_obj_clear_flag(battery_frame, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *battery_fill = lv_obj_create(battery_frame);
    int32_t fill_height = 116 * level / 100;
    if (fill_height < 2 && level > 0) {
        fill_height = 2;
    }
    lv_obj_set_size(battery_fill, 42, fill_height);
    lv_obj_set_style_bg_color(battery_fill, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(battery_fill, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(battery_fill, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(battery_fill, 2, LV_PART_MAIN);
    lv_obj_clear_flag(battery_fill, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(battery_fill, LV_ALIGN_BOTTOM_MID, 0, -2);

    return screen;
}
