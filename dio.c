/*
 * FumenBot v0.71
 * http://fumenbot.sourceforge.net/
 *
 * Andres Basile GPLv3
 * http://www.gnu.org/licenses/gpl-3.0.en.html
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "dio.h"

void dio_set_pwm(uint8_t * ta, uint8_t tas, uint8_t * tb, uint8_t tbs) {
	*(ta) |= tas;
	*(tb) |= tbs;
}

void dio_set_pwm_hi(uint8_t * t, uint8_t * d, uint8_t c, uint8_t v) {
	*(t) |= (1<<c);
	*(d) |= v;
}

void dio_set_pwm_low(uint8_t * t, uint8_t c) {
	*(t) &= ~(1<<c);
}

void dio_set_out(uint8_t * r, uint8_t n) {
	*(r) |= (1<<n);
}

void dio_set_in(uint8_t * r, uint8_t n) {
	*(r) &= ~(1<<n);
}

void dio_set_hi(uint8_t * p, uint8_t n) {
	*(p) |= (1<<n);
}

void dio_set_low(uint8_t * p, uint8_t n){
	*(p) &= ~(1<<n);
}
