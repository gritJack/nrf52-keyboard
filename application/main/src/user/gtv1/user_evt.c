/*
Copyright (C) 2019 Geno Kolar <geno@live.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifdef STATUS_LED_ENABLE
#include <stdbool.h>
#include <stdint.h>

#include "ble_keyboard.h"
#include "config.h"
#include "keyboard_command.h"
#include "keyboard_led.h"
#include "main.h"
#include "nrf_delay.h"
#include "status_led.h"
#include "usb_comm.h"

enum keyboard_status {
    kbd_ble,
    kbd_charge,
    kbd_usb
};

static enum keyboard_status status = 0;
#ifdef CHARGING_DETECT
static bool charging_full = false;
#endif
static bool ble_connected = false;
static bool ble_adving = false;

/**
 * @brief 按系统状态更改系统LED、RGB 的颜色
 * 
 */
static void led_status_change()
{
    switch (status) {
    case kbd_ble:
#ifdef LED_BLE
        if (ble_connected) {
            status_led_set_val(LED_BIT_BLE, 1);
        } else {
            status_led_set_val(LED_BIT_BLE, 0);
        }
#endif
#ifdef LED_CHARGING
        status_led_set_val(LED_BIT_CHARGING, 0);
#endif
#ifdef HAS_USB
#ifdef LED_USB
        status_led_set_val(LED_BIT_USB, 0);
#endif
#endif
        status_led_set();
        break;
    case kbd_charge:
#ifdef LED_CHARGING
        if (charging_full) {
            status_led_set_val(LED_BIT_CHARGING, 0);
        } else {
            status_led_set_val(LED_BIT_CHARGING, 1);
        }
#endif
#ifdef LED_BLE
        if (ble_connected) {
            status_led_set_val(LED_BIT_BLE, 1);
        } else {
            status_led_set_val(LED_BIT_BLE, 0);
        }
#endif
#ifdef HAS_USB
#ifdef LED_USB
        if (usb_working()) {
            status_led_set_val(LED_BIT_USB, 1);
        } else {
            status_led_set_val(LED_BIT_USB, 0);
        }
#endif
#endif
        if (!ble_adving) {
            status_led_set();
        }
        break;
    case kbd_usb:
#ifdef LED_USB
        status_led_set_val(LED_BIT_USB, 1);
#endif
#ifdef LED_CHARGING
        if (charging_full) {
            status_led_set_val(LED_BIT_CHARGING, 0);
        } else {
            status_led_set_val(LED_BIT_CHARGING, 1);
        }
#endif
#ifdef LED_BLE
        if (usb_working()) {
            status_led_set_val(LED_BIT_BLE, 0);
        } else {
            status_led_set_val(LED_BIT_BLE, 1);
        }
#endif
        status_led_set();
        break;
    default:
        break;
    }
}

// 这里可以放置用户自定义的处理程序，例如设置灯光等。
void custom_event_handler(enum user_ble_event arg)
{
    // 将事件传递给RGB灯光设置
    switch (arg) {
    case USER_EVT_POST_INIT:
        status_led_init();
        status_led_display();
        nrf_delay_ms(200);
        break;
    case USER_LED_ON:
        //status_led_on();
        break;
    case USER_LED_OFF:
        status_led_off();
        break;
    case USER_LED_DEINIT:
        status_led_deinit();
        break;
    default:
        break;
    }

    switch (arg) {
    case USER_EVT_INITED:
        led_status_change();
        break;
    case USER_USB_DISCONNECT:
        status = kbd_ble;
        led_status_change();
        break;
    case USER_USB_CHARGE:
        status = kbd_charge;
        led_status_change();
        break;
    case USER_USB_CONNECTED:
        status = kbd_usb;
        led_status_change();
        break;
#ifdef CHARGING_DETECT
    case USER_BAT_CHARGING:
        charging_full = false;
        led_status_change();
        break;
    case USER_BAT_FULL:
        charging_full = true;
        led_status_change();
        break;
#endif
    case USER_BLE_DISCONNECT:
        ble_connected = false;
        led_status_change();
        break;
    case USER_BLE_CONNECTED:
        ble_connected = true;
        ble_adving = false;
        ble_blink_led_off();
        led_status_change();
        break;
    case USER_BLE_IDLE:
        ble_connected = false;
        ble_adving = false;
        ble_blink_led_off();
        led_status_change();
        break;
    case USER_BLE_FAST_ADV:
        status_led_off();
        ble_blink_led_on();
        ble_adving = true;
        led_status_change();
        break;
    case USER_BLE_SLOW_ADV: //慢速广播就关闭状态灯闪烁
        ble_connected = false;
        ble_adving = true;
        ble_blink_led_off();
        led_status_change();
        break;
    case USER_EVT_SLEEP_AUTO:
    case USER_EVT_SLEEP_MANUAL:
        status_led_all_on();
        nrf_delay_ms(200);
        break;
    default:
        break;
    }
}
#endif
