/**
  *     An Abracon AB18X5 Real-Time Clock library for Arduino
  *     Copyright (C) 2015 NigelB
  *
  *     This program is free software; you can redistribute it and/or modify
  *     it under the terms of the GNU General Public License as published by
  *     the Free Software Foundation; either version 2 of the License, or
  *     (at your option) any later version.
  *
  *     This program is distributed in the hope that it will be useful,
  *     but WITHOUT ANY WARRANTY; without even the implied warranty of
  *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  *     GNU General Public License for more details.
  *
  *     You should have received a copy of the GNU General Public License along
  *     with this program; if not, write to the Free Software Foundation, Inc.,
  *     51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
  **/

#ifndef AB1815_H_
#define AB1815_H_

#include "AB1815_registers.h"
#include "Arduino.h"
#include "Time.h"

struct ab1815_tmElements_t: tmElements_t
{
	uint8_t Hundredth;
};

// 0x0F
struct status_t {
	union{
		uint8_t value;
		struct{
			bool EX1: 1;
			bool EX2: 1;
			bool ALM: 1;
			bool TIM: 1;
			bool BL : 1;
			bool WD_T: 1;
			bool BAT: 1;
			bool CB : 1;
		} fields;
	};
};

// 0x10
struct control1_t {
	union{
		uint8_t value;
		struct {
			bool WRTC: 1;
			bool PWR2: 1;
			bool ARST: 1;
			bool RSP : 1;
			bool OUT : 1;
			bool OUTB: 1;
			bool _12_24: 1;
			bool STOP: 1;
		} fields;
	};
};

// 0x11
struct control2_t {
	union{
		uint8_t value;
		struct {
			uint8_t OUT1S: 2;
			uint8_t OUT2S: 3;
			bool    RS1E: 1;
			bool    PAD: 1;
			bool    OUTPP: 1;
		} fields;
	};
};

// 0x12
struct inturrupt_mask_t
{
	union{
		uint8_t value;
		struct{
			bool EX1E : 1;
			bool EX2E : 1;
			bool AIE : 1;
			bool TIE : 1;
			bool BLIE : 1;
			uint8_t IM : 2;
			bool CEB : 1;
		} fields;
	};
};

// 0x13
struct square_wave_t {
    union {
        uint8_t value;
        struct {
            uint8_t SQFS: 5;
            uint8_t PAD: 2;
            bool SQWE: 1;
        } fields;
    };
};

// 0x14
struct cal_xt_t {
    union {
        uint8_t value;
        struct {
            uint8_t OFFSETX: 7;
            uint8_t CMDX: 1;
        } fields;
    };
};

// 0x15
struct cal_rc_hi_t {
    union {
        uint8_t value;
        struct {
            uint8_t OFFSETR: 6;
            uint8_t CMDR: 2;
        } fields;
    };
};

// 0x16
struct cal_rc_low_t
{
	uint8_t OFFSETR;
};

// 0x17
struct sleep_control_t
{
	union{
		uint8_t value;
		struct {
			uint8_t SLTO : 3;
			uint8_t SLST : 1;
			uint8_t EX1P : 1;
			uint8_t EX2P : 1;
			uint8_t SLRES: 1;
			uint8_t SLP  : 1;
		} fields;
	};
};

// 0x18
struct countdown_control_t
{
	union{
		uint8_t value;
		struct{
			uint8_t TFS: 2;
			uint8_t RPT: 3;
			uint8_t TRPT: 1;
			uint8_t TM: 1;
			uint8_t TE: 1;
		} fields;
	};
};
// 0X19 - Countdown Timer - no struct

// 0x1A - Timer Initial Value - no struct

// 0x1B
struct watchdog_timer_t {
    union {
        uint8_t value;
        struct {
            uint8_t WRB: 2;
            uint8_t BMB: 5;
            uint8_t WDS: 1;
        } fields;
    };
};

// 0x1C

struct oscillator_control_t
{
	union {
		uint8_t value;
		struct {
			bool _ACIE : 1;
			bool OFIE : 1;
			bool PWGT : 1;
			bool FOS : 1;
			bool AOS : 1;
			uint8_t ACAL : 2;
			bool OSEL : 1;
		} fields;
	};
};

// 0x1D
struct oscillator_status_t {
	union {
		uint8_t value;
		struct {
			bool ACF: 1;
			bool OF: 1;
			uint8_t RESERVED: 2;
			bool OMODE: 1;
			bool LKO2: 1;
			uint8_t XTCAL: 2;
		} fields;
	};
};

// 0x1F - Configuration Key - See Enums below

// 0x20
struct trickle_t {
	union {
		uint8_t value;
		struct {
			uint8_t ROUT: 2;
			uint8_t DIODE: 2;
			uint8_t TCS: 4;
		} fields;
	};
};

// 0x21
struct bref_control_t {
    union {
        uint8_t value;
        struct {
            uint8_t RESERVED: 4;
            uint8_t BREF: 4;
        } fields;
    };
};

// 0x26 – AFCTRL - See Enums below

// 0x27
struct battery_mode_io_t {
	union {
		uint8_t value;
		struct {
			uint8_t RESERVED: 7; // Must only be 0b0000000
			bool IOBM: 1;
		} fields;
	};
};

// 0x28
struct ab1815_id_t {
	uint8_t ID0;
	uint8_t ID1;
	union ID2 {
		uint8_t value;
		struct {
			uint8_t MINOR:3;
			uint8_t MAJOR:5;
		} Revision;
	} ID2;
	uint8_t ID3;
	uint8_t ID4;
	uint8_t ID5;
	uint8_t ID6;
};

// 0x2F
struct ab1815_analog_status_t {
    union {
        uint8_t value;
        struct {
            bool RESERVED: 1;
            bool VINT: 1;
            bool RESERVED_2: 4;
            bool BMIN: 1;
            bool BMOD: 1;
        } fields;
    };
};

// 0x30
struct ab1815_output_control_t
{
    uint8_t value;
    struct {
        bool O1EN: 1;
        bool O3EN: 1;
        bool O4EN: 1;
        bool RSEN: 1;
        bool EXDS: 1;
        bool WDDS: 1;
        bool EXBM: 1;
        bool WDBM: 1;
    } fields;
};


// 0x3f
struct extension_ram_t {
	union {
		uint8_t value;
		struct {
			uint8_t XADS: 2;
			bool XADA: 1;
			bool RSVD: 1;
			bool EXIN: 1;
			bool WDIN: 1;
			bool BPOL: 1;
			bool O4BM: 1;
		} fields;
	};
};

// 0x1F

enum configuration_key_e
{
	ab1815_oscillator_control = 0xA1,
	ab1815_software_reset = 0x3c,
	ab1815_reg_control = 0x9D,
};


enum ab1815_clk_format_e
{
	ab1815_clk_format_unset = 0,
	ab1815_clk_format_12_hour = 1,
	ab1815_clk_format_25_hour = 2
};

enum ab1815_interrupt_im_e
{
	ab1815_interrupt_im_level = 0,
	ab1815_interrupt_im_1_8192 = 1,
	ab1815_interrupt_im_1_64 = 2,
	ab1815_interrupt_im_1_4 = 3
};

enum ab1815_psw_nirq2_pin_control_e
{
	ab1815_psw_nIRQ_or_OUTB = 0,  // If at least one interrupt is enabled
	ab1815_psw_SQW_or_OUTB = 1,   // If SQWE == 1
	ab1815_psw_RESERVED = 2,
	ab1815_psw_nAIRQ_or_OUTB = 3, // If AIE  == 1
	ab1815_psw_TIRQ_or_OUTB = 4,  // If TIE  == 1
	ab1815_psw_nTIRQ_or_OUTB = 5, // If TIE  == 1
	ab1815_psw_SLEEP = 6,
	ab1815_psw_OUTB = 7
};

enum ab1815_fout_nirq_pin_control_e
{
	ab1815_fout_nIRQ_or_OUT = 0,         // If at least one interrupt is enabled
	ab1815_fout_SQW_or_OUT = 1,          // If SQWE == 1
	ab1815_fout_SQW_or_nIRQ_or_OUT = 2,  // If SQWE == 1, else nIRQ if at least one interrupt is enabled
	ab1815_fout_nAIRQ_or_OUT = 3         // If AIE == 1
};

enum days_of_week_e {
	Sunday = 1,
	Monday = 2,
	Tuesday = 3,
	Wednesday = 4,
	Thursday = 4,
	Friday = 5,
	Saterday = 6
};

enum ab1815_status_e {
	ab1815_status_e_OK,
	ab1815_status_e_ERROR
};

enum ab1815_batmodeio_e {
	ab1815_batmodeio_disabled = 0,
	ab1815_batmodeio_enabled = 128,
};

// 0x26
enum afctrl_e {
    afctrl_disable = 0,
    afctrl_enable = 0xA0,
};

enum ab1815_alarm_repeat_mode {
	ab1815_alarm_repeat_alarm_disabled = 0,
	ab1815_alarm_repeat_once_per_year = 1,
	ab1815_alarm_repeat_once_per_month = 2,
	ab1815_alarm_repeat_once_per_week = 3,
	ab1815_alarm_repeat_once_per_day = 4,
	ab1815_alarm_repeat_once_per_hour = 5,
	ab1815_alarm_repeat_once_per_minute = 6,
	ab1815_alarm_repeat_once_per_second = 7,
	ab1815_alarm_repeat_once_per_tenth = 8,
	ab1815_alarm_repeat_once_per_hundredth = 9 //Invalid without
};

// 0x1B
enum watchdog_timer_frequency_e
{
        ab1815_watchdog_16HZ = 0b00,
        ab1815_watchdog_4HZ =  0b01,
        ab1815_watchdog_1HZ = 0b10,
        ab1815_watchdog_1_4HZ = 0b11, // 1/4 HZ
};

// 0x20
enum trickle_charge_tcs_e
{
        ab1815_trickle_charge_enable =  0b1010,
};

enum trickle_charge_diode_e
{
        ab1815_trickle_charge_diode_0v6 = 0b10,         // 0.6v standard diode
        ab1815_trickle_charge_diode_0v3 = 0b01,         // 0.3v schottky diode
        ab1815_trickle_charge_diode_disable = 0b00, //
};

enum trickle_charge_resistor_e
{
        ab1815_trickle_charge_resistor_disable = 0b00,
        ab1815_trickle_charge_resistor_3k_ohm = 0b01,
        ab1815_trickle_charge_resistor_6k_ohm = 0b10,
        ab1815_trickle_charge_resistor_11k_ohm = 0b11,
};

// 0x21
enum battery_voltage_reference_select_e
{
        ab1815_battery_reference_2v5_3v0 = 0b0111,
        ab1815_battery_reference_2v1_2v5 = 0b1011,
        ab1815_battery_reference_1v8_2v2 = 0b1101,
        ab1815_battery_reference_1v4_1v6 = 0b1111,//Also reset value?
};

class AB1815
{
private:

	uint64_t error_code;

	uint16_t cs_pin;
	struct {
		uint8_t _12_24: 2;
		uint8_t clk_source: 2;
	} fields;

    enum ab1815_status_e init();
    enum ab1815_status_e read(uint8_t offset, uint8_t *buf, uint8_t length);
    enum ab1815_status_e write(uint8_t offset, uint8_t *buf, uint8_t length);
    void spi_select_slave(bool select);

public:
    ab1815_id_t id;

    AB1815(uint16_t cs_pin);

    // 0x00
    time_t get();
    void set(time_t time);
    enum ab1815_status_e get_time(ab1815_tmElements_t *time);
    enum ab1815_status_e set_time(ab1815_tmElements_t *time);
    enum ab1815_status_e hundrdeds();
    enum ab1815_status_e clear_hundrdeds();

    // 0x08
    enum ab1815_status_e get_alarm(ab1815_tmElements_t *time, enum ab1815_alarm_repeat_mode *alarm_mode);
    enum ab1815_status_e set_alarm(ab1815_tmElements_t *time, enum ab1815_alarm_repeat_mode alarm_mode);

    // 0x0F - See also: ARST in Control1.
    //	If ARST is a 1, a read of the Status register will produce the current state of all
    //	the interrupt flags and then clear them
    enum ab1815_status_e set_status(status_t* status);
    enum ab1815_status_e get_status(status_t* status);

    // 0x10
    enum ab1815_status_e set_control1(control1_t* control1);
    enum ab1815_status_e get_control1(control1_t* control1);

    // 0x11
    enum ab1815_status_e set_control2(control2_t* control2);
    enum ab1815_status_e get_control2(control2_t* control2);

    // 0x12
    enum ab1815_status_e set_interrupt_mask(inturrupt_mask_t* inturrupt_mask);
    enum ab1815_status_e get_interrupt_mask(inturrupt_mask_t* inturrupt_mask);

    // 0x13
    enum ab1815_status_e set_square_wave(square_wave_t* square_wave);
    enum ab1815_status_e get_square_wave(square_wave_t* square_wave);

    // 0x14
    enum ab1815_status_e set_cal_xt(cal_xt_t* cal_xt);
    enum ab1815_status_e get_cal_xt(cal_xt_t* cal_xt);

    // 0x15
    enum ab1815_status_e set_cal_rc_hi(cal_rc_hi_t* cal_rc_hi);
    enum ab1815_status_e get_cal_rc_hi(cal_rc_hi_t* cal_rc_hi);

    // 0x16
    enum ab1815_status_e set_cal_rc_low(cal_rc_low_t* cal_rc_low);
    enum ab1815_status_e get_cal_rc_low(cal_rc_low_t* cal_rc_low);

    // 0x17 sleep_control_t
    enum ab1815_status_e set_sleep_control(sleep_control_t* sleep_control);
    enum ab1815_status_e get_sleep_control(sleep_control_t* sleep_control);

    // 0x18 Set the countdown control register
    enum ab1815_status_e set_countdown_control(countdown_control_t* countdown_control);
    enum ab1815_status_e get_countdown_control(countdown_control_t* countdown_control);

    // 0x19
    enum ab1815_status_e set_countdown_timer(uint8_t timer_value);
    enum ab1815_status_e get_countdown_timer(uint8_t* timer_value);

    // 0x1A
    enum ab1815_status_e set_countdown_timer_initial_value(uint8_t timer_value);
    enum ab1815_status_e get_countdown_timer_initial_value(uint8_t* timer_value);

    // 0x1B
    enum ab1815_status_e set_watchdog_timer(watchdog_timer_t* watchdog_timer);
    enum ab1815_status_e get_watchdog_timer(watchdog_timer_t* watchdog_timer);


    // 0x1C Get the oscillator control register
    enum ab1815_status_e get_oscillator_control(oscillator_control_t *oscillator_control);
    enum ab1815_status_e set_oscillator_control(oscillator_control_t *oscillator_control);

    // 0x1D
    enum ab1815_status_e set_oscillator_status(oscillator_status_t* oscillator_status);
    enum ab1815_status_e get_oscillator_status(oscillator_status_t* oscillator_status);

    // 0x1E - Nothing on the AB1815
    // 0x1F
    enum ab1815_status_e set_configuration_key(enum configuration_key_e configuration_key);

    // 0x20
    enum ab1815_status_e set_trickle(trickle_t* trickle);
    enum ab1815_status_e get_trickle(trickle_t* trickle);

    // 0x21
    enum ab1815_status_e set_bref_control(bref_control_t* bref_control);
    enum ab1815_status_e get_bref_control(bref_control_t* bref_control);

    // 0x26
    enum ab1815_status_e set_afctrl(afctrl_e afctrl);
    enum ab1815_status_e get_(afctrl_e* afctrl);

    // 0x27
    enum ab1815_status_e set_batmodeio(enum ab1815_batmodeio_e mode);
    enum ab1815_status_e get_batmodeio(enum ab1815_batmodeio_e* mode);

    // 0x28
    enum ab1815_status_e get_id(struct ab1815_id_t *id);

    // 0x2F
    enum ab1815_status_e set_analog_status_register(ab1815_analog_status_t* analog_status);
    enum ab1815_status_e get_analog_status_register(ab1815_analog_status_t* analog_status);

    // 0x30
    enum ab1815_status_e set_output_control(ab1815_output_control_t* output_control);
    enum ab1815_status_e get_output_control(ab1815_output_control_t* output_control);

    // 0x3F
    enum ab1815_status_e set_extension_ram(extension_ram_t* extension_ram);
    enum ab1815_status_e get_extension_ram(extension_ram_t* extension_ram);


    void hex_dump(FILE* dump_to);

};

inline uint8_t bcd2bin(uint8_t value)
{
	return (value & 0x0F) + ((value >> 4) * 10);
}

inline uint8_t bin2bcd(uint8_t value)
{
	return ((value / 10) << 4) + value % 10;
}

#endif /* AB1815_H_ */


