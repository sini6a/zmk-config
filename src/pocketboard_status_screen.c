/*
 * SPDX-License-Identifier: MIT
 */

#include <lvgl.h>
#include <stdio.h>

#include <zmk/battery.h>
#include <zmk/display/status_screen.h>
#include <zmk/display/widgets/layer_status.h>
#include <zmk/display/widgets/output_status.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/usb.h>

static struct zmk_widget_layer_status layer_status_widget;
static struct zmk_widget_output_status output_status_widget;

static lv_obj_t *percent_label;
static lv_obj_t *charge_label;
static lv_obj_t *battery_fill;
static uint8_t battery_level;
static bool usb_powered;

static void update_battery_ui(struct k_work *work) {
    if (percent_label == NULL || charge_label == NULL || battery_fill == NULL) {
        return;
    }

    char percent_text[5];
    snprintf(percent_text, sizeof(percent_text), "%u%%", battery_level);
    lv_label_set_text(percent_label, percent_text);
    lv_label_set_text(charge_label, usb_powered ? LV_SYMBOL_CHARGE : "");

    int32_t fill_height = 116 * battery_level / 100;
    if (fill_height < 2 && battery_level > 0) {
        fill_height = 2;
    }

    lv_obj_set_height(battery_fill, fill_height);
    lv_obj_align(battery_fill, LV_ALIGN_BOTTOM_MID, 0, -2);
}

K_WORK_DEFINE(battery_ui_work, update_battery_ui);

static void queue_battery_ui_update(void) {
    battery_level = zmk_battery_state_of_charge();
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
    usb_powered = zmk_usb_is_powered();
#else
    usb_powered = false;
#endif
    k_work_submit(&battery_ui_work);
}

static int pocketboard_display_event_listener(const zmk_event_t *eh) {
    const struct zmk_battery_state_changed *battery_ev = as_zmk_battery_state_changed(eh);
    const struct zmk_usb_conn_state_changed *usb_ev = as_zmk_usb_conn_state_changed(eh);

    if (battery_ev != NULL || usb_ev != NULL) {
        queue_battery_ui_update();
        return 0;
    }

    return -ENOTSUP;
}

ZMK_LISTENER(pocketboard_display, pocketboard_display_event_listener);
ZMK_SUBSCRIPTION(pocketboard_display, zmk_battery_state_changed);
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
ZMK_SUBSCRIPTION(pocketboard_display, zmk_usb_conn_state_changed);
#endif

lv_obj_t *zmk_display_status_screen(void) {
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    zmk_widget_output_status_init(&output_status_widget, screen);
    lv_obj_t *output = zmk_widget_output_status_obj(&output_status_widget);
    lv_obj_set_style_text_color(output, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(output, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_align(output, LV_ALIGN_BOTTOM_MID, 0, -4);

    percent_label = lv_label_create(screen);
    lv_obj_set_style_text_color(percent_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(percent_label, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_align(percent_label, LV_ALIGN_TOP_LEFT, 2, 2);

    charge_label = lv_label_create(screen);
    lv_obj_set_style_text_color(charge_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(charge_label, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_align(charge_label, LV_ALIGN_TOP_RIGHT, -2, 2);

    lv_obj_t *battery_frame = lv_obj_create(screen);
    lv_obj_set_size(battery_frame, 52, 120);
    lv_obj_align(battery_frame, LV_ALIGN_CENTER, 0, 8);
    lv_obj_set_style_bg_opa(battery_frame, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_color(battery_frame, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_border_width(battery_frame, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(battery_frame, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_all(battery_frame, 0, LV_PART_MAIN);
    lv_obj_clear_flag(battery_frame, LV_OBJ_FLAG_SCROLLABLE);

    battery_fill = lv_obj_create(battery_frame);
    lv_obj_set_width(battery_fill, 46);
    lv_obj_set_style_bg_color(battery_fill, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(battery_fill, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(battery_fill, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(battery_fill, 2, LV_PART_MAIN);
    lv_obj_clear_flag(battery_fill, LV_OBJ_FLAG_SCROLLABLE);

    zmk_widget_layer_status_init(&layer_status_widget, screen);
    lv_obj_t *layer = zmk_widget_layer_status_obj(&layer_status_widget);
    lv_obj_set_style_text_color(layer, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(layer, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_align(layer, LV_ALIGN_TOP_MID, 0, 2);

    queue_battery_ui_update();

    return screen;
}
