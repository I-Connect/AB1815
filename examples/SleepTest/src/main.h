/*
    An Abracon AB18X5 Real-Time Clock library for Arduino
    Copyright (C) 2015 NigelB

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef AB1815EXAMPLES_MAIN_H
#define AB1815EXAMPLES_MAIN_H

#include "Arduino.h"
#include "AB1815.h"
#include "SPI.h"

#ifdef __cplusplus
extern "C" {
#endif

void loop();

void setup();

void initialize_clock(AB1815* clock);

void do_psw_sleep(AB1815 *ab1815_clock, time_t wake_at);

int debug_putchar(char ch, FILE* stream);

#ifdef __cplusplus
}
#endif

#endif //AB1815EXAMPLES_MAIN_H
