/**
  *	An Abracon AB18X5 Real-Time Clock library for Arduino
  *	Copyright (C) 2015 NigelB
  *
  *	This program is free software; you can redistribute it and/or modify
  *	it under the terms of the GNU General Public License as published by
  *	the Free Software Foundation; either version 2 of the License, or
  *	(at your option) any later version.
  *
  *	This program is distributed in the hope that it will be useful,
  *	but WITHOUT ANY WARRANTY; without even the implied warranty of
  *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  *	GNU General Public License for more details.
  *
  *	You should have received a copy of the GNU General Public License along
  *	with this program; if not, write to the Free Software Foundation, Inc.,
  *	51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
  **/ 

#ifndef AB1815_REGISTERS_H_
#define AB1815_REGISTERS_H_

#define AB1815_SPI_READ(offset) (127 & offset)
#define AB1815_SPI_WRITE(offset) (128 | offset)

#define AB1815_REG_TIME_HUNDREDTHS 0x00
#define AB1815_REG_ALARM_HUNDREDTHS 0x08

#define AB1815_REG_STATUS                   0x0F
#define AB1815_REG_CONTROL1                 0x10
#define AB1815_REG_CONTROL2                 0x11
#define AB1815_REG_INTERRUPT_MASK           0x12
#define AB1815_REG_SQW                      0x13
#define AB1815_REG_CAL_XT                   0x14
#define AB1815_REG_CAL_RC_HI                0x15
#define AB1815_REG_CAL_RC_LOW               0x16
#define AB1815_REG_SLEEP_CONTROL            0x17
#define AB1815_REG_COUNTDOWN_TIMER_CONTROL  0x18
#define AB1815_REG_COUNTDOWN_TIMER          0x19
#define AB1815_REG_COUNTDOWN_TIMER_INITIAL  0x1A
#define AB1815_REG_WATCHDOG_TIMER           0x1B
#define AB1815_REG_OSCILLATOR_CONTROL       0x1C
#define AB1815_REG_OSCILLATOR_STATUS        0x1D
#define AB1815_REG_CONFIGURATION_KEY        0x1F
#define AB1815_REG_TRICKLE_CONTROL          0x20
#define AB1815_REG_BREF_CONTROL             0x21
#define AB1815_REG_AFCTRL                   0x26
#define AB1815_REG_BATMODE_IO               0x27
#define AB1815_REG_ANALOG_STATUS            0x2F
#define AB1815_REG_OUTPUT_CONTROL           0x30

#define AB1815_REG_ID0 0x28
#define AB1815_REG_ID1 0x29
#define AB1815_REG_ID2 0x2A
#define AB1815_REG_ID3 0x2B
#define AB1815_REG_ID4 0x2C
#define AB1815_REG_ID5 0x2D
#define AB1815_REG_ID6 0x2E

#define AB1815_EXTENTION_RAM 0x3F



#endif /* AB1815_REGISTERS_H_ */
