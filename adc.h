/*
 * FumenBot v0.71
 * http://fumenbot.sourceforge.net/
 *
 * Andres Basile GPLv3
 * http://www.gnu.org/licenses/gpl-3.0.en.html
 */

#ifndef ADC_H
#define ADC_H
	volatile uint16_t ADC_BUFFER;
	volatile uint8_t ADC_READY;
	char * (*ADC_HANDLER)(uint16_t *);
	char * adc_lm35(uint16_t *);
	void adc_setup();
	void adc_read(uint8_t);
	char * adc_uu2c(uint16_t *, char *);
#endif
