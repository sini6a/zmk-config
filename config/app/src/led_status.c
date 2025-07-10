/*
 * Copyright (c) 2024
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/led.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/conn.h>
#include <zmk/battery.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/bluetooth_ready.h>
#include <zmk/events/bluetooth_state_changed.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#define LED_NODE DT_ALIAS(blue_led)
const struct device *led_dev;
#define LED_ID 0

static struct k_work_delayable led_blink_work;
static bool ble_connected = false;
static uint8_t battery_level = 100;

static void led_blink_fn(struct k_work *work) {
    static bool led_is_on = false;
    uint32_t on_time = 200;   // default blink on duration
    uint32_t off_time = 800;  // default off duration

    if (ble_connected) {
        led_is_on = true;
    } else {
        if (battery_level < 15) {
            on_time = 500;
            off_time = 500;
        }
    }

    if (ble_connected) {
        led_on(led_dev, LED_ID);
        return;  // Keep solid ON when connected
    } else {
        if (led_is_on) {
            led_off(led_dev, LED_ID);
            led_is_on = false;
            k_work_schedule(&led_blink_work, K_MSEC(off_time));
        } else {
            led_on(led_dev, LED_ID);
            led_is_on = true;
            k_work_schedule(&led_blink_work, K_MSEC(on_time));
        }
    }
}

static int led_status_init() {
    led_dev = DEVICE_DT_GET(LED_NODE);
    if (!device_is_ready(led_dev)) {
        LOG_ERR("LED device not ready");
        return -ENODEV;
    }

    k_work_init_delayable(&led_blink_work, led_blink_fn);

    // Start blinking by default
    k_work_schedule(&led_blink_work, K_NO_WAIT);

    return 0;
}

static int led_bluetooth_listener(const zmk_event_t *eh) {
    const struct zmk_bluetooth_state_changed *ev = as_zmk_bluetooth_state_changed(eh);

    if (ev) {
        ble_connected = ev->connected;
        if (ble_connected) {
            led_on(led_dev, LED_ID);
        } else {
            // Restart blinking
            k_work_schedule(&led_blink_work, K_NO_WAIT);
        }
    }
    return 0;
}

static int led_battery_listener(const zmk_event_t *eh) {
    const struct zmk_battery_state_changed *ev = as_zmk_battery_state_changed(eh);

    if (ev) {
        battery_level = ev->state_of_charge;
    }
    return 0;
}

ZMK_LISTENER(led_status, led_bluetooth_listener);
ZMK_SUBSCRIPTION(led_status, zmk_bluetooth_state_changed);

ZMK_LISTENER(led_battery, led_battery_listener);
ZMK_SUBSCRIPTION(led_battery, zmk_battery_state_changed);

SYS_INIT(led_status_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
