#include <stdio.h>

#include "py/objtuple.h"
#include "py/objlist.h"
#include "py/runtime.h"
#include "py/stream.h"
#include "py/mperrno.h"
#include "lib/netutils/netutils.h"

#include "shared-bindings/network/__init__.h"
#include "shared-bindings/socket/__init__.h"


/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Damien P. George
 *               2018 Nick Moore for Adafruit Industries
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

void _select_nic(socket_obj_t *self, const byte *ip) {
    if (self->nic == MP_OBJ_NULL) {
        // select NIC based on IP
        self->nic = mod_network_find_nic(ip);
        self->nic_type = (mod_network_nic_type_t*)mp_obj_get_type(self->nic);

        // call the NIC to open the socket
        int _errno;
        if (self->nic_type->socket(self, &_errno) != 0) {
            mp_raise_OSError(_errno);
        }
    }
}

int shared_module_socket_construct(socket_obj_t *self, int family, int type, int proto, int fileno) {
    self->nic = MP_OBJ_NULL;
    self->nic_type = NULL;
    self->u_param.domain = family;
    self->u_param.type = type;
    self->u_param.fileno = fileno;
    return 0;
}

int shared_module_socket_bind(socket_obj_t *self, uint8_t *ip, mp_uint_t port) {
    _select_nic(self, ip);

    int _errno;
    if (self->nic_type->bind(self, ip, port, &_errno) != 0) return _errno;
    return 0;
}

int shared_module_socket_listen(socket_obj_t *self, mp_uint_t backlog) {
    if (self->nic == MP_OBJ_NULL) {
        // not connected
        // TODO I think we can listen even if not bound...
        return MP_ENOTCONN;
    }

    int _errno;
    if (self->nic_type->listen(self, backlog, &_errno) != 0) return _errno;
    return 0;
}

int shared_module_socket_accept(socket_obj_t *self, socket_obj_t *socket2, uint8_t *ip, mp_uint_t *port) {
    int _errno;
    if (self->nic_type->accept(self, socket2, ip, port, &_errno) != 0) return _errno;
    return 0;
}

int shared_module_socket_connect(socket_obj_t *self, uint8_t *ip, mp_uint_t port) {
    int _errno;
    if (self->nic_type->connect(self, ip, port, &_errno) != 0) return _errno;
    return 0;

}

int shared_module_socket_send(socket_obj_t *self, uint8_t *buf, size_t len) {
    int _errno;
    int ret = self->nic_type->send(self, buf, len, &_errno);
    if (ret == -1) return -_errno;
    return ret;
}

int shared_module_socket_recv(socket_obj_t *self, uint8_t *buf, size_t len) {
    int _errno;
    int ret = self->nic_type->recv(self, buf, len, &_errno);
    if (ret == -1) return -_errno;
    return ret;
}

int shared_module_socket_sendto(socket_obj_t *self, uint8_t *buf, size_t len, uint8_t *ip, mp_uint_t port) {
    int _errno;
    int ret = self->nic_type->sendto(self, buf, len, ip, port, &_errno);
    if (ret == -1) return -_errno;
    return ret;
}

int shared_module_socket_recvfrom(socket_obj_t *self, uint8_t *buf, size_t len, uint8_t *ip, mp_uint_t *port) {
    int _errno;
    int ret = self->nic_type->recvfrom(self, buf, len, ip, port, &_errno);
    if (ret == -1) return -_errno;
    return ret;
}

int shared_module_socket_setsockopt(socket_obj_t *self, mp_int_t level, mp_int_t opt, const void *optval, mp_uint_t optlen) {
    int _errno;
    int ret = self->nic_type->setsockopt(self, buf, len, ip, port, &_errno);
    if (ret == -1) return -_errno;
    return ret;
    return -1;
}

int shared_module_socket_settimeout(socket_obj_t *self, mp_int_t timeout) {
    int _errno;
    if (self->nic_type->settimeout(self, timeout, &_errno) != 0) return _errno;
    return 0;
}

int shared_module_socket_ioctl(socket_obj_t *self, mp_uint_t request, mp_int_t arg, mp_int_t *errcode) {
    return -1;
}


int shared_module_socket_getaddrinfo(const char *host, mp_uint_t port, uint8_t *buf, size_t len) {
    return -1;
}
