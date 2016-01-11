/*******

FumenBot v0.7
http://fumenbot.sourceforge.net/

Andres Basile GPLv3

*******/

// Mostly here was taken from differents examples around web.

#include <avr/io.h>
#include <avr/interrupt.h>

#include "main.h"
#include "fumenbot.h"
#include "serial.h"

void serial_setup()
{ 
	UBRR0H = (BAUD_PRESCALLER>>8);
	UBRR0L = BAUD_PRESCALLER;
	UCSR0B = (1<<RXEN0) | (1<<TXEN0) | (1<<RXCIE0);
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);
	serial_idx = 0;
	serial_go = 0;
}

ISR(USART_RX_vect)
{
	volatile uint8_t tmp = UDR0;
	if(tmp == '\r') {
		serial_go = 1;
	}
	else {
		serial_buffer[serial_idx] = tmp;
		serial_idx++;
	}
	UDR0 = tmp;
}


char serial_get(void)
{
	while(!(UCSR0A & (1<<RXC0)));
	return UDR0;
}
 
void serial_put(char data)
{
	while(!(UCSR0A & (1<<UDRE0)));
	UDR0 = data;
}
 
void serial_puts(char* data)
{
	while(*data != 0x00) {
		serial_put(*data);
		data++;
	}
}

void serial_flush()
{
	serial_idx = 0;
	serial_go = 0;
	int i;
	for(i=0; i<SERIAL_BUFFER_SIZE; i++) {
		serial_buffer[i] = '\0';
	}
}

