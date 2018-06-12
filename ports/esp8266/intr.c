/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Damien P. George
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

#include "etshal.h"
#include "ets_alt_task.h"

#include "py/mpstate.h"
#include "modmachine.h"
#include "common-hal/pulseio/PulseIn.h"

// this is in a separate file so it can go in iRAM

void intr_gpio_register_handler(uint8_t gpio_number, void (*func)(void *), void *data) {
    ETS_GPIO_INTR_DISABLE();
    MP_STATE_PORT(pin_irq_handler_func)[gpio_number] = func;
    MP_STATE_PORT(pin_irq_handler_data)[gpio_number] = data;
    ETS_GPIO_INTR_ENABLE();
}

void pin_intr_handler_iram(void *arg) {
    uint32_t status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, status);

    status &= (1 << GPIO_PIN_COUNT) - 1;
    for (int p = 0; status; ++p, status >>= 1) {
        if (status & 1) {
            void (*func)(void *) = MP_STATE_PORT(pin_irq_handler_func)[p];
            if (func) {
                func(MP_STATE_PORT(pin_irq_handler_data)[p]);
            }
        }
    }
}

void intr_gpio_initialize() {
    ETS_GPIO_INTR_DISABLE();
    ETS_GPIO_INTR_ATTACH(pin_intr_handler_iram, NULL);
    ETS_GPIO_INTR_ENABLE();
}

