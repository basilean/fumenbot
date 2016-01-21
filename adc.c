/*
 * FumenBot v0.71
 * http://fumenbot.sourceforge.net/
 *
 * Andres Basile GPLv3
 * http://www.gnu.org/licenses/gpl-3.0.en.html
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "adc.h"
#include "serial.h"

static char ADC_LCD[16];

char * adc_uu2c(uint16_t * u, char * c) {
	uint16_t i;
	uint16_t t = *(u);
	uint8_t ii = 0;
	for(i=10000;i>0;i/=10) {
		*(c + ii) = (char)(t / i) + '0';
		t %= i;
		++ii;
	}
	*(c + ii) = '\0';
	return c;
}

void adc_setup() {
	ADMUX = (1<<REFS0);
	ADMUX &= ~(1<<ADLAR);
	ADCSRA = (1 << ADEN) | (1<<ADIE) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
	ADC_READY = 0;
}

void adc_read(uint8_t n) {
	n &= 0b00000111;
	ADMUX = (ADMUX & 0xF8) | n;
	ADCSRA |= (1 << ADSC);
}

ISR(ADC_vect) {
	ADC_BUFFER = ADC;
	ADC_READY = 1;
}

char * adc_u2cc(uint8_t * u, char * c) {
	*(c) = (char)(*(u) / 10) + '0';
	*(c + 1) = (char)(*(u) % 10) + '0';
	*(c + 2) = '\0';
	return c;
}

char * adc_lm35(uint16_t * u) {
	uint8_t i = 500 * *(u) / 1024;
	uint8_t ii = (500 * *(u) % 1024) * 10 / 1024;
	*(&ADC_LCD[0]) = 'T';
	*(&ADC_LCD[1]) = 'e';
	*(&ADC_LCD[2]) = 'm';
	*(&ADC_LCD[3]) = 'p';
	*(&ADC_LCD[4]) = ':';
	*(&ADC_LCD[5]) = ' ';
	adc_u2cc(&i, &ADC_LCD[6]);
	*(&ADC_LCD[8]) = '.';
	adc_u2cc(&ii, &ADC_LCD[9]);
	return &ADC_LCD[0];
}

