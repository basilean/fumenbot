/*
 * FumenBot v0.71
 * http://fumenbot.sourceforge.net/
 *
 * Andres Basile GPLv3
 * http://www.gnu.org/licenses/gpl-3.0.en.html
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "time.h"

static alarm_t TIME_ALARM[] = {
{1, 0, "S"}, // 600s = 0:10 | Sync RTC
{900, 3600, "C0"}, // Fisrt at 900s(0:15) then interval each 3600s (1h) | ON cooler
{902, 3600, "C1"},
{1200, 3600, "c0"}, // Fisrt at 900s(0:20) then interval each 3600s (1h) | OFF cooler
{1202, 3600, "c1"},
{4200, 0, "l0"}, // 4200s = 1:10 | OFF Lights
{4202, 0, "l1"}, //
{4204, 0, "l2"}, //
{4206, 0, "l3"}, //
{4208, 0, "l4"}, //
{7800, 0, "V0"}, // 7800 = 2:10 | ON Water
{7810, 0, "v0"}, // 7810 = 2:10:10 | OFF Water
{18600, 0, "L0"}, // 4200s = 4:10 | ON Lights
{18602, 0, "L1"}, //
{18604, 0, "L2"}, //
{18606, 0, "L3"}, //
{18608, 0, "L4"}, //
};

volatile uint32_t TIME_COUNT = 0; // u32 max 4294967296
volatile uint8_t TIME_ALARM_READY = 0;
volatile uint8_t TIME_NEXT_DAY = 0;
const uint8_t TIME_MONTH[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static char TIME_LCD[] = "MMDDhhmmYYss";
static char *pTIME_LCD = &TIME_LCD[0];
static uint8_t TIME_DATE[3] = {16, 1, 7};
static uint8_t TIME_ALARM_COUNT = sizeof(TIME_ALARM)/sizeof(alarm_t);
static uint8_t TIME_ALARM_ENABLED = 0;
volatile uint8_t TIME_ALARM_RESUME = 0;
static uint32_t TIME_ALARM_NEXT = 86400;
static char * TIME_ALARM_RUNCMD;

/////// TIMER

void time_setup() {
	TCCR1B |= ((1 << CS10) | (1 << CS12) | (1 << WGM12));
	OCR1A = (F_CPU / 1024) - 1; // 1 sec
	TIMSK1 |= (1 << OCIE1A);
}

ISR(TIMER1_COMPA_vect) {
	if(TIME_COUNT == 86399) {
		TIME_COUNT = 0;
		TIME_NEXT_DAY = 1;
	}
	else {
		++TIME_COUNT;
	}
	if((TIME_ALARM_ENABLED) && (TIME_ALARM_NEXT == TIME_COUNT)) {
		TIME_ALARM_READY = 1;
	}
}

void time_up_D() {
	TIME_NEXT_DAY = 0;
	if (TIME_DATE[2] >= TIME_MONTH[(TIME_DATE[1] - 1)]) {
		TIME_DATE[2] = 1;
		time_up_M();
	}
	else {
		++TIME_DATE[2];
	}
	if(TIME_ALARM_ENABLED) {
		alarm_next();
	}
}

void time_up_M() {
	if (TIME_DATE[1] >= 12) {
		TIME_DATE[1] = 1;
		time_up_Y();
	}
	else {
		++TIME_DATE[1];
	}
}

void time_up_Y() {
	++TIME_DATE[0];
}

uint8_t * time_cc2u(char * c, uint8_t * u) {
	*(u) = ((c[0] - '0') * 10) + (c[1] - '0');
	return u;
}

char * time_u2cc(uint8_t * u, char * c) {
	*(c) = (char)(*(u) / 10) + '0';
	*(c + 1) = (char)(*(u) % 10) + '0';
	*(c + 2) = '\0';
	return c;
}

char * time_uuuu2c(uint32_t * u, char * c) {
	uint32_t i;
	uint32_t t = *(u);
	uint8_t ii = 0;
	for(i=1000000000;i>0;i/=10) {
		*(c + ii) = (char)(t / i) + '0';
		t %= i;
		++ii;
	}
	*(c + ii) = '\0';
	return c;
}

uint8_t * time_get_h(uint32_t * u, uint8_t * h) {
	*(h) = *(u) / 3600;
	return h;
}

uint8_t * time_get_m(uint32_t * u, uint8_t * m) {
	*(m) = (*(u) % 3600) / 60;
	return m;
}

uint8_t * time_get_s(uint32_t * u, uint8_t * s) {
	*(s) = (*(u) % 3600) % 60;
	return s;
}

char * time_getc_s(uint32_t * u, char * c) {
	uint8_t s;
	time_u2cc(time_get_s(u, &s), c);
	return c;
}

char * time_getc_m(uint32_t * u, char * c) {
	uint8_t m;
	time_u2cc(time_get_m(u, &m), c);
	return c;
}

char * time_getc_h(uint32_t * u, char * c) {
	uint8_t h;
	time_u2cc(time_get_h(u, &h), c);
	return c;
}

char * time_getc_c() {
	time_uuuu2c((uint32_t *)&TIME_COUNT, pTIME_LCD);
	return pTIME_LCD;
}

uint32_t * time_set_count(uint32_t * count, uint8_t * h, uint8_t * m, uint8_t * s) {
	*(count) = *(h) *(uint32_t) 3600;
	*(count) += *(m) *(uint32_t) 60;
	*(count) += *(s);
	return count;
}

void time_setc(char * c) {
	time_cc2u(c, &TIME_DATE[1]); // Month
	time_cc2u((c + 2), &TIME_DATE[2]); // Day
	time_cc2u((c + 8), &TIME_DATE[0]); // Year
	uint8_t h, m, s;
	time_set_count(
		(uint32_t *)&TIME_COUNT,
		time_cc2u((c + 4), &h),
		time_cc2u((c + 6), &m),
		time_cc2u((c + 10), &s)
	);
	if(TIME_ALARM_ENABLED) {
		TIME_ALARM_RESUME = 1;
		alarm_next();
	}
}

char * time_getc() {
	time_u2cc(&TIME_DATE[1], pTIME_LCD);
	time_u2cc(&TIME_DATE[2], (pTIME_LCD + 2));
	time_getc_h((uint32_t *)&TIME_COUNT, (pTIME_LCD + 4));
	time_getc_m((uint32_t *)&TIME_COUNT, (pTIME_LCD + 6));
	time_u2cc(&TIME_DATE[0], (pTIME_LCD + 8));
	time_getc_s((uint32_t *)&TIME_COUNT, (pTIME_LCD + 10));
	return TIME_LCD;
}

/////// ALARM

void alarm_setup() {
	TIME_ALARM_ENABLED = 1;
	alarm_next();
}

void alarm_next() {
	TIME_ALARM_READY = 0;
	uint8_t i;
	uint32_t s;
	uint32_t c;
	uint32_t cb = 86400;
	for(i=0;i<TIME_ALARM_COUNT;i++) {
		if (TIME_ALARM[i].interval) {
			s = ((TIME_COUNT / TIME_ALARM[i].interval) * TIME_ALARM[i].interval) + TIME_ALARM[i].sec;
		}

		else {
			s = TIME_ALARM[i].sec;
		}
		c = s - TIME_COUNT;
		if((c > 0) && (c < cb)) {
			TIME_ALARM_NEXT = s;
			TIME_ALARM_RUNCMD = TIME_ALARM[i].cmd;
			cb = c;
		}
		else {
		}
	}
}

char * alarm_cmd() {
	return TIME_ALARM_RUNCMD;
}

void alarm_resume(void * (*f)(char *)) {
	uint8_t i;
	for(i=0;i<TIME_ALARM_COUNT;++i) {
		if((TIME_COUNT<TIME_ALARM[i].sec) && (TIME_ALARM[i].sec>10)) {
			f(TIME_ALARM[i].cmd);
		}
	}
	for(i=0;i<TIME_ALARM_COUNT;++i) {
		if((TIME_COUNT>TIME_ALARM[i].sec) && (TIME_ALARM[i].sec>10)) {
			f(TIME_ALARM[i].cmd);
		}
	}
}

