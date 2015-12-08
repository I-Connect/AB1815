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

AB1815::AB1815(uint16_t cs_pin){
	this->cs_pin = cs_pin;
	pinMode(cs_pin, OUTPUT);
	pinMode(SS, OUTPUT);
	digitalWrite(cs_pin, HIGH);

	SPI.setBitOrder(MSBFIRST);
	SPI.setDataMode(SPI_MODE0);
	SPI.setClockDivider(SPI_CLOCK_DIV128);
	init();
}

enum ab1815_status AB1815::init()
{
	this->fields._12_24 = 0;
	this->fields.clk_source = 0;
	enum ab1815_status status = get_id(&this->id);
	if(status == ab1815_status_OK)
	{
		if(!(this->id.ID0 == 18 && (this->id.ID1 == 4 || this->id.ID1 == 5 || this->id.ID1 == 14 || this->id.ID1 == 15)))
		{
			printf("Invalid Clock ID.");
			status = ab1815_status_ERROR;
		}
		
	}
	return status; 	
};

void AB1815::spi_select_slave(bool select)
{
	if(select)
	{
		digitalWrite(cs_pin, LOW);
	}else
	{
		digitalWrite(cs_pin, HIGH);
	}
}

enum ab1815_status AB1815::read(uint8_t offset, uint8_t *buf, uint8_t length)
{
	uint8_t address = AB1815_SPI_READ(offset);
	enum ab1815_status ret_code = ab1815_status_OK;
	SPI.begin();
	spi_select_slave(true);
	SPI.transfer(address);
	uint8_t val = 0;
	for(uint16_t i = 0; i < length; i++)
	{
		 val = SPI.transfer(0);
		 buf[i] = val;
	}
	ret_code = ab1815_status_OK;
	spi_select_slave(false);
	SPI.end();
	return ret_code;
};

enum ab1815_status AB1815::write(uint8_t offset, uint8_t *buf, uint8_t length)
{

	uint8_t address = AB1815_SPI_WRITE(offset);
	enum ab1815_status ret_code = ab1815_status_OK;


	SPI.begin();
	spi_select_slave(true);

	SPI.transfer(address);
	for(uint16_t i = 0; i < length; i++)
	{
		SPI.transfer(buf[i]);
	}

	spi_select_slave(false);
	SPI.end();
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
enum ab1815_status AB1815::get_time(ab1815_tmElements_t *time)
{
			enum ab1815_status to_ret = ab1815_status_ERROR;
			size_t length = (AB1815_REG_ALARM_HUNDREDTHS - AB1815_REG_TIME_HUNDREDTHS) ;
			uint8_t buffer[length];
			memset(buffer, 0, length);
			if(read(AB1815_REG_TIME_HUNDREDTHS, buffer, length) == ab1815_status_OK)
			{
				to_ret = ab1815_status_OK;
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
enum ab1815_status AB1815::set_time(ab1815_tmElements_t *time)
{
	size_t length = (AB1815_REG_ALARM_HUNDREDTHS - AB1815_REG_TIME_HUNDREDTHS);
	uint8_t buffer[length];
	enum ab1815_status result = ab1815_status_ERROR;

	memset(buffer, 0, length);
	buffer[0] = bin2bcd(time->Hundredth);
	buffer[1] = bin2bcd(0x7F & time->Second);
	buffer[2] = bin2bcd(0x7F & time->Minute);
	buffer[3] = bin2bcd(0x3F & time->Hour);
	buffer[4] = bin2bcd(0x3F & time->Day);
	buffer[5] = bin2bcd(0x1F & time->Month);
	buffer[6] = bin2bcd(tmYearToY2k(time->Year));
	buffer[7] = bin2bcd(0x07 & time->Wday);

	if(write(AB1815_REG_TIME_HUNDREDTHS, buffer, length) == ab1815_status_OK)
	{
		result = ab1815_status_OK;
	}

	return result;
};

enum ab1815_status AB1815::clear_hundrdeds()
{
	uint8_t buf[1];
	buf[0] = 0;
	return write( AB1815_REG_TIME_HUNDREDTHS, buf, 1);
};

// 0x08
enum ab1815_status AB1815::get_alarm(ab1815_tmElements_t *time, enum ab1815_alarm_repeat_mode *alarm_mode)
{
	enum ab1815_status to_ret = ab1815_status_ERROR;
	size_t length = AB1815_REG_STATUS - AB1815_REG_ALARM_HUNDREDTHS ;
	uint8_t buffer[length];
	memset(buffer, 0, length);
	struct countdown_control_t cd_reg;
	uint32_t *val = (uint32_t*)alarm_mode;
	
	if(get_countdown_control(&cd_reg) == ab1815_status_OK)
	{
		if(read(AB1815_REG_ALARM_HUNDREDTHS, buffer, length) == ab1815_status_OK)
		{
			to_ret = ab1815_status_OK;
			time->Hundredth = bcd2bin(buffer[0]);
			time->Second = bcd2bin(0x7F & buffer[1]);
			time->Minute = bcd2bin(0x7F & buffer[2]);
			time->Hour = bcd2bin(0x3F & buffer[3]);
			time->Day = bcd2bin(0x3F & buffer[4]);
			time->Month = bcd2bin(0x1F & buffer[5]);
			time->Wday = bcd2bin(0x07 & buffer[6]);
		}
		*alarm_mode = (ab1815_alarm_repeat_mode)cd_reg.fields.RPT;
		if(cd_reg.fields.RPT == 7)
		{
			if((time->Hundredth & 0xF0) == 0xF0)
			{
				
				(*val)++;
				if((time->Hundredth & 0xFF) == 0xFF)
				{
					(*val)++;
				}
			}
		}
	}
	return to_ret;
};

// 0x08
enum ab1815_status AB1815::set_alarm(ab1815_tmElements_t *time, enum ab1815_alarm_repeat_mode alarm_mode)
{
	size_t length = AB1815_REG_STATUS - AB1815_REG_ALARM_HUNDREDTHS;
	uint8_t buffer[length];
	enum ab1815_status result = ab1815_status_ERROR;
	uint8_t repeat = alarm_mode;
	
	memset(buffer, 0, length);
	buffer[0] = bin2bcd(time->Hundredth);
	buffer[1] = bin2bcd(0x7F & time->Second);
	buffer[2] = bin2bcd(0x7F & time->Minute);
	buffer[3] = bin2bcd(0x3F & time->Hour);
	buffer[4] = bin2bcd(0x3F & time->Day);
	buffer[5] = bin2bcd(0x1F & time->Month);
	buffer[6] = bin2bcd(0x07 & time->Wday);
	
	switch(alarm_mode)
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
	
	if(write(AB1815_REG_ALARM_HUNDREDTHS, buffer, length) == ab1815_status_OK)
	{
		struct countdown_control_t cd_reg;
		
		if(get_countdown_control(&cd_reg) == ab1815_status_OK)
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
enum ab1815_status AB1815::set_status(status_t* status)
{
		return write(AB1815_REG_STATUS, &status->raw, 1);
};

enum ab1815_status AB1815::get_status(status_t* status)
{
			return read(AB1815_REG_STATUS, &status->raw, 1);
};

// 0x10
enum ab1815_status AB1815::set_control1(control1_t* control1)
{
	return write(AB1815_REG_CONTROL1, &control1->raw, 1);
};

enum ab1815_status AB1815::get_control1(control1_t* control1)
{
	return read(AB1815_REG_CONTROL1, &control1->raw, 1);
};

// 0x11
enum ab1815_status AB1815::set_control2(control2_t* control2)
{
	return write(AB1815_REG_CONTROL2, &control2->raw, 1);
};

enum ab1815_status AB1815::get_control2(control2_t* control2)
{
	return read(AB1815_REG_CONTROL2, &control2->raw, 1);
};

// 0x12
enum ab1815_status AB1815::set_interrupt_mask(inturrupt_mask_t* inturrupt_mask)
{
	return write(AB1815_REG_INTERRUPT_MASK, &inturrupt_mask->raw, 1);
};

enum ab1815_status AB1815::get_interrupt_mask(inturrupt_mask_t* inturrupt_mask)
{
	return read(AB1815_REG_INTERRUPT_MASK, &inturrupt_mask->raw, 1);
};

// 0x17 sleep_control_t
enum ab1815_status AB1815::set_sleep_control(sleep_control_t* sleep_control)
{
	return write(AB1815_REG_SLEEP_CONTROL, &sleep_control->raw, 1);
};

enum ab1815_status AB1815::get_sleep_control(sleep_control_t* sleep_control)
{
	return read( AB1815_REG_SLEEP_CONTROL, &sleep_control->raw, 1);
};

// 0x18
enum ab1815_status AB1815::set_countdown_control(countdown_control_t* countdown_control)
{
	return write(AB1815_REG_COUNTDOWN_TIMER_CONTROL, &countdown_control->raw, 1);
};

enum ab1815_status AB1815::get_countdown_control(countdown_control_t* countdown_control)
{
	return read(AB1815_REG_COUNTDOWN_TIMER_CONTROL, &countdown_control->raw, 1);
};

enum ab1815_status AB1815::get_id(ab1815_id_t *id)
{
	size_t length = AB1815_REG_ID6 - AB1815_REG_ID0; 
	uint8_t buffer[length];
	memset(buffer, 0, length);
	enum ab1815_status result = ab1815_status_ERROR;
	
	result = read(AB1815_REG_ID0, buffer, length);
	if(result == ab1815_status_OK)
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

enum ab1815_status AB1815::get_oscillator_control(struct oscillator_control_t *oscillator_control)
{
	return read(AB1815_REG_OSCILLATOR_CONTROL, &oscillator_control->value, 1);
};

enum ab1815_status AB1815::set_oscillator_control(struct oscillator_control_t *oscillator_control)
{
	if(set_configuration_key(ab1815_oscillator_control) != ab1815_status_OK)
	{
		return ab1815_status_ERROR;
	}
	Serial.print("Setting osc: ");
	Serial.println(oscillator_control->value);
	return write(AB1815_REG_OSCILLATOR_CONTROL, &oscillator_control->value, 1);
};


enum ab1815_status AB1815::set_configuration_key(enum configuration_key_t configuration_key)
{
	return write(AB1815_REG_CONFIGURATION_KEY, (uint8_t *)&configuration_key, 1);
};


enum ab1815_status AB1815::set_batmodeio(enum ab1815_batmodeio mode)
{
	if(set_configuration_key(ab1815_reg_control) != ab1815_status_OK)
	{
		return ab1815_status_ERROR;
	}
	uint8_t buf[1];
	buf[0] = mode;
	return write(AB1815_REG_BATMODE_IO, buf, 1);
};

void p(const char *fmt, ... ){
        char buf[128]; // resulting string limited to 128 chars
        va_list args;
        va_start (args, fmt );
        vsnprintf(buf, 128, fmt, args);
        va_end (args);
        Serial.print(buf);
}

void AB1815::hex_dump()
{

	uint8_t buffer[8];
//	p("\033[s");
	for(uint8_t pos = 0; pos < 0x7F; pos += 8)
	{
		read(pos, buffer, 8);

		p("# 0x%02x: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\r\n", pos, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);
	}
//	p("\033[u");
}





