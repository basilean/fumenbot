/*
 * FumenBot v0.71
 * http://fumenbot.sourceforge.net/
 *
 * Andres Basile GPLv3
 * http://www.gnu.org/licenses/gpl-3.0.en.html
 */

#ifndef SERIAL_H
#define SERIAL_H
#define BAUDRATE 9600
#define BAUD_PRESCALLER (((F_CPU / (BAUDRATE * 16UL))) - 1)
#define SERIAL_BUFFER_SIZE 18
	void serial_setup();
	char serial_get(void);
	void serial_put(char data);
	void serial_puts(char* data);
	void serial_flush();
	volatile char SERIAL_BUFFER[SERIAL_BUFFER_SIZE];
	volatile uint8_t SERIAL_INDEX;
	volatile uint8_t SERIAL_READY;
	volatile uint8_t SERIAL_CHAR;
#endif
