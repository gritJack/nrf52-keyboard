/*
Copyright (C) 2019 Jim Jiang <jim@lotlab.org>

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

#include "passkey.h"
#include "custom_hook.h"
#include "../ble/ble_services.h"

static uint8_t inputed_len = 0xFF;
static uint8_t passkey[6];

void hook_send_keyboard(report_keyboard_t* report)
{
    // 处理Passkey的输入
    if (inputed_len < 6) {
        for (uint8_t i = 0; i < sizeof(report->keys); i++) {
            uint8_t keycode = report->keys[i];
            if (keycode >= KC_1 && keycode <= KC_0) {
                passkey[inputed_len++] = (keycode + 1 - KC_1) % 10 + '0';
                break;
            } else if (keycode >= KC_KP_1 && keycode <= KC_KP_0) {
                passkey[inputed_len++] = (keycode + 1 - KC_KP_1) % 10 + '0';
                break;
            } else if (keycode == KC_BSPACE && inputed_len > 0) {
                inputed_len--;
                break;
            }
        }
        if (inputed_len == 6) {
            ble_passkey_send(passkey);
            inputed_len = 0xFF;
        }
    }
}

/**
 * @brief 设置需要输入Passkey
 * 
 */
void passkey_req_handler() {
    inputed_len = 0;
}