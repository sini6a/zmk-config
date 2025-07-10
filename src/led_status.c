#include <zephyr/device.h>
#include <zephyr/drivers/led.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zmk/ble.h>
#include <zmk/events/activity_state_changed.h>

LOG_MODULE_REGISTER(led_status, CONFIG_ZMK_LOG_LEVEL);

#define LED_NODE DT_ALIAS(blue_led)

BUILD_ASSERT(DT_NODE_EXISTS(LED_NODE), "LED0 alias must be defined in the devicetree");

static const struct device *led_dev = DEVICE_DT_GET(LED_NODE);

K_THREAD_STACK_DEFINE(led_stack_area, 512);
static struct k_thread led_thread_data;

static bool sleep_state = false;

static void led_thread_fn(void *, void *, void *) {
    bool prev_connected = false;
    bool prev_advertising = false;

    while (1) {
        if (!device_is_ready(led_dev)) {
            LOG_ERR("LED device not ready");
            k_sleep(K_SECONDS(5));
            continue;
        }

        if (sleep_state) {
            led_off(led_dev, 0);
            k_sleep(K_SECONDS(1));
            continue;
        }

        bool connected = zmk_ble_active_profile_is_connected();
        bool advertising = zmk_ble_active_profile_is_open();

        if (connected) {
            if (!prev_connected) {
                LOG_INF("BLE connected → LED ON");
                led_on(led_dev, 0);
                prev_connected = true;
                prev_advertising = false;
            }
            k_sleep(K_SECONDS(1));
        } else if (advertising) {
            if (!prev_advertising) {
                LOG_INF("BLE advertising → LED blinking");
                prev_advertising = true;
                prev_connected = false;
            }
            led_on(led_dev, 0);
            k_sleep(K_MSEC(200));
            led_off(led_dev, 0);
            k_sleep(K_MSEC(800));
        } else {
            // Not connected and not advertising
            LOG_INF("BLE idle → LED OFF");
            led_off(led_dev, 0);
            prev_connected = false;
            prev_advertising = false;
            k_sleep(K_SECONDS(1));
        }
    }
}

static int activity_listener_cb(const zmk_event_t *eh) {
    struct zmk_activity_state_changed *ev = as_zmk_activity_state_changed(eh);

    if (ev->state == ZMK_ACTIVITY_SLEEP) {
        LOG_INF("Going to sleep → LED OFF");
        sleep_state = true;
    } else {
        sleep_state = false;
    }
    return 0;
}

ZMK_LISTENER(led_activity_listener, activity_listener_cb);
ZMK_SUBSCRIPTION(led_activity_listener, zmk_activity_state_changed);

static int led_status_init(void) {
    k_thread_create(&led_thread_data, led_stack_area, K_THREAD_STACK_SIZEOF(led_stack_area),
                    led_thread_fn, NULL, NULL, NULL,
                    K_LOWEST_APPLICATION_THREAD_PRIO, 0, K_NO_WAIT);
    return 0;
}

SYS_INIT(led_status_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);