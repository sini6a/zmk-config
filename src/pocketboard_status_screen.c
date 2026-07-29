/*
 * SPDX-License-Identifier: MIT
 */

#include <lvgl.h>

#include <zmk/display/status_screen.h>
#include <zmk/display/widgets/battery_status.h>
#include <zmk/display/widgets/layer_status.h>
#include <zmk/display/widgets/output_status.h>

static struct zmk_widget_battery_status battery_status_widget;
static struct zmk_widget_layer_status layer_status_widget;
static struct zmk_widget_output_status output_status_widget;

lv_obj_t *zmk_display_status_screen(void) {
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    zmk_widget_output_status_init(&output_status_widget, screen);
    lv_obj_t *output = zmk_widget_output_status_obj(&output_status_widget);
    lv_obj_set_style_text_color(output, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(output, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_align(output, LV_ALIGN_TOP_MID, 0, 8);

    zmk_widget_battery_status_init(&battery_status_widget, screen);
    lv_obj_t *battery = zmk_widget_battery_status_obj(&battery_status_widget);
    lv_obj_set_style_text_color(battery, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(battery, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_align(battery, LV_ALIGN_CENTER, 0, -8);

    zmk_widget_layer_status_init(&layer_status_widget, screen);
    lv_obj_t *layer = zmk_widget_layer_status_obj(&layer_status_widget);
    lv_obj_set_style_text_color(layer, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(layer, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_align(layer, LV_ALIGN_BOTTOM_MID, 0, -8);

    return screen;
}
