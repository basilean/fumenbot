/*******

FumenBot v0.7
http://fumenbot.sourceforge.net/

Andres Basile GPLv3

*******/

#ifndef FUMENBOT_H
#define FUMENBOT_H

typedef struct {
	uint8_t bit;
	volatile uint8_t *port;
	volatile uint8_t *reg;
} dio_t;

typedef struct {
	uint8_t bit;
} aio_t;

typedef struct {
	uint8_t bit;
	volatile uint8_t *port;
	volatile uint8_t *reg;
	uint8_t com;
	volatile uint8_t *timer;
	volatile uint8_t *duty;
	uint8_t vol;
	uint8_t on;
	uint8_t off;
} light_t;

typedef struct {
	aio_t temp[1];
	aio_t lrd[2];
	aio_t water[1];
	dio_t led[1];
	dio_t valve[4];
	light_t light[4];
	uint8_t rtc;
	uint8_t is_default;
	uint8_t i2c[3];
} fumenbot_t;

char * fumenbot_handler(char*);
void fumenbot_setup(uint8_t);
void fumenbot_resume();

void time_alarm_check();
#define TIME_ALARM_SIZE 18
#define TIME_ALARM_CMD_SIZE 8

typedef struct {
	uint8_t enabled;
	uint8_t h;
	uint8_t m;
	uint8_t s;
	char cmd[TIME_ALARM_CMD_SIZE];
} time_alarm_t;

typedef struct {
	uint8_t s;
	uint8_t m;
	uint8_t h;
	uint8_t D;
	uint8_t M;
	uint8_t Y;
} time_t;

#endif
