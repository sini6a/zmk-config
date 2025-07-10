#include <zephyr/device.h>
#include <zephyr/drivers/led.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zmk/ble.h>
#include <zmk/events/activity_state_changed.h>

LOG_MODULE_REGISTER(led_status, CONFIG_ZMK_LOG_LEVEL);

#define PWM_LED_NODE_ID DT_COMPAT_GET_ANY_STATUS_OKAY(gpio_leds)

BUILD_ASSERT(DT_NODE_EXISTS(DT_ALIAS(led0)), "LED0 alias must be defined in the devicetree");

static const struct device *pwm_dev = DEVICE_DT_GET(DT_PWMS_CTLR(PWM_LED_NODE_ID));
static const uint32_t pwm_channel = DT_PWMS_CHANNEL(PWM_LED_NODE_ID);
static const uint32_t pwm_flags = DT_PWMS_FLAGS(PWM_LED_NODE_ID);

K_THREAD_STACK_DEFINE(led_stack_area, 512);
static struct k_thread led_thread_data;

static bool sleep_state = false;

#define PWM_PERIOD_USEC 1000U // 1 kHz for example

void set_led_brightness(uint32_t level) {
    // level 0 = off, level 100 = full on
    uint32_t pulse = (PWM_PERIOD_USEC * level) / 100;
    pwm_set_pulse(pwm_dev, pwm_channel, PWM_PERIOD_USEC, pulse, pwm_flags);
}

static void led_thread_fn(void *, void *, void *) {
    bool prev_connected = false;
    bool prev_advertising = false;

    while (1) {
        if (!device_is_ready(pwm_dev)) {
            LOG_ERR("LED device not ready");
            k_sleep(K_SECONDS(5));
            continue;
        }

        if (sleep_state) {
            set_led_brightness(0);
            k_sleep(K_SECONDS(1));
            continue;
        }

        bool connected = zmk_ble_active_profile_is_connected();
        bool advertising = zmk_ble_active_profile_is_open();

        if (connected) {
            if (!prev_connected) {
                LOG_INF("BLE connected → LED ON");
                set_led_brightness(50);
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
            set_led_brightness(50);
            k_sleep(K_MSEC(200));
            set_led_brightness(0);
            k_sleep(K_MSEC(800));
        } else {
            // Not connected and not advertising
            LOG_INF("BLE idle → LED OFF");
            set_led_brightness(0);
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