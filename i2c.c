/*
 * FumenBot v0.71
 * http://fumenbot.sourceforge.net/
 *
 * Andres Basile GPLv3
 * http://www.gnu.org/licenses/gpl-3.0.en.html
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/twi.h>
#include "i2c.h"
// #include "serial.h"

volatile uint8_t I2C_BUFFER[18] = "          ";
volatile uint8_t I2C_INDEX;
volatile uint8_t I2C_READY = 0;
static uint8_t I2C_ADDRESS;
static uint8_t I2C_ADDRESS_READ;
static uint8_t I2C_LENGHT;
static uint8_t I2C_RW = 0;
static char I2C_LCD[18] = "               ";
void * (*I2C_HANDLER)();

void i2c_setup() {
	TWSR = 0;
	TWBR = (F_CPU / 100000UL - 16) / 2;
	TWCR = (1 << TWEN)  | (1 << TWIE);
}

void i2c_u2cc(uint8_t * u, char * c) {
	*(c) = (char)(*(u) / 10) + '0';
	*(c + 1) = (char)(*(u) % 10) + '0';
	*(c + 2) = '\0';
}

void i2c_b2c(uint8_t * b, char * c) {
	uint8_t u = (*(b) - 6 * (*(b) >> 4));
	i2c_u2cc(&u, c);
}

void i2c_cc2u(char * c, uint8_t * u) {
	*(u) = ((c[0] - '0') * 10) + (c[1] - '0');
}

void i2c_cc2b(char * c, uint8_t * b) {
	uint8_t u;
	i2c_cc2u(c, &u);
	*(b) = u + 6 * (u / 10);
}

void i2c_start() {
	TWCR = (1<<TWINT) | (1<<TWEN) | (1 << TWIE)| (1<<TWSTA);
}

void i2c_start_mode(uint8_t u) {
	I2C_INDEX = 0;
	TWDR = I2C_ADDRESS | u;
	TWCR = (1<<TWINT) | (1<<TWEN) | (1 << TWIE);
}

void i2c_stop() {
	TWCR = (1<<TWINT) | (1<<TWEN)| (1 << TWIE) | (1<<TWSTO);
	I2C_READY = 1;
}

void i2c_set_ack() {
	TWCR = (1<<TWINT) | (1<<TWEN) | (1 << TWIE) | (1<<TWEA);
}

void i2c_set_nack() {
	TWCR = (1<<TWINT) | (1<<TWEN) | (1 << TWIE);
}

void i2c_put_nack(uint8_t * u) {
	TWDR = *(u);
	TWCR = (1<<TWINT) | (1<<TWEN) | (1 << TWIE);
}

void i2c_put_ack(uint8_t * u) {
	TWDR = *(u);
	TWCR = (1<<TWINT) | (1<<TWEN) | (1 << TWIE) | (1<<TWEA);
}

void i2c_get_ack(uint8_t * u) {
	*(u) = TWDR;
	TWCR = (1<<TWINT) | (1<<TWEN)| (1 << TWIE) | (1<<TWEA) ;
}

void i2c_get_nack(uint8_t * u) {
	*(u) = TWDR;
	TWCR = (1<<TWINT) | (1<<TWEN) | (1 << TWIE);
}


void * ds1307_print() {
	*(&I2C_LCD[0]) = (char)'D';
	i2c_b2c((uint8_t *)&I2C_BUFFER[5], &I2C_LCD[1]);
	i2c_b2c((uint8_t *)&I2C_BUFFER[4], &I2C_LCD[3]);
	i2c_b2c((uint8_t *)&I2C_BUFFER[2], &I2C_LCD[5]);
	i2c_b2c((uint8_t *)&I2C_BUFFER[1], &I2C_LCD[7]);
	i2c_b2c((uint8_t *)&I2C_BUFFER[6], &I2C_LCD[9]);
	i2c_b2c((uint8_t *)&I2C_BUFFER[0], &I2C_LCD[11]);
	i2c_b2c((uint8_t *)&I2C_BUFFER[3], &I2C_LCD[13]);
	return &I2C_LCD[0];
}

void ds1307_setc(char * c) {
	i2c_cc2b((c + 10), (uint8_t *)&I2C_BUFFER[0]); // s
	i2c_cc2b((c + 6), (uint8_t *)&I2C_BUFFER[1]); // m
	i2c_cc2b((c + 4), (uint8_t *)&I2C_BUFFER[2]); // h
	i2c_cc2b((c + 12), (uint8_t *)&I2C_BUFFER[3]); // DoW
	i2c_cc2b((c + 2), (uint8_t *)&I2C_BUFFER[4]); // Day
	i2c_cc2b(c, (uint8_t *)&I2C_BUFFER[5]); // Month
	i2c_cc2b((c + 8), (uint8_t *)&I2C_BUFFER[6]); // Year
	I2C_ADDRESS = (0x68<<1);
	I2C_ADDRESS_READ = 0x00;
	I2C_LENGHT = 7;
	I2C_RW = TW_WRITE;
	I2C_HANDLER = &ds1307_print;
	i2c_start();
}

void * ds1307_read() {
	I2C_ADDRESS = (0x68<<1);
	I2C_ADDRESS_READ = 0x00;
	I2C_LENGHT = 7;
	I2C_RW = TW_READ;
	I2C_HANDLER = &ds1307_print;
	i2c_start();
	return 0;
}

void ds1307_read_init() {
	I2C_ADDRESS = (0x68<<1);
	I2C_ADDRESS_READ = 0x00;
	I2C_LENGHT = 0;
	I2C_RW = TW_WRITE;
	I2C_HANDLER = &ds1307_read;
	i2c_start();
}

ISR(TWI_vect)	// Interrupt service routine
{
	if((TW_STATUS == TW_START) | (TW_STATUS == TW_REP_START)) {
//		serial_puts("TW_START | TW_REP_START");
		i2c_start_mode(I2C_RW);
	}
	else if(TW_STATUS == TW_MT_SLA_ACK) {
//		serial_puts("TW_MT_SLA_ACK");
		i2c_put_ack((uint8_t *)&I2C_ADDRESS_READ);
	}
	else if(TW_STATUS == TW_MT_SLA_NACK) {
//		serial_puts("TW_MT_SLA_NACK");
		i2c_stop();
	}
	else if(TW_STATUS == TW_MT_DATA_ACK) {
//		serial_puts("TW_MT_DATA_ACK");
		if(I2C_INDEX < I2C_LENGHT) {
			i2c_put_ack((uint8_t *)&I2C_BUFFER[I2C_INDEX]);
			++I2C_INDEX;
		}
		else {
			i2c_stop();
		}
	}
	else if(TW_STATUS == TW_MT_DATA_NACK) {
//		serial_puts("TW_MT_DATA_NACK");
		i2c_stop();
	}
	else if(TW_STATUS == TW_MT_ARB_LOST) {
//		serial_puts("TW_MT_ARB_LOST");
		i2c_stop();
	}
	else if(TW_STATUS == TW_MR_ARB_LOST) {
//		serial_puts("TW_MR_ARB_LOST");
		i2c_stop();
	}
	else if(TW_STATUS == TW_MR_SLA_ACK) {
//		serial_puts("TW_MR_SLA_ACK");
		i2c_set_ack();
	}
	else if(TW_STATUS == TW_MR_SLA_NACK) {
//		serial_puts("TW_MR_SLA_NACK");
		i2c_stop();
	}
	else if(TW_STATUS == TW_MR_DATA_ACK) {
//		serial_puts("TW_MR_DATA_ACK");
		if(I2C_INDEX < I2C_LENGHT) {
			i2c_get_ack((uint8_t *)&I2C_BUFFER[I2C_INDEX]);
			++I2C_INDEX;
		}
		else {
			i2c_get_nack((uint8_t *)&I2C_BUFFER[I2C_INDEX]);
		}
	}
	else if(TW_STATUS == TW_MR_DATA_NACK) {
//		serial_puts("TW_MR_DATA_NACK");
		i2c_stop();
		I2C_READY = 1;
	}

	else if(TW_STATUS == TW_NO_INFO) {
//		serial_puts("TW_NO_INFO");
	}
	else if(TW_STATUS == TW_BUS_ERROR) {
//		serial_puts("TW_BUS_ERROR");
		i2c_stop();
	}
	else {
//		serial_puts("ELSE");
		i2c_stop();
	}
//	serial_puts("\n\r");
}
