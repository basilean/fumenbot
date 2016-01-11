/*******

FumenBot v0.7
http://fumenbot.sourceforge.net/

Andres Basile GPLv3

*******/

#ifndef SERIAL_H
#define SERIAL_H
#define BAUDRATE 9600
#define BAUD_PRESCALLER (((F_CPU / (BAUDRATE * 16UL))) - 1)
#define SERIAL_BUFFER_SIZE 16
	void serial_setup();
	char serial_get(void);
	void serial_put(char data);
	void serial_puts(char* data);
	void serial_flush();
	volatile char serial_buffer[SERIAL_BUFFER_SIZE];
	volatile uint8_t serial_idx;
	volatile uint8_t serial_go;
#endif
