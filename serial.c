/*
 * FumenBot v0.71
 * http://fumenbot.sourceforge.net/
 *
 * Andres Basile GPLv3
 * http://www.gnu.org/licenses/gpl-3.0.en.html
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "serial.h"

void serial_setup()
{ 
	UBRR0H = (BAUD_PRESCALLER>>8);
	UBRR0L = BAUD_PRESCALLER;
	UCSR0B = (1<<RXEN0) | (1<<TXEN0) | (1<<RXCIE0);
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);
	SERIAL_INDEX = 0;
	SERIAL_READY = 0;
}

ISR(USART_RX_vect)
{
	SERIAL_CHAR = UDR0;
	if(SERIAL_CHAR == '\r' || SERIAL_CHAR == '\n') {
		SERIAL_READY = 1;
	}
	else {
		SERIAL_BUFFER[SERIAL_INDEX] = SERIAL_CHAR;
		SERIAL_INDEX++;
	}
	UDR0 = SERIAL_CHAR;
}

char serial_get(void)
{
	while(!(UCSR0A & (1<<RXC0)));
	return UDR0;
}
 
void serial_put(char c)
{
	while(!(UCSR0A & (1<<UDRE0)));
	UDR0 = c;
}
 
void serial_puts(char* c)
{
	while(*c != 0x00) {
		serial_put(*c);
		c++;
	}
}

void serial_flush()
{
	SERIAL_INDEX = 0;
	SERIAL_READY = 0;
	uint8_t i;
	for(i=0; i<SERIAL_BUFFER_SIZE; i++) {
		SERIAL_BUFFER[i] = '\0';
	}
}

