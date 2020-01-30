/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Scott Shawcroft for Adafruit Industries
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

#ifndef MICROPY_INCLUDED_SUPERVISOR_SHARED_DISPLAY_H
#define MICROPY_INCLUDED_SUPERVISOR_SHARED_DISPLAY_H

#include "shared-bindings/displayio/Bitmap.h"
#include "shared-bindings/displayio/TileGrid.h"
#include "shared-bindings/fontio/BuiltinFont.h"
#include "shared-bindings/terminalio/Terminal.h"

// These are autogenerated resources.

// This is fixed so it doesn't need to be in RAM.
extern const fontio_builtinfont_t supervisor_terminal_font;

// These will change so they must live in RAM.
extern displayio_bitmap_t supervisor_terminal_font_bitmap;
extern displayio_tilegrid_t supervisor_terminal_text_grid;
extern terminalio_terminal_obj_t supervisor_terminal;

void supervisor_start_terminal(uint16_t width_px, uint16_t height_px);
void supervisor_stop_terminal(void);

void supervisor_display_move_memory(void);

#endif  // MICROPY_INCLUDED_SUPERVISOR_SHARED_DISPLAY_H
