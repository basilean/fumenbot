/*
 * FumenBot v0.71
 * http://fumenbot.sourceforge.net/
 *
 * Andres Basile GPLv3
 * http://www.gnu.org/licenses/gpl-3.0.en.html
 */

#ifndef DIO_H
#define DIO_H
	void dio_set_in(uint8_t *, uint8_t);
	void dio_set_out(uint8_t *, uint8_t);
	void dio_set_hi(uint8_t *, uint8_t);
	void dio_set_low(uint8_t *, uint8_t);
	void dio_set_pwm(uint8_t *, uint8_t, uint8_t *, uint8_t);
	void dio_set_pwm_hi(uint8_t *, uint8_t *, uint8_t, uint8_t);
	void dio_set_pwm_low(uint8_t *, uint8_t);
#endif
