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


#include "AB1815.h"
#include "SPI.h"
#include "stdarg.h"

AB1815::AB1815(uint16_t cs_pin) {
  this->cs_pin = cs_pin;
  pinMode(cs_pin, OUTPUT);
  pinMode(SS, OUTPUT);
  digitalWrite(cs_pin, HIGH);
  SPI.begin();
  init();
}

enum ab1815_status_e AB1815::init()
{
  this->fields._12_24 = 0;
  this->fields.clk_source = 0;
  enum ab1815_status_e status = get_id(&this->id);
  if (status == ab1815_status_e_OK)
  {
    if (!(this->id.ID0 == 18 && (this->id.ID1 == 4 || this->id.ID1 == 5 || this->id.ID1 == 14 || this->id.ID1 == 15)))
    {
      printf("Invalid Clock ID.");
      status = ab1815_status_e_ERROR;
    }

  }
  return status;
};

void AB1815::spi_select_slave(bool select)
{
  if (select)
  {
    digitalWrite(cs_pin, LOW);
  } else
  {
    digitalWrite(cs_pin, HIGH);
  }
}

enum ab1815_status_e AB1815::read(uint8_t offset, uint8_t* buf, uint8_t length)
{
  uint8_t address = AB1815_SPI_READ(offset);
  enum ab1815_status_e ret_code = ab1815_status_e_OK;
  SPI.beginTransaction(spiSettings);
  spi_select_slave(true);
  SPI.transfer(address);
  uint8_t val = 0;
  for (uint16_t i = 0; i < length; i++)
  {
    val = SPI.transfer(0);
    buf[i] = val;
  }
  ret_code = ab1815_status_e_OK;
  spi_select_slave(false);
  SPI.endTransaction();
  return ret_code;
};

enum ab1815_status_e AB1815::write(uint8_t offset, uint8_t* buf, uint8_t length)
{

  uint8_t address = AB1815_SPI_WRITE(offset);
  enum ab1815_status_e ret_code = ab1815_status_e_OK;

  SPI.beginTransaction(spiSettings);
  spi_select_slave(true);

  SPI.transfer(address);
  for (uint16_t i = 0; i < length; i++)
  {
    SPI.transfer(buf[i]);
  }

  spi_select_slave(false);
  SPI.endTransaction();
  return ret_code;
};

// 0x00
time_t AB1815::get()
{
  ab1815_tmElements_t tm;
  get_time(&tm);
  return makeTime(tm);
}

// 0x00
void AB1815::set(time_t time)
{
  ab1815_tmElements_t tm;
  breakTime(time, tm);
  set_time(&tm);
}

// 0x00
enum ab1815_status_e AB1815::get_time(ab1815_tmElements_t* time)
{
  enum ab1815_status_e to_ret = ab1815_status_e_ERROR;
  size_t length = (AB1815_REG_ALARM_HUNDREDTHS - AB1815_REG_TIME_HUNDREDTHS) ;
  uint8_t buffer[length];
  memset(buffer, 0, length);
  if (read(AB1815_REG_TIME_HUNDREDTHS, buffer, length) == ab1815_status_e_OK)
  {
    to_ret = ab1815_status_e_OK;
    time->Hundredth = bcd2bin(buffer[0]);
    time->Second = bcd2bin(0x7F & buffer[1]);
    time->Minute = bcd2bin(0x7F & buffer[2]);
    time->Hour = bcd2bin(0x3F & buffer[3]);
    time->Day = bcd2bin(0x3F & buffer[4]);
    time->Month = bcd2bin(0x1F & buffer[5]);
    time->Year = y2kYearToTm(bcd2bin(buffer[6]));
    time->Wday = bcd2bin(0x07 & buffer[7]);
  }
  return to_ret;
}

// 0x00
enum ab1815_status_e AB1815::set_time(ab1815_tmElements_t* time)
{
  size_t length = (AB1815_REG_ALARM_HUNDREDTHS - AB1815_REG_TIME_HUNDREDTHS);
  uint8_t buffer[length];
  enum ab1815_status_e result = ab1815_status_e_ERROR;

  memset(buffer, 0, length);
  buffer[0] = bin2bcd(time->Hundredth);
  buffer[1] = bin2bcd(0x7F & time->Second);
  buffer[2] = bin2bcd(0x7F & time->Minute);
  buffer[3] = bin2bcd(0x3F & time->Hour);
  buffer[4] = bin2bcd(0x3F & time->Day);
  buffer[5] = bin2bcd(0x1F & time->Month);
  buffer[6] = bin2bcd(tmYearToY2k(time->Year));
  buffer[7] = bin2bcd(0x07 & time->Wday);

  if (write(AB1815_REG_TIME_HUNDREDTHS, buffer, length) == ab1815_status_e_OK)
  {
    result = ab1815_status_e_OK;
  }

  return result;
};

enum ab1815_status_e AB1815::clear_hundrdeds()
{
  uint8_t buf[1];
  buf[0] = 0;
  return write( AB1815_REG_TIME_HUNDREDTHS, buf, 1);
};

// 0x08
enum ab1815_status_e AB1815::get_alarm(ab1815_tmElements_t* time, enum ab1815_alarm_repeat_mode* alarm_mode)
{
  enum ab1815_status_e to_ret = ab1815_status_e_ERROR;
  size_t length = AB1815_REG_STATUS - AB1815_REG_ALARM_HUNDREDTHS ;
  uint8_t buffer[length];
  memset(buffer, 0, length);
  struct countdown_control_t cd_reg;
  uint32_t* val = (uint32_t*)alarm_mode;

  if (get_countdown_control(&cd_reg) == ab1815_status_e_OK)
  {
    if (read(AB1815_REG_ALARM_HUNDREDTHS, buffer, length) == ab1815_status_e_OK)
    {
      to_ret = ab1815_status_e_OK;
      time->Hundredth = bcd2bin(buffer[0]);
      time->Second = bcd2bin(0x7F & buffer[1]);
      time->Minute = bcd2bin(0x7F & buffer[2]);
      time->Hour = bcd2bin(0x3F & buffer[3]);
      time->Day = bcd2bin(0x3F & buffer[4]);
      time->Month = bcd2bin(0x1F & buffer[5]);
      time->Wday = bcd2bin(0x07 & buffer[6]);
    }
    *alarm_mode = (ab1815_alarm_repeat_mode)cd_reg.fields.RPT;
    if (cd_reg.fields.RPT == 7)
    {
      if ((time->Hundredth & 0xF0) == 0xF0)
      {

        (*val)++;
        if ((time->Hundredth & 0xFF) == 0xFF)
        {
          (*val)++;
        }
      }
    }
  }
  return to_ret;
};

// 0x08
enum ab1815_status_e AB1815::set_alarm(ab1815_tmElements_t* time, enum ab1815_alarm_repeat_mode alarm_mode)
{
  size_t length = AB1815_REG_STATUS - AB1815_REG_ALARM_HUNDREDTHS;
  uint8_t buffer[length];
  enum ab1815_status_e result = ab1815_status_e_ERROR;
  uint8_t repeat = alarm_mode;

  memset(buffer, 0, length);
  buffer[0] = bin2bcd(time->Hundredth);
  buffer[1] = bin2bcd(0x7F & time->Second);
  buffer[2] = bin2bcd(0x7F & time->Minute);
  buffer[3] = bin2bcd(0x3F & time->Hour);
  buffer[4] = bin2bcd(0x3F & time->Day);
  buffer[5] = bin2bcd(0x1F & time->Month);
  buffer[6] = bin2bcd(0x07 & time->Wday);

  switch (alarm_mode)
  {
    case ab1815_alarm_repeat_once_per_tenth:
      repeat = 7;
      buffer[0] |= 0xF0;
      break;
    case ab1815_alarm_repeat_once_per_hundredth:
      repeat = 7;
      buffer[0] = 0xFF;
      break;
    default:
      repeat = alarm_mode;
  }

  if (write(AB1815_REG_ALARM_HUNDREDTHS, buffer, length) == ab1815_status_e_OK)
  {
    struct countdown_control_t cd_reg;

    if (get_countdown_control(&cd_reg) == ab1815_status_e_OK)
    {
      cd_reg.fields.RPT = repeat;
      return set_countdown_control(&cd_reg);
    }
  }
  return result;
};

// 0x0F - See also: ARST in Control1.
//	If ARST is a 1, a read of the Status register will produce the current state of all
//	the interrupt flags and then clear them
enum ab1815_status_e AB1815::set_status(status_t* status)
{
  return write(AB1815_REG_STATUS, &status->value, 1);
};

enum ab1815_status_e AB1815::get_status(status_t* status)
{
  return read(AB1815_REG_STATUS, &status->value, 1);
};

// 0x10
enum ab1815_status_e AB1815::set_control1(control1_t* control1)
{
  return write(AB1815_REG_CONTROL1, &control1->value, 1);
};

enum ab1815_status_e AB1815::get_control1(control1_t* control1)
{
  return read(AB1815_REG_CONTROL1, &control1->value, 1);
};

// 0x11
enum ab1815_status_e AB1815::set_control2(control2_t* control2)
{
  return write(AB1815_REG_CONTROL2, &control2->value, 1);
};

enum ab1815_status_e AB1815::get_control2(control2_t* control2)
{
  return read(AB1815_REG_CONTROL2, &control2->value, 1);
};

// 0x12
enum ab1815_status_e AB1815::set_interrupt_mask(inturrupt_mask_t* inturrupt_mask)
{
  return write(AB1815_REG_INTERRUPT_MASK, &inturrupt_mask->value, 1);
};

enum ab1815_status_e AB1815::get_interrupt_mask(inturrupt_mask_t* inturrupt_mask)
{
  return read(AB1815_REG_INTERRUPT_MASK, &inturrupt_mask->value, 1);
};

// 0x13
enum ab1815_status_e AB1815::set_square_wave(square_wave_t* square_wave)
{
  return write(AB1815_REG_SQW, &square_wave->value, 1);
}

enum ab1815_status_e AB1815::get_square_wave(square_wave_t* square_wave)
{
  return read(AB1815_REG_SQW, &square_wave->value, 1);
}

// 0x14
enum ab1815_status_e AB1815::set_cal_xt(cal_xt_t* cal_xt)
{
  return write(AB1815_REG_CAL_XT, &cal_xt->value, 1);
}

enum ab1815_status_e AB1815::get_cal_xt(cal_xt_t* cal_xt)
{
  return read(AB1815_REG_CAL_XT, &cal_xt->value, 1);
}

// 0x15
enum ab1815_status_e AB1815::set_cal_rc_hi(cal_rc_hi_t* cal_rc_hi)
{
  return write(AB1815_REG_CAL_RC_HI, &cal_rc_hi->value, 1);
}

enum ab1815_status_e AB1815::get_cal_rc_hi(cal_rc_hi_t* cal_rc_hi)
{
  return read(AB1815_REG_CAL_RC_HI, &cal_rc_hi->value, 1);
}

// 0x16
enum ab1815_status_e AB1815::set_cal_rc_low(cal_rc_low_t* cal_rc_low)
{
  return write(AB1815_REG_CAL_RC_LOW, &cal_rc_low->OFFSETR, 1);
}

enum ab1815_status_e AB1815::get_cal_rc_low(cal_rc_low_t* cal_rc_low)
{
  return read(AB1815_REG_CAL_RC_LOW, &cal_rc_low->OFFSETR, 1);
}

// 0x17 sleep_control_t
enum ab1815_status_e AB1815::set_sleep_control(sleep_control_t* sleep_control)
{
  return write(AB1815_REG_SLEEP_CONTROL, &sleep_control->value, 1);
};

enum ab1815_status_e AB1815::get_sleep_control(sleep_control_t* sleep_control)
{
  return read( AB1815_REG_SLEEP_CONTROL, &sleep_control->value, 1);
};

// 0x18
enum ab1815_status_e AB1815::set_countdown_control(countdown_control_t* countdown_control)
{
  return write(AB1815_REG_COUNTDOWN_TIMER_CONTROL, &countdown_control->value, 1);
};

enum ab1815_status_e AB1815::get_countdown_control(countdown_control_t* countdown_control)
{
  return read(AB1815_REG_COUNTDOWN_TIMER_CONTROL, &countdown_control->value, 1);
};

// 0x19
enum ab1815_status_e AB1815::set_countdown_timer(uint8_t timer_value)
{
  return write(AB1815_REG_COUNTDOWN_TIMER, &timer_value, 1);
}

enum ab1815_status_e AB1815::get_countdown_timer(uint8_t* timer_value)
{
  return read(AB1815_REG_COUNTDOWN_TIMER, timer_value, 1);
}

// 0x1A
enum ab1815_status_e AB1815::set_countdown_timer_initial_value(uint8_t timer_value)
{
  return write(AB1815_REG_COUNTDOWN_TIMER_INITIAL, &timer_value, 1);
}

enum ab1815_status_e AB1815::get_countdown_timer_initial_value(uint8_t* timer_value)
{
  return read(AB1815_REG_COUNTDOWN_TIMER_INITIAL, timer_value, 1);
}

// 0x1B
enum ab1815_status_e AB1815::set_watchdog_timer(watchdog_timer_t* watchdog_timer)
{
  return write(AB1815_REG_WATCHDOG_TIMER, &watchdog_timer->value, 1);
}

enum ab1815_status_e AB1815::get_watchdog_timer(watchdog_timer_t* watchdog_timer)
{
  return read(AB1815_REG_WATCHDOG_TIMER, &watchdog_timer->value, 1);
}


// 0x1C Get the oscillator control register
enum ab1815_status_e AB1815::get_oscillator_control(struct oscillator_control_t* oscillator_control)
{
  return read(AB1815_REG_OSCILLATOR_CONTROL, &oscillator_control->value, 1);
};

enum ab1815_status_e AB1815::set_oscillator_control(struct oscillator_control_t* oscillator_control)
{
  if (set_configuration_key(ab1815_oscillator_control) != ab1815_status_e_OK)
  {
    return ab1815_status_e_ERROR;
  }
  return write(AB1815_REG_OSCILLATOR_CONTROL, &oscillator_control->value, 1);
};


// 0x1D
enum ab1815_status_e AB1815::set_oscillator_status(oscillator_status_t* oscillator_status)
{
  return write(AB1815_REG_OSCILLATOR_STATUS, &oscillator_status->value, 1);
}

enum ab1815_status_e AB1815::get_oscillator_status(oscillator_status_t* oscillator_status)
{
  return read(AB1815_REG_OSCILLATOR_STATUS, &oscillator_status->value, 1);
}

// 0x1E - Nothing on the AB1815
// 0x1F
enum ab1815_status_e AB1815::set_configuration_key(enum configuration_key_e configuration_key)
{
  return write(AB1815_REG_CONFIGURATION_KEY, (uint8_t*)&configuration_key, 1);
};

// 0x20
enum ab1815_status_e AB1815::set_trickle(trickle_t* trickle)
{
  return write(AB1815_REG_TRICKLE_CONTROL, &trickle->value, 1);
}

enum ab1815_status_e AB1815::get_trickle(trickle_t* trickle)
{
  return read(AB1815_REG_TRICKLE_CONTROL, &trickle->value, 1);
}

// 0x21
enum ab1815_status_e AB1815::set_bref_control(bref_control_t* bref_control)
{
  return write(AB1815_REG_BREF_CONTROL, &bref_control->value, 1);
}

enum ab1815_status_e AB1815::get_bref_control(bref_control_t* bref_control)
{
  return read(AB1815_REG_BREF_CONTROL, &bref_control->value, 1);
}

// 0x26
enum ab1815_status_e AB1815::set_afctrl(afctrl_e afctrl)
{
  return write(AB1815_REG_AFCTRL, (uint8_t*)&afctrl, 1);
}

enum ab1815_status_e AB1815::get_(afctrl_e* afctrl)
{
  return read(AB1815_REG_AFCTRL, (uint8_t*)afctrl, 1);
}

// 0x27
enum ab1815_status_e AB1815::set_batmodeio(enum ab1815_batmodeio_e mode)
{
  if (set_configuration_key(ab1815_reg_control) != ab1815_status_e_OK)
  {
    return ab1815_status_e_ERROR;
  }
  uint8_t buf[1];
  buf[0] = mode;
  return write(AB1815_REG_BATMODE_IO, buf, 1);
};

enum ab1815_status_e AB1815::get_batmodeio(enum ab1815_batmodeio_e* mode)
{
  return read(AB1815_REG_BATMODE_IO, (uint8_t*)mode, 1);
}

// 0x28
enum ab1815_status_e AB1815::get_id(ab1815_id_t* id)
{
  size_t length = AB1815_REG_ID6 - AB1815_REG_ID0;
  uint8_t buffer[length];
  memset(buffer, 0, length);
  enum ab1815_status_e result = ab1815_status_e_ERROR;

  result = read(AB1815_REG_ID0, buffer, length);
  if (result == ab1815_status_e_OK)
  {
    id->ID0 = bcd2bin(buffer[0]);
    id->ID1 = bcd2bin(buffer[1]);
    id->ID2.value = buffer[2];
    id->ID3 = bcd2bin(buffer[3]);
    id->ID4 = bcd2bin(buffer[4]);
    id->ID5 = bcd2bin(buffer[5]);
    id->ID6 = bcd2bin(buffer[6]);

  }

  return result;

};

// 0x2F
enum ab1815_status_e AB1815::set_analog_status_register(ab1815_analog_status_t* analog_status)
{
  return write(AB1815_REG_ANALOG_STATUS, &analog_status->value, 1);
}

enum ab1815_status_e AB1815::get_analog_status_register(ab1815_analog_status_t* analog_status)
{
  return read(AB1815_REG_ANALOG_STATUS, &analog_status->value, 1);
}

// 0x30
enum ab1815_status_e AB1815::set_output_control(ab1815_output_control_t* output_control)
{
  return write(AB1815_REG_OUTPUT_CONTROL, &output_control->value, 1);
}

enum ab1815_status_e AB1815::get_output_control(ab1815_output_control_t* output_control)
{
  return read(AB1815_REG_OUTPUT_CONTROL, &output_control->value, 1);
}


// 0x3F
enum ab1815_status_e AB1815::set_extension_ram(extension_ram_t* extension_ram)
{
  return write(AB1815_EXTENTION_RAM, &extension_ram->value, 1);
}

enum ab1815_status_e AB1815::get_extension_ram(extension_ram_t* extension_ram)
{
  return read(AB1815_EXTENTION_RAM, &extension_ram->value, 1);
}


void AB1815::hex_dump(FILE* dump_to)
{

  uint8_t buffer[8];
  for (uint8_t pos = 0; pos < 0x7F; pos += 8)
  {
    read(pos, buffer, 8);
    fprintf(dump_to, "# 0x%02x: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\r\n", pos, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);
  }
}





