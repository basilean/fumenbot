/*
 * FumenBot v0.71
 * http://fumenbot.sourceforge.net/
 *
 * Andres Basile GPLv3
 * http://www.gnu.org/licenses/gpl-3.0.en.html
 */

#ifndef I2C_H
#define I2C_H
	volatile uint8_t I2C_READY;
	void * (*I2C_HANDLER)();
	void i2c_setup();
	void i2c_start();
	void i2c_stop();
	void i2c_put();
	void * ds1307_read();
	void ds1307_read_init();
	void i2c_b2c(uint8_t *, char *);
	void ds1307_setc(char * c);
#endif
