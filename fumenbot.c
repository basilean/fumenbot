/*******

FumenBot v0.7
http://fumenbot.sourceforge.net/

Andres Basile GPLv3

*******/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <stdio.h>

#include "fumenbot.h"
#include "main.h"
#include "serial.h"
#include "ds1307.h"

/*

FumenBot Configuration / Definition

Here is where you can configure different pins or try porting it to a bigger atmega.

*/

fumenbot_t fumenbot = {
	{ // Sensors Input
		{0} // A0 - LM35 (temp)
	},
	{
		{2}, // A2 - 10k (lrd)
		{3} // A3 - 10k (lrd)
	},
	{
		{1} // A1 - 1k (water)
	},
	{ // Led
		{PORTB5, &PORTB, &DDRB} // 13 - Led
	},
	{ // Output
		{PORTD4, &PORTD, &DDRD}, // 4 - Mosfet 4 // 12v (2r + 2b)
		{PORTD7, &PORTD, &DDRD}, // 7 - Mosfet 5 // 12v (Cooler OUT)
		{PORTB0, &PORTB, &DDRB}, // 8 - Mosfet 6 // 12v (Cooler IN)
		{PORTB4, &PORTB, &DDRB} // 12 - Mosfet 7 // 12v (Water Pump)

	},
	{ // PWM Output
		{PORTD6, &PORTD, &DDRD, COM0A1, &TCCR0A, &OCR0A, 255, 5, 1}, // 6 - Mosfet 0 // 12v (2r + 2b)
		{PORTD5, &PORTD, &DDRD, COM0B1, &TCCR0A, &OCR0B, 221, 5, 1}, // 5 - Mosfet 1 // 10.4v (4r)
		{PORTB3, &PORTB, &DDRB, COM2A1, &TCCR2A, &OCR2A, 221, 5, 1}, // 11 - Mosfet 2 // 10.4v (4r)
		{PORTD3, &PORTD, &DDRD, COM2B1, &TCCR2A, &OCR2B, 225, 5, 1} // 3 - Mosfet 3 // 10.6v (4ir + 1uv)
	},
	0,
	1,
	{4, 5} // A4, A5 - RTC
};

static uint8_t FUMENBOT_TEMP_COUNT = sizeof(fumenbot.temp) / sizeof(fumenbot.temp[0]);
static uint8_t FUMENBOT_LRD_COUNT = sizeof(fumenbot.lrd) / sizeof(fumenbot.lrd[0]);
static uint8_t FUMENBOT_WATER_COUNT = sizeof(fumenbot.water) / sizeof(fumenbot.water[0]);
static uint8_t FUMENBOT_LIGHT_COUNT = sizeof(fumenbot.light) / sizeof(fumenbot.light[0]);
static uint8_t FUMENBOT_VALVE_COUNT = sizeof(fumenbot.valve) / sizeof(fumenbot.valve[0]);
static uint8_t FUMENBOT_LED_COUNT = sizeof(fumenbot.led) / sizeof(fumenbot.led[0]);

void fumenbot_setup(uint8_t rtc)
{
	fumenbot.rtc = rtc;
	// Setup IO
	int i;
	for(i=0; i < FUMENBOT_LIGHT_COUNT; i++) {
		*(fumenbot.light[i].reg) |= (1<<fumenbot.light[i].bit);
		*(fumenbot.light[i].port) &= ~(1<<fumenbot.light[i].bit);
	}
	for(i=0; i < FUMENBOT_VALVE_COUNT; i++) {
		*(fumenbot.valve[i].reg) |= (1<<fumenbot.valve[i].bit);
		*(fumenbot.valve[i].port) &= ~(1<<fumenbot.valve[i].bit);
	}
	for(i=0; i < FUMENBOT_LED_COUNT; i++) {
		*(fumenbot.led[i].reg) |= (1<<fumenbot.led[i].bit);
		*(fumenbot.led[i].port) &= ~(1<<fumenbot.led[i].bit);
	}

	// Setup PWM
	TCCR0A |= (1 << WGM01) | (1 << WGM00);
	TCCR0B |= (1 << CS01);

	TCCR2A |= (1 << WGM21) | (1 << WGM20);
	TCCR2B |= (1 << CS21);
}

// This function was taken from a post.
// I'm going to add proper link when I found it back.
uint16_t aio_read(uint8_t pin)
{
    int ADCval;

    ADMUX = pin;         // use #1 ADC
    ADMUX |= (1 << REFS0);    // use AVcc as the reference
    ADMUX &= ~(1 << ADLAR);   // clear for 10 bit resolution
  
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);    // 128 prescale for 8Mhz
    ADCSRA |= (1 << ADEN);    // Enable the ADC

    ADCSRA |= (1 << ADSC);    // Start the ADC conversion

    while(ADCSRA & (1 << ADSC));      // Thanks T, this line waits for the ADC to finish 


    ADCval = ADCL;
    ADCval = (ADCH << 8) + ADCval;    // ADCH is read so ADC can be updated again

    return ADCval;
}

// Nasty global.
static char out[200] = "";
char * fumenbot_handler(char *in)
{
	char a = in[0];
	uint8_t n = in[1] - '0';
	uint8_t d = in[2] - '0';

	if (a == 'l') {
		if (n > FUMENBOT_LIGHT_COUNT - 1) {
			sprintf(out, "Light %d not found, try 0 to %d\n\r", n, FUMENBOT_LIGHT_COUNT - 1);
		}
		else {
			if (d < 1 || d > 9) {
				*(fumenbot.light[n].timer) &= ~(1<<fumenbot.light[n].com);
				*(fumenbot.light[n].port) &= ~(1<<fumenbot.light[n].bit);
				sprintf(out, "Light %d OFF\n\r", n);
			}
			else {
				*(fumenbot.light[n].timer) |= (1<<fumenbot.light[n].com);
				*(fumenbot.light[n].duty) = fumenbot.light[n].vol / d;
				sprintf(out, "Light %d ON\n\r", n);
			}
		}
	}

	else if (a == 'v') {
		if (n > FUMENBOT_VALVE_COUNT - 1) {
			sprintf(out, "Valve %d not found, try 0 to %d\n\r", n, FUMENBOT_VALVE_COUNT - 1);
		}
		else {
			if (d < 1 || d > 9) {
				*(fumenbot.valve[n].port) &= ~(1<<fumenbot.valve[n].bit);
				sprintf(out, "Valve %d OFF\n\r", n);
			}
			else {
				*(fumenbot.valve[n].port) |= (1<<fumenbot.valve[n].bit);
				sprintf(out, "Valve %d ON\n\r", n);
			}
		}
	}

	else if (a == 'e') {
		if (n > FUMENBOT_LED_COUNT - 1) {
			sprintf(out, "Led %d not found, try 0 to %d\n\r", n, FUMENBOT_LED_COUNT - 1);
		}
		else {
			if (d < 1 || d > 9) {
				*(fumenbot.led[n].port) &= ~(1<<fumenbot.led[n].bit);
				sprintf(out, "Led %d OFF\n\r", n);
			}
			else {
				*(fumenbot.led[n].port) |= (1<<fumenbot.led[n].bit);
				sprintf(out, "Led %d ON\n\r", n);
			}
		}
	}

	else if (a == 't') {
		if (n > FUMENBOT_TEMP_COUNT - 1) {
			sprintf(out, "Temp Sensor %d not found, try 0 to %d\n\r", n, FUMENBOT_TEMP_COUNT - 1);
		}
		else {
			uint16_t temp = aio_read(fumenbot.temp[n].bit); // temperature = (5.0 * lm35read * 100.0)/1024.0;
			sprintf(out, "Temp: %d.%d\n\r", 500 * temp / 1024, (500 * temp % 1024) * 10 / 1024);

		}
	}

	else if (a == 'w') {
		if (n > FUMENBOT_WATER_COUNT - 1) {
			sprintf(out, "Water Sensor %d not found, try 0 to %d\n\r", n, FUMENBOT_WATER_COUNT - 1);
		}
		else {
			uint16_t water = aio_read(fumenbot.water[n].bit);
			sprintf(out, "Water: %d\n\r", 500 * water / 1024);
		}
	}

	else if (a == 'r') {
		if (n > FUMENBOT_LRD_COUNT - 1) {
			sprintf(out, "Light Sensor %d not found, try 0 to %d\n\r", n, FUMENBOT_LRD_COUNT - 1);
		}
		else {
			uint16_t lumen = aio_read(fumenbot.lrd[n].bit);
			sprintf(out, "Lumens: %d\n\r", 500 * lumen / 1024);
		}
	}

	else if (a == 'd') {
		if(fumenbot.rtc) {
			time_t time;
			ds1307_getdate(&time.Y, &time.M, &time.D, &time.h, &time.m, &time.s);
			sprintf(out, "%d/%d/%d %d:%d:%d\n\r", time.Y, time.M, time.D, time.h, time.m, time.s);
		}
		else {
			sprintf(out, "RTC not found.\n\r");
		}
	}

	else if (a == 'D') {
		if(fumenbot.rtc) {
			// MMDDhhmmYYss
			uint8_t M = 10 * (in[1] - '0');
			uint8_t MM = M + (in[2] - '0');
			uint8_t D = 10 * (in[3] - '0');
			uint8_t DD = D + (in[4] - '0');
			uint8_t h = 10 * (in[5] - '0');
			uint8_t hh = h + (in[6] - '0');
	  		uint8_t m = 10 * (in[7] - '0');
			uint8_t mm = m + (in[8] - '0');
			uint8_t Y = 10 * (in[9] - '0');
			uint8_t YY = Y + (in[10] - '0');
			uint8_t s = 10 * (in[11] - '0');
			uint8_t ss = s + (in[12] - '0');
			ds1307_setdate(YY, MM, DD, hh, mm, ss);
			time_t time;
			ds1307_getdate(&time.Y, &time.M, &time.D, &time.h, &time.m, &time.s);
			sprintf(out, "DATE set: %d/%d/%d %d:%d:%d\n\r", time.Y, time.M, time.D, time.h, time.m, time.s);
		}
		else {
			sprintf(out, "RTC not found.\n\r");
		}
	}

	else if (a == 'h') {
		sprintf(out, "HELP:\n\r(D)MMDDhhmmYYss -> set date\n\r(d)ate\n\r(l)ight(0..x)num(0..9)state\n\r(v)alve(0..x)(0|1)\n\r(e)led(0|1)(0..x)\n\r(t)emp(0..x)\n\r(w)ater(0..x)\n\r(r)lrd(0..x)\n\r");
	}

	else {
		sprintf(out, "UNKNOWN -> %s\n\r(h) -> help\n\r", in);
	}

	return out;
}

/*
!!! PARENTAL ADVISOR !!!

This is a nasty way to run cron alike.

Remember to increaze TIME_ALARM_SIZE if you add new ones.
*/

static time_alarm_t alarm[TIME_ALARM_SIZE] = {
{1, 1, 0, 2, "e01     "}, // Turn On Led 0 at 1:00:02 Hs.
{1, 1, 0, 3, "l00     "}, // Turn Off Light 0 at 1:00:03 Hs. Just one sec after led.
{1, 1, 0, 4, "l10     "},
{1, 1, 0, 5, "l20     "},
{1, 1, 0, 6, "l30     "},
{1, 1, 0, 7, "v20     "},
{1, 2, 0, 0, "v01     "}, // 10 secs of water.
{1, 2, 0, 10, "v00     "},
{1, 5, 0, 2, "e00     "},
{1, 5, 0, 3, "l01     "}, // Turn On Light 0 at 5:00:03 Hs. 4 hours dark.
{1, 5, 0, 4, "l11     "},
{1, 5, 0, 5, "l21     "},
{1, 5, 0, 6, "l31     "},
{1, 5, 0, 7, "v21     "},
{1, 25, 10, 0, "v11     "}, // Using '25' hour as wildcard for each hour run coolers.
{1, 25, 10, 0, "v31     "},
{1, 25, 15, 0, "v10     "},
{1, 25, 15, 0, "v30     "},
};

void time_alarm_check() {
	time_t time;
	ds1307_getdate(&time.Y, &time.M, &time.D, &time.h, &time.m, &time.s);
	int i;
	for(i=0; i < TIME_ALARM_SIZE; i++) {
		if(alarm[i].enabled) {
			if((alarm[i].h == time.h) || (alarm[i].h == 25)) {
				if(alarm[i].m == time.m) {
					if(alarm[i].s == time.s) {
						fumenbot_handler((char*)alarm[i].cmd);
					}
				}
			}
		}
	}
}

// Run two times over schedule list simulating a whole past day to catch up.
void fumenbot_resume()
{
	time_t time;
	ds1307_getdate(&time.Y, &time.M, &time.D, &time.h, &time.m, &time.s);
	int i;
	for(i=0; i < TIME_ALARM_SIZE; i++) {
		if(alarm[i].enabled) {
			if(alarm[i].h > time.h) {
				fumenbot_handler((char*)alarm[i].cmd);
			}
		}
	}
	for(i=0; i < TIME_ALARM_SIZE; i++) {
		if(alarm[i].enabled) {
			if(alarm[i].h < time.h) {
				fumenbot_handler((char*)alarm[i].cmd);
			}
			else if(((alarm[i].h == time.h) || (alarm[i].h == 25)) && (alarm[i].m <= time.m)) {
				fumenbot_handler((char*)alarm[i].cmd);
			}
			else {
			}
		}
	}
}

