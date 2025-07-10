#include <zephyr/device.h>
#include <zephyr/drivers/led.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/events/activity_state_changed.h>

LOG_MODULE_REGISTER(led_status, CONFIG_ZMK_LOG_LEVEL);

#define LED_NODE DT_ALIAS(led0)

BUILD_ASSERT(DT_NODE_EXISTS(LED_NODE), "LED0 alias must be defined in the devicetree");

static const struct device *led_dev = DEVICE_DT_GET(LED_NODE);

static bool connected = false;

K_THREAD_STACK_DEFINE(led_stack_area, 512);
static struct k_thread led_thread_data;
static struct k_sem led_sem;

static void led_thread_fn(void *, void *, void *) {
    while (1) {
        k_sem_take(&led_sem, K_FOREVER);

        if (!device_is_ready(led_dev)) {
            LOG_ERR("LED device not ready");
            continue;
        }

        if (!connected) {
            LOG_INF("LED: advertising mode (blink)");
            while (!connected) {
                led_on(led_dev, 0);
                k_sleep(K_MSEC(200));
                led_off(led_dev, 0);
                k_sleep(K_MSEC(800));
            }
        }

        if (connected) {
            LOG_INF("LED: connected mode (solid ON)");
            led_on(led_dev, 0);
        }
    }
}

static int ble_listener_cb(const zmk_event_t *eh) {
    const struct zmk_ble_active_profile_changed *ev = as_zmk_ble_active_profile_changed(eh);

    connected = ev->connected;
    k_sem_give(&led_sem);
    return 0;
}

ZMK_LISTENER(led_ble_listener, ble_listener_cb);
ZMK_SUBSCRIPTION(led_ble_listener, zmk_ble_active_profile_changed);

static int activity_listener_cb(const zmk_event_t *eh) {
    struct zmk_activity_state_changed *ev = as_zmk_activity_state_changed(eh);

    if (ev->state == ZMK_ACTIVITY_SLEEP) {
        LOG_INF("LED: deep sleep → turning OFF");
        led_off(led_dev, 0);
    }

    return 0;
}

ZMK_LISTENER(led_activity_listener, activity_listener_cb);
ZMK_SUBSCRIPTION(led_activity_listener, zmk_activity_state_changed);

static int led_status_init(void) {
    k_sem_init(&led_sem, 0, 1);
    k_thread_create(&led_thread_data, led_stack_area, K_THREAD_STACK_SIZEOF(led_stack_area),
                    led_thread_fn, NULL, NULL, NULL, K_LOWEST_APPLICATION_THREAD_PRIO, 0,
                    K_NO_WAIT);
    return 0;
}

SYS_INIT(led_status_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
