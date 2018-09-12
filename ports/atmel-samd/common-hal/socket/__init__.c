#include <stdio.h>

#include "py/objtuple.h"
#include "py/objlist.h"
#include "py/runtime.h"
#include "py/stream.h"
#include "py/mperrno.h"
#include "lib/netutils/netutils.h"
#include "modnetwork.h"

#include "shared-bindings/socket/__init__.h"


/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Nick Moore for Adafruit Industries
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

int common_hal_socket_construct(socket_obj_t *self, int family, int type, int proto, int fileno) {
    return -1;
}

int common_hal_socket_bind(socket_obj_t *self, uint8_t *ip, mp_uint_t port) {
    return -1;
}

int common_hal_socket_listen(socket_obj_t *self, mp_uint_t backlog) {
    return -1;
}

int common_hal_socket_accept(socket_obj_t *self, socket_obj_t *socket2, uint8_t *ip, mp_uint_t *port) {
    return -1;
}

int common_hal_socket_connect(socket_obj_t *self, uint8_t *ip, mp_uint_t port) {
    return -1;
}

int common_hal_socket_send(socket_obj_t *self, uint8_t *buf, size_t len) {
    return -1;
}

int common_hal_socket_recv(socket_obj_t *self, uint8_t *buf, size_t len) {
    return -1;
}

int common_hal_socket_sendto(socket_obj_t *self, uint8_t *buf, size_t len, uint8_t *ip, mp_uint_t port) {
    return -1;
}

int common_hal_socket_recvfrom(socket_obj_t *self, uint8_t *buf, size_t len, uint8_t *ip, mp_uint_t *port) {
    return -1;
}

int common_hal_socket_setsockopt(socket_obj_t *self, mp_int_t level, mp_int_t opt, const void *optval, mp_uint_t optlen) {
    return -1;
}

int common_hal_socket_settimeout(socket_obj_t *self, mp_int_t timeout) {
    return -1;
}

int common_hal_socket_ioctl(socket_obj_t *self, mp_uint_t request, mp_int_t arg, mp_int_t *errcode) {
    return -1;
}


int common_hal_socket_getaddrinfo(const char *host, mp_uint_t port, uint8_t *buf, size_t len) {
    return -1;
}
