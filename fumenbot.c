/*
 * FumenBot v0.71
 * http://fumenbot.sourceforge.net/
 *
 * Andres Basile GPLv3
 * http://www.gnu.org/licenses/gpl-3.0.en.html
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/twi.h>

#include "fumenbot.h"
#include "serial.h"
#include "time.h"
#include "i2c.h"
#include "adc.h"
#include "dio.h"

static char *MESSAGES[] =
{
    "CMD Not Found.",
    "RTC Syncing...",
    "RTC Error.",
    "ADC Querying...",
    "ADC Not Found.",
    "DIO Low.",
    "DIO High.",
    "DIO Not Found."
};
static char **msg;

void fumenbot_setup() {
	serial_setup();
	time_setup();
	i2c_setup();
	adc_setup();
	dio_set_out((uint8_t *)&DDRD, PORTD4); // 4 - Mosfet 4 // 12v (Water Pump)
	dio_set_out((uint8_t *)&DDRD, PORTD7); // 7 - Mosfet 5 // 12v (Cooler OUT)
	dio_set_out((uint8_t *)&DDRB, PORTB0); // 8 - Mosfet 6 //  12v (2r + 2b)
	dio_set_out((uint8_t *)&DDRB, PORTB4); // 12 - Mosfet 7 // 12v (Cooler IN)
	dio_set_pwm((uint8_t *)&TCCR0A, (1 << WGM01) | (1 << WGM00), (uint8_t *)&TCCR0B, (1 << CS01));
	dio_set_out((uint8_t *)&DDRD, PORTD6); // 6 - Mosfet 0 // 12v (2r + 2b)
	dio_set_out((uint8_t *)&DDRD, PORTD5); // 5 - Mosfet 1 // 10.4v (4r)
	dio_set_pwm((uint8_t *)&TCCR2A, (1 << WGM21) | (1 << WGM20), (uint8_t *)&TCCR2B, (1 << CS21));
	dio_set_out((uint8_t *)&DDRB, PORTB3); // 11 - Mosfet 2 // 10.4v (4r)
	dio_set_out((uint8_t *)&DDRD, PORTD3); // 3 - Mosfet 3 // 10.6v (4ir + 1uv)
	sei();
	serial_puts("Fumenbot v0.71\n\r");
	alarm_setup();
	serial_puts("Resuming...\n\r");
	alarm_resume((void *)&fumenbot_handler);
}

void fumenbot_loop() {
	while(1) {
		if(SERIAL_READY) {
			serial_puts((char *)fumenbot_handler((char*)SERIAL_BUFFER));
			serial_puts("\n\r");
			serial_flush();
		}
		if(TIME_NEXT_DAY) {
			time_up_D();
			serial_puts("\n\r- A NEW DAY ARISES -\n\r");
		}
		if(TIME_ALARM_READY) {
			serial_puts((char *)fumenbot_handler(alarm_cmd()));
			serial_puts("\n\r--- ALARM ---\n\r");
			alarm_next();
		}
		if(TIME_ALARM_RESUME) {
			TIME_ALARM_RESUME = 0;
			alarm_resume((void *)&fumenbot_handler);
			serial_puts("\n\r--- RESUME ---\n\r");
		}
		if(I2C_READY) {
			I2C_READY = 0;
			char *c = I2C_HANDLER();
			if(c) {
				serial_puts((char *)fumenbot_handler(c));
			}
			serial_puts("\n\r---- I2C ----\n\r");
		}
		if(ADC_READY) {
			ADC_READY = 0;
			serial_puts((char *)ADC_HANDLER((uint16_t *)&ADC_BUFFER));
			serial_puts("\n\r---- ADC ----\n\r");
		}
	}
}



char ** fumenbot_handler(char *in)
{
	char a = in[0];
	char b = in[1];

	if (a == 's') {
		msg = (char **)time_getc_c();
	}
	else if (a == 'S') {
		ds1307_read_init();
		msg = (char **)MESSAGES[1];
	}
	else if (a == 'd') {
		msg = (char **)time_getc();
	}
	else if (a == 'D') {
		time_setc(&in[1]);
		msg = (char **)time_getc();
	}
	else if (a == 'r') {
// GET RTC
	}
	else if (a == 'R') {
		ds1307_setc(&in[1]);
// SET RTC
		msg = (char **)MESSAGES[1];
	}
	else if (a == 'a') {
		if (b == '0') {
			ADC_HANDLER = (void *)&adc_lm35;
			adc_read(0);
			msg = (char **)MESSAGES[3];
		}
		else if (b == '1') {
//			adc_read(1, (void *)&adc_lm35);
			msg = (char **)MESSAGES[3];
		}
		else if (b == '2') {
//			adc_read(2, (void *)&adc_lm35);
			msg = (char **)MESSAGES[3];
		}
		else if (b == '3') {
//			adc_read(4, (void *)&adc_lm35);
			msg = (char **)MESSAGES[3];
		}
		else {
			msg = (char **)MESSAGES[4];
		}
	}
	else if (a == 'l') {
		if (b == '0') {
			dio_set_pwm_low((uint8_t *)&TCCR0A, COM0A1);
			msg = (char **)MESSAGES[5];
		}
		else if (b == '1') {
			dio_set_pwm_low((uint8_t *)&TCCR0A, COM0B1);
			msg = (char **)MESSAGES[5];
		}
		else if (b == '2') {
			dio_set_pwm_low((uint8_t *)&TCCR2A, COM2A1);
			msg = (char **)MESSAGES[5];
		}
		else if (b == '3') {
			dio_set_pwm_low((uint8_t *)&TCCR2A, COM2B1);
			msg = (char **)MESSAGES[5];
		}
		else if (b == '4') {
			dio_set_low((uint8_t *)&PORTB, PORTB0);
			msg = (char **)MESSAGES[5];
		}
		else {
			msg = (char **)MESSAGES[7];
		}
	}
	else if (a == 'L') {
		if (b == '0') {
			dio_set_pwm_hi((uint8_t *)&TCCR0A, (uint8_t *)&OCR0A, COM0A1, 255);
			msg = (char **)MESSAGES[6];
		}
		else if (b == '1') {
			dio_set_pwm_hi((uint8_t *)&TCCR0A, (uint8_t *)&OCR0B, COM0B1, 221);
			msg = (char **)MESSAGES[6];
		}
		else if (b == '2') {
			dio_set_pwm_hi((uint8_t *)&TCCR2A, (uint8_t *)&OCR2A, COM2A1, 221);
			msg = (char **)MESSAGES[6];
		}
		else if (b == '3') {
			dio_set_pwm_hi((uint8_t *)&TCCR2A, (uint8_t *)&OCR2B, COM2B1, 225);
			msg = (char **)MESSAGES[6];
		}
		else if (b == '4') {
			dio_set_hi((uint8_t *)&PORTB, PORTB0);
			msg = (char **)MESSAGES[6];
		}
		else {
			msg = (char **)MESSAGES[7];
		}	}
	else if (a == 'c') {
		if (b == '0') {
			dio_set_low((uint8_t *)&PORTD, PORTD7);
			msg = (char **)MESSAGES[5];
		}
		else if (b == '1') {
			dio_set_low((uint8_t *)&PORTB, PORTB4);
			msg = (char **)MESSAGES[5];
		}
		else {
			msg = (char **)MESSAGES[7];
		}
	}
	else if (a == 'C') {
		if (b == '0') {
			dio_set_hi((uint8_t *)&PORTD, PORTD7);
			msg = (char **)MESSAGES[6];
		}
		else if (b == '1') {
			dio_set_hi((uint8_t *)&PORTB, PORTB4);
			msg = (char **)MESSAGES[6];
		}
		else {
			msg = (char **)MESSAGES[7];
		}
	}
	else if (a == 'v') {
		if (b == '0') {
			dio_set_low((uint8_t *)&PORTD, PORTD4);
			msg = (char **)MESSAGES[5];
		}
		else {
			msg = (char **)MESSAGES[7];
		}
	}
	else if (a == 'V') {
		if (b == '0') {
			dio_set_hi((uint8_t *)&PORTD, PORTD4);
			msg = (char **)MESSAGES[6];
		}
		else {
			msg = (char **)MESSAGES[7];
		}
	}
	else {
		msg = (char **)MESSAGES[0];
	}

	return msg;
}

