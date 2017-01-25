/*
    An Abracon AB1815 Real-Time Clock library for Arduino
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

#include "main.h"

FILE* debug;
AB1815 * ab1815_clock;

int debug_putchar(char ch, FILE* stream)
{
    Serial.write(ch) ;
    return (0) ;
}

void setup() {
    Serial.begin(9600);
    debug = new FILE();
    fdev_setup_stream (debug, debug_putchar, NULL, _FDEV_SETUP_WRITE);
    ab1815_clock = new AB1815(10);
    initialize_clock(ab1815_clock);
    delay(1000);
    ab1815_clock->hex_dump(debug);

    Serial.print("# Found: AB");
    Serial.print(ab1815_clock->id.ID0);
    Serial.println(ab1815_clock->id.ID1);
}


int a = 1;
int b = 0;
int tmp = 0;

void loop() {
    long start = millis();

    while (millis() - start < 20000)
    {
        tmp = a + b;
        b=a;
        a=tmp;
    }

    ab1815_tmElements_t time;
    do_psw_sleep(ab1815_clock, ab1815_clock->get()+200000000);
    delay(20000);

}

void initialize_clock(AB1815* clock)
{
    oscillator_control_t oscillator_control;
    clock->get_oscillator_control(&oscillator_control);
    Serial.print("# retrieved oscillator_control: ");
    Serial.println(oscillator_control.value, HEX);

//  OSEL = 1 when using RC oscillator instead of XTAL
	oscillator_control.fields.OSEL = 1;

//  Disable I/O Interface during sleep to ensure the clock it not corrupted
//  by floating pins and what not.
    oscillator_control.fields.PWGT = 1;
    Serial.print("# set oscillator_control: ");
    Serial.println(oscillator_control.value, HEX);

    clock->set_oscillator_control(&oscillator_control);

//  Hundredths don't seem to tick over when using the RC clock source
//  So I clear them
    clock->clear_hundrdeds();


    struct control1_t control1;
    clock->get_control1(&control1);

//	0 is 24 Hour Mode
    control1.fields._12_24 = 0;

//  1 is Power Switch Mode
    control1.fields.PWR2 = 1;
    clock->set_control1(&control1);


    inturrupt_mask_t int_mask;
    clock->get_interrupt_mask(&int_mask);

//  Alarm Interrupt Enable = true
    int_mask.fields.AIE = 1;

//  Set Interrupt Mode to be a Logic Level (opposed to a pulse)
    int_mask.fields.IM = ab1815_interrupt_im_level;
    clock->set_interrupt_mask(&int_mask);

    control2_t control2;

//  Set NIRQ Pin to output NIRQ (since AIE is enabled)
//    control2.fields.OUT1S = ab1815_fout_nIRQ_or_OUT;

//  Set NIRQ2 pin to be power switched sleep
    control2.fields.OUT2S = ab1815_psw_SLEEP;
    clock->set_control2(&control2);

}

void do_psw_sleep(AB1815 *ab1815_clock, time_t wake_at)
{
    ab1815_tmElements_t alarm, tm;
    breakTime(wake_at, alarm);

    ab1815_clock->get_time(&tm);
    fprintf(debug,
            "# Current Time: %i/%i/%i %i:%i:%02i\r\n",
            tmYearToCalendar(tm.Year),
            tm.Month,
            tm.Day,
            tm.Hour,
            tm.Minute,
            tm.Second
    );

    fprintf(debug, "# Wake At: %i/%i/%i %i:%i:%02i\r\n",
            tmYearToCalendar(alarm.Year),
            alarm.Month,
            alarm.Day,
            alarm.Hour,
            alarm.Minute,
            alarm.Second
    );

    status_t status;
    ab1815_clock->get_status(&status);
    status.fields.ALM = 0;
    ab1815_clock->set_status(&status);


    ab1815_clock->set_alarm(&alarm, ab1815_alarm_repeat_once_per_day);
    ab1815_clock->hex_dump(debug);
    delay(100);

    // Set the sleep bit and wait ~8ms (SLTO=1) before enabling the PSW switch
    // and powering down you microcontroller.
    sleep_control_t sleep_control;
    ab1815_clock->get_sleep_control(&sleep_control);
    sleep_control.fields.SLP = 1;
    sleep_control.fields.SLTO = 1;
    ab1815_clock->set_sleep_control(&sleep_control);

    Serial.println("Sleep Sent");
    delay(3000);
}

