/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Nick Moore
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "esp_log.h"
#include "esp_now.h"
#include "esp_wifi.h"

#include "freertos/queue.h"

#include "py/runtime.h"
#include "py/mphal.h"
#include "py/nlr.h"
#include "py/objlist.h"
#include "py/runtime.h"
#include "py/mphal.h"
#include "py/mperrno.h"

#include "modnetwork.h"

NORETURN void _esp_espnow_exceptions(esp_err_t e) {
   switch (e) {
      case ESP_ERR_ESPNOW_NOT_INIT:
        mp_raise_msg(&mp_type_OSError, "ESP-Now Not Initialized");
      case ESP_ERR_ESPNOW_ARG:
        mp_raise_msg(&mp_type_OSError, "ESP-Now Invalid Argument");
      case ESP_ERR_ESPNOW_NO_MEM:
        mp_raise_msg(&mp_type_OSError, "ESP-Now Out Of Mem");
      case ESP_ERR_ESPNOW_FULL:
        mp_raise_msg(&mp_type_OSError, "ESP-Now Peer List Full");
      case ESP_ERR_ESPNOW_NOT_FOUND:
        mp_raise_msg(&mp_type_OSError, "ESP-Now Peer Not Found");
      case ESP_ERR_ESPNOW_INTERNAL:
        mp_raise_msg(&mp_type_OSError, "ESP-Now Internal");
      case ESP_ERR_ESPNOW_EXIST:
        mp_raise_msg(&mp_type_OSError, "ESP-Now Peer Exists");
      default:
        nlr_raise(mp_obj_new_exception_msg_varg(
          &mp_type_RuntimeError, "ESP-Now Unknown Error 0x%04x", e
        ));
   }
}

static inline void esp_espnow_exceptions(esp_err_t e) {
    if (e != ESP_OK) _esp_espnow_exceptions(e);
}

static inline void _get_bytes(mp_obj_t str, size_t len, uint8_t *dst) {
    size_t str_len;
    const char *data = mp_obj_str_get_data(str, &str_len);	
    if (str_len != len) mp_raise_ValueError("bad len");
    memcpy(dst, data, len);
}

static mp_obj_t recv_cb_obj = MP_OBJ_NULL, send_cb_obj = MP_OBJ_NULL;

void _recv_cb(const uint8_t *macaddr, const uint8_t *data, int len) 
{
    if (recv_cb_obj != MP_OBJ_NULL) {
	mp_obj_t msg = mp_obj_new_bytes(macaddr, ESP_NOW_ETH_ALEN);
	mp_obj_t mac = mp_obj_new_bytes(data, len);
	mp_call_function_2_protected(recv_cb_obj, msg, mac);
    }
} 

void _send_cb(const uint8_t *macaddr, esp_now_send_status_t status)
{
    if (send_cb_obj != MP_OBJ_NULL) {
	mp_obj_t mac = mp_obj_new_bytes(macaddr, ESP_NOW_ETH_ALEN);
	mp_call_function_2_protected(send_cb_obj, mac, status ? mp_const_false : mp_const_true);
    }
}

static int initialized = 0;

STATIC mp_obj_t espnow_init() {
    if (!initialized) {
        esp_now_init();
        initialized = 1;
	recv_cb_obj = MP_OBJ_NULL;
	send_cb_obj = MP_OBJ_NULL;
	esp_now_register_recv_cb(_recv_cb);
	esp_now_register_send_cb(_send_cb);
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(espnow_init_obj, espnow_init);

STATIC mp_obj_t espnow_deinit() {
    if (initialized) {
        esp_now_deinit();
	recv_cb_obj = MP_OBJ_NULL;
	send_cb_obj = MP_OBJ_NULL;
        initialized = 0;
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(espnow_deinit_obj, espnow_deinit);

STATIC mp_obj_t espnow_set_pmk(mp_obj_t pmk) {
    uint8_t buf[ESP_NOW_KEY_LEN];
    _get_bytes(pmk, ESP_NOW_KEY_LEN, buf);
    esp_espnow_exceptions(esp_now_set_pmk(buf));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(espnow_set_pmk_obj, espnow_set_pmk);

STATIC mp_obj_t espnow_add_peer(size_t n_args, const mp_obj_t *args) {
    esp_now_peer_info_t peer = {0};
    // leaving channel as 0 for autodetect
    peer.ifidx = ((wlan_if_obj_t *)MP_OBJ_TO_PTR(args[0]))->if_id;
    _get_bytes(args[1], ESP_NOW_ETH_ALEN, peer.peer_addr);
    if (n_args > 2) {
        _get_bytes(args[2], ESP_NOW_KEY_LEN, peer.lmk);
        peer.encrypt = 1;
    }
    esp_espnow_exceptions(esp_now_add_peer(&peer));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(espnow_add_peer_obj, 2, 3, espnow_add_peer);

STATIC mp_obj_t espnow_del_peer(mp_obj_t addr) {
    uint8_t peer_addr[ESP_NOW_ETH_ALEN];
    _get_bytes(addr, ESP_NOW_ETH_ALEN, peer_addr);
    esp_espnow_exceptions(esp_now_del_peer(peer_addr));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(espnow_del_peer_obj, espnow_del_peer);

STATIC mp_obj_t espnow_send(mp_obj_t addr, mp_obj_t mesg) {
    uint8_t peer_addr[ESP_NOW_ETH_ALEN];
    _get_bytes(addr, ESP_NOW_ETH_ALEN, peer_addr);

    mp_uint_t mesg_len;
    const uint8_t *mesg_buf = (const uint8_t *)mp_obj_str_get_data(mesg, &mesg_len);
    if (mesg_len > ESP_NOW_MAX_DATA_LEN) mp_raise_ValueError("Mesg too long");
    esp_espnow_exceptions(esp_now_send(peer_addr, mesg_buf, mesg_len));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(espnow_send_obj, espnow_send);

STATIC mp_obj_t espnow_set_recv_cb(mp_obj_t callback) {
    recv_cb_obj = callback;
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(espnow_set_recv_cb_obj, espnow_set_recv_cb);

STATIC mp_obj_t espnow_set_send_cb(mp_obj_t callback) {
    send_cb_obj = callback;
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(espnow_set_send_cb_obj, espnow_set_send_cb);

STATIC mp_obj_t espnow_send_all(mp_obj_t mesg) {
    mp_uint_t mesg_len;
    const uint8_t *mesg_buf = (const uint8_t *)mp_obj_str_get_data(mesg, &mesg_len);
    if (mesg_len > ESP_NOW_MAX_DATA_LEN) mp_raise_ValueError("Mesg too long");
    esp_espnow_exceptions(esp_now_send(NULL, mesg_buf, mesg_len));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(espnow_send_all_obj, espnow_send_all);

STATIC const mp_rom_map_elem_t espnow_globals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&espnow_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&espnow_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_pmk), MP_ROM_PTR(&espnow_set_pmk_obj) },
    { MP_ROM_QSTR(MP_QSTR_add_peer), MP_ROM_PTR(&espnow_add_peer_obj) },
    { MP_ROM_QSTR(MP_QSTR_send), MP_ROM_PTR(&espnow_send_obj) },
    { MP_ROM_QSTR(MP_QSTR_send_all), MP_ROM_PTR(&espnow_send_all_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_recv_cb), MP_ROM_PTR(&espnow_set_recv_cb_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_send_cb), MP_ROM_PTR(&espnow_set_send_cb_obj) },
};
STATIC MP_DEFINE_CONST_DICT(espnow_globals_dict, espnow_globals_dict_table);

const mp_obj_module_t mp_module_esp_espnow = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&espnow_globals_dict,
};
