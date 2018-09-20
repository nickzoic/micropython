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

#include <stdio.h>
#include <string.h>

#include "py/objtuple.h"
#include "py/objlist.h"
#include "py/runtime.h"
#include "py/stream.h"
#include "py/mperrno.h"
#include "lib/netutils/netutils.h"
#include "modnetwork.h"

#include "shared-bindings/socket/__init__.h"

//| :mod:`socket` --- TCP, UDP and RAW socket support
//| =================================================
//| 
//| .. module:: socket
//|   :synopsis: TCP, UDP and RAW sockets
//|   :platform: SAMD21, SAMD51
//|
//| XXX TODO Write Docs.


STATIC mp_obj_t socket_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 4, false);

    // create socket object (not bound to any NIC yet)
    socket_obj_t *self = m_new_obj_with_finaliser(socket_obj_t);

    mp_int_t family, sock_type, fileno;

    if (n_args >= 1) {
        family = mp_obj_get_int(args[0]);
        if (n_args >= 2) {
            sock_type = mp_obj_get_int(args[1]);
            if (n_args >= 4) {
                fileno = mp_obj_get_int(args[3]);
            } else {
                fileno = -1;
            }
        } else {
            sock_type = MOD_NETWORK_SOCK_STREAM;
        }
    } else {
        family = MOD_NETWORK_AF_INET;
    }

    int err = shared_module_socket_construct(self, family, sock_type, 0, fileno);
    if (err) mp_raise_OSError(err);

    return MP_OBJ_FROM_PTR(self);
}


// method socket.bind(address)
STATIC mp_obj_t socket_bind(mp_obj_t self_in, mp_obj_t addr_in) {
    socket_obj_t *self = MP_OBJ_TO_PTR(self_in);

    // get address
    uint8_t ip[MOD_NETWORK_IPADDR_BUF_SIZE];
    mp_uint_t port = netutils_parse_inet_addr(addr_in, ip, NETUTILS_BIG);

    int err = shared_module_socket_bind(self, ip, port);
    if (err) mp_raise_OSError(err);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(socket_bind_obj, socket_bind);

// method socket.listen(backlog)
STATIC mp_obj_t socket_listen(mp_obj_t self_in, mp_obj_t backlog) {
    socket_obj_t *self = MP_OBJ_TO_PTR(self_in);

    int err = shared_module_socket_listen(self, mp_obj_get_int(backlog));
    if (err) mp_raise_OSError(err);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(socket_listen_obj, socket_listen);

// method socket.accept()
STATIC mp_obj_t socket_accept(mp_obj_t self_in) {
    socket_obj_t *self = MP_OBJ_TO_PTR(self_in);

    // create new socket object
    // starts with empty NIC so that finaliser doesn't run close() method if accept() fails
    socket_obj_t *socket2 = m_new_obj_with_finaliser(socket_obj_t);

    // accept incoming connection
    uint8_t ip[MOD_NETWORK_IPADDR_BUF_SIZE];
    mp_uint_t port;

    int err = shared_module_socket_accept(self, socket2, ip, &port);
    if (err) mp_raise_OSError(err);
   
    // make the return value
    mp_obj_tuple_t *client = MP_OBJ_TO_PTR(mp_obj_new_tuple(2, NULL));
    client->items[0] = MP_OBJ_FROM_PTR(socket2);
    client->items[1] = netutils_format_inet_addr(ip, port, NETUTILS_BIG);

    return MP_OBJ_FROM_PTR(client);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(socket_accept_obj, socket_accept);

// method socket.connect(address)
STATIC mp_obj_t socket_connect(mp_obj_t self_in, mp_obj_t addr_in) {
    socket_obj_t *self = MP_OBJ_TO_PTR(self_in);

    // get address
    uint8_t ip[MOD_NETWORK_IPADDR_BUF_SIZE];
    mp_uint_t port = netutils_parse_inet_addr(addr_in, ip, NETUTILS_BIG);

    // call the NIC to connect the socket

    int err = shared_module_socket_connect(self, ip, port);
    if (err) mp_raise_OSError(err);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(socket_connect_obj, socket_connect);

// method socket.send(bytes)
STATIC mp_obj_t socket_send(mp_obj_t self_in, mp_obj_t buf_in) {
    socket_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf_in, &bufinfo, MP_BUFFER_READ);

    int ret = shared_module_socket_send(self, bufinfo.buf, bufinfo.len);
    if (ret < 0) mp_raise_OSError(-ret);

    return mp_obj_new_int(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(socket_send_obj, socket_send);

// method socket.recv(bufsize)
STATIC mp_obj_t socket_recv(mp_obj_t self_in, mp_obj_t len_in) {
    socket_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_int_t len = mp_obj_get_int(len_in);
    vstr_t vstr;
    vstr_init_len(&vstr, len);

    int ret = shared_module_socket_recv(self, (byte*)vstr.buf, len);
    if (ret < 0) mp_raise_OSError(-ret);
    if (ret == 0) return mp_const_empty_bytes;

    vstr.len = ret;
    return mp_obj_new_str_from_vstr(&mp_type_bytes, &vstr);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(socket_recv_obj, socket_recv);

// method socket.sendto(bytes, address)
STATIC mp_obj_t socket_sendto(mp_obj_t self_in, mp_obj_t data_in, mp_obj_t addr_in) {
    socket_obj_t *self = MP_OBJ_TO_PTR(self_in);

    // get the data
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(data_in, &bufinfo, MP_BUFFER_READ);

    // get address
    uint8_t ip[MOD_NETWORK_IPADDR_BUF_SIZE];
    mp_uint_t port = netutils_parse_inet_addr(addr_in, ip, NETUTILS_BIG);

    int ret = shared_module_socket_sendto(self, bufinfo.buf, bufinfo.len, ip, port);
    if (ret < 0) mp_raise_OSError(-ret);

    return mp_obj_new_int(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(socket_sendto_obj, socket_sendto);

// method socket.recvfrom(bufsize)
STATIC mp_obj_t socket_recvfrom(mp_obj_t self_in, mp_obj_t len_in) {
    socket_obj_t *self = MP_OBJ_TO_PTR(self_in);
    vstr_t vstr;
    vstr_init_len(&vstr, mp_obj_get_int(len_in));
    byte ip[4];
    mp_uint_t port;

    int ret = shared_module_socket_recvfrom(self, (byte*)vstr.buf, vstr.len, ip, &port);
    if (ret < 0) mp_raise_OSError(-ret);

    mp_obj_t tuple[2];
    if (ret == 0) {
        tuple[0] = mp_const_empty_bytes;
    } else {
        vstr.len = ret;
        tuple[0] = mp_obj_new_str_from_vstr(&mp_type_bytes, &vstr);
    }
    tuple[1] = netutils_format_inet_addr(ip, port, NETUTILS_BIG);
    return mp_obj_new_tuple(2, tuple);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(socket_recvfrom_obj, socket_recvfrom);

// method socket.setsockopt(level, optname, value)
STATIC mp_obj_t socket_setsockopt(size_t n_args, const mp_obj_t *args) {
    socket_obj_t *self = MP_OBJ_TO_PTR(args[0]);

    mp_int_t level = mp_obj_get_int(args[1]);
    mp_int_t opt = mp_obj_get_int(args[2]);

    const void *optval;
    mp_uint_t optlen;
    mp_int_t val;
    if (mp_obj_is_integer(args[3])) {
        val = mp_obj_get_int_truncated(args[3]);
        optval = &val;
        optlen = sizeof(val);
    } else {
        mp_buffer_info_t bufinfo;
        mp_get_buffer_raise(args[3], &bufinfo, MP_BUFFER_READ);
        optval = bufinfo.buf;
        optlen = bufinfo.len;
    }

    int err = shared_module_socket_setsockopt(self, level, opt, optval, optlen);
    if (err) mp_raise_OSError(err);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(socket_setsockopt_obj, 4, 4, socket_setsockopt);

// method socket.settimeout(value)
// timeout=0 means non-blocking
// timeout=None means blocking
// otherwise, timeout is in seconds
STATIC mp_obj_t socket_settimeout(mp_obj_t self_in, mp_obj_t timeout_in) {
    socket_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_uint_t timeout;
    if (timeout_in == mp_const_none) {
        timeout = -1;
    } else {
        #if MICROPY_PY_BUILTINS_FLOAT
        timeout = 1000 * mp_obj_get_float(timeout_in);
        #else
        timeout = 1000 * mp_obj_get_int(timeout_in);
        #endif
    }

    int err = shared_module_socket_settimeout(self, timeout);
    if (err) mp_raise_OSError(err);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(socket_settimeout_obj, socket_settimeout);

// method socket.setblocking(flag)
STATIC mp_obj_t socket_setblocking(mp_obj_t self_in, mp_obj_t blocking) {
    socket_obj_t *self = MP_OBJ_TO_PTR(self_in);
    int err = shared_module_socket_settimeout(self, mp_obj_is_true(blocking) ? -1 : 0);
    if (err) mp_raise_OSError(err);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(socket_setblocking_obj, socket_setblocking);

STATIC const mp_rom_map_elem_t socket_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&mp_stream_close_obj) },
    { MP_ROM_QSTR(MP_QSTR_close), MP_ROM_PTR(&mp_stream_close_obj) },
    { MP_ROM_QSTR(MP_QSTR_bind), MP_ROM_PTR(&socket_bind_obj) },
    { MP_ROM_QSTR(MP_QSTR_listen), MP_ROM_PTR(&socket_listen_obj) },
    { MP_ROM_QSTR(MP_QSTR_accept), MP_ROM_PTR(&socket_accept_obj) },
    { MP_ROM_QSTR(MP_QSTR_connect), MP_ROM_PTR(&socket_connect_obj) },
    { MP_ROM_QSTR(MP_QSTR_send), MP_ROM_PTR(&socket_send_obj) },
    { MP_ROM_QSTR(MP_QSTR_recv), MP_ROM_PTR(&socket_recv_obj) },
    { MP_ROM_QSTR(MP_QSTR_sendto), MP_ROM_PTR(&socket_sendto_obj) },
    { MP_ROM_QSTR(MP_QSTR_recvfrom), MP_ROM_PTR(&socket_recvfrom_obj) },
    { MP_ROM_QSTR(MP_QSTR_setsockopt), MP_ROM_PTR(&socket_setsockopt_obj) },
    { MP_ROM_QSTR(MP_QSTR_settimeout), MP_ROM_PTR(&socket_settimeout_obj) },
    { MP_ROM_QSTR(MP_QSTR_setblocking), MP_ROM_PTR(&socket_setblocking_obj) },
};

STATIC MP_DEFINE_CONST_DICT(socket_locals_dict, socket_locals_dict_table);

mp_uint_t socket_ioctl(mp_obj_t self_in, mp_uint_t request, uintptr_t arg, int *errcode) {
    socket_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return shared_module_socket_ioctl(self, request, arg, errcode);
}

STATIC const mp_stream_p_t socket_stream_p = {
    .ioctl = socket_ioctl,
    .is_text = false,
};

STATIC const mp_obj_type_t socket_obj_type = {
    { &mp_type_type },
    .name = MP_QSTR_socket,
    .make_new = socket_make_new,
    .protocol = &socket_stream_p,
    .locals_dict = (mp_obj_dict_t*)&socket_locals_dict,
};

/******************************************************************************/
// socket module

// function socket.getaddrinfo(host, port)
STATIC mp_obj_t mod_socket_getaddrinfo(mp_obj_t host_in, mp_obj_t port_in) {
    size_t hlen;
    const char *host = mp_obj_str_get_data(host_in, &hlen);
    mp_int_t port = mp_obj_get_int(port_in);
    uint8_t out_ip[MOD_NETWORK_IPADDR_BUF_SIZE];

    int err = shared_module_socket_getaddrinfo(host, port, out_ip, MOD_NETWORK_IPADDR_BUF_SIZE);
    if (err) mp_raise_OSError(err);

    mp_obj_tuple_t *tuple = MP_OBJ_TO_PTR(mp_obj_new_tuple(5, NULL));
    tuple->items[0] = MP_OBJ_NEW_SMALL_INT(MOD_NETWORK_AF_INET);
    tuple->items[1] = MP_OBJ_NEW_SMALL_INT(MOD_NETWORK_SOCK_STREAM);
    tuple->items[2] = MP_OBJ_NEW_SMALL_INT(0);
    tuple->items[3] = MP_OBJ_NEW_QSTR(MP_QSTR_);
    tuple->items[4] = netutils_format_inet_addr(out_ip, port, NETUTILS_BIG);
    return mp_obj_new_list(1, (mp_obj_t*)&tuple);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(mod_socket_getaddrinfo_obj, mod_socket_getaddrinfo);

STATIC const mp_rom_map_elem_t mp_module_socket_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_socket) },

    { MP_ROM_QSTR(MP_QSTR_socket), MP_ROM_PTR(&socket_obj_type) },
    { MP_ROM_QSTR(MP_QSTR_getaddrinfo), MP_ROM_PTR(&mod_socket_getaddrinfo_obj) },

    // class constants
    { MP_ROM_QSTR(MP_QSTR_AF_INET), MP_ROM_INT(MOD_NETWORK_AF_INET) },
    { MP_ROM_QSTR(MP_QSTR_AF_INET6), MP_ROM_INT(MOD_NETWORK_AF_INET6) },

    { MP_ROM_QSTR(MP_QSTR_SOCK_STREAM), MP_ROM_INT(MOD_NETWORK_SOCK_STREAM) },
    { MP_ROM_QSTR(MP_QSTR_SOCK_DGRAM), MP_ROM_INT(MOD_NETWORK_SOCK_DGRAM) },
    { MP_ROM_QSTR(MP_QSTR_SOCK_RAW), MP_ROM_INT(MOD_NETWORK_SOCK_RAW) },

    /*
    { MP_ROM_QSTR(MP_QSTR_IPPROTO_IP), MP_ROM_INT(MOD_NETWORK_IPPROTO_IP) },
    { MP_ROM_QSTR(MP_QSTR_IPPROTO_ICMP), MP_ROM_INT(MOD_NETWORK_IPPROTO_ICMP) },
    { MP_ROM_QSTR(MP_QSTR_IPPROTO_IPV4), MP_ROM_INT(MOD_NETWORK_IPPROTO_IPV4) },
    { MP_ROM_QSTR(MP_QSTR_IPPROTO_TCP), MP_ROM_INT(MOD_NETWORK_IPPROTO_TCP) },
    { MP_ROM_QSTR(MP_QSTR_IPPROTO_UDP), MP_ROM_INT(MOD_NETWORK_IPPROTO_UDP) },
    { MP_ROM_QSTR(MP_QSTR_IPPROTO_IPV6), MP_ROM_INT(MOD_NETWORK_IPPROTO_IPV6) },
    { MP_ROM_QSTR(MP_QSTR_IPPROTO_RAW), MP_ROM_INT(MOD_NETWORK_IPPROTO_RAW) },
    */
};

STATIC MP_DEFINE_CONST_DICT(mp_module_socket_globals, mp_module_socket_globals_table);

const mp_obj_module_t mp_module_socket = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_socket_globals,
};
