/*******

FumenBot v0.7
http://fumenbot.sourceforge.net/

Andres Basile GPLv3

*******/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/twi.h>
#include "util/delay.h"

#include "main.h"
#include "fumenbot.h"
#include "serial.h"

#include "i2cmaster.h"
// Modified one line to avoid warnings:
// From: TWBR = ((F_CPU/SCL_CLOCK)-16)/2;  /* must be > 10 for stable operation */
// To: TWBR = (uint8_t)((F_CPU/SCL_CLOCK)-16)/2;  /* must be > 10 for stable operation */

#include "ds1307.h"

// I want to initialize i2c myself to check if RTC exists.
// "Initialization of the I2C bus interface. Need to be called only once" <- twimaster.c
#undef DS1307_I2CINIT
#define DS1307_I2CINIT == 0

uint8_t check_rtc()
{
	i2c_init();
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
	_delay_us(100);
	if (TWCR & (1<<TWINT)) {
		return 1;
	}
	else {
		return 0;
	}
}

int main()
{
	uint8_t rtc = 0;
	serial_setup();
	serial_puts("FumenBot v0.7\n\r");
	if(check_rtc()) {
		rtc = 1;
		ds1307_init();
	}
	fumenbot_setup(rtc);
	if(rtc) {
		serial_puts("RTC found.\n\r");
		fumenbot_resume();
		serial_puts("Resuming.......\n\r\n\r");
	}
	else {
		serial_puts("RTC not found.\n\r\n\r");
	}
	sei();
	while(1) {
		if(serial_go == 1) {
			serial_puts(fumenbot_handler((char*)serial_buffer));
			serial_flush();
		}
		if(rtc) {
			time_alarm_check();
		}
	}
	return 0;
}

