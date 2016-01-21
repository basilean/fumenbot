/*
 * FumenBot v0.71
 * http://fumenbot.sourceforge.net/
 *
 * Andres Basile GPLv3
 * http://www.gnu.org/licenses/gpl-3.0.en.html
 */

#ifndef TIME_H
#define TIME_H
#define TIME_ALARM_CMD_SIZE 8
	typedef struct {
		uint32_t sec;
		uint32_t interval;
		char cmd[TIME_ALARM_CMD_SIZE];
	} alarm_t;
	volatile uint32_t TIME_COUNT;
	volatile uint8_t TIME_ALARM_READY;
	volatile uint8_t TIME_ALARM_RESUME;
	volatile uint8_t TIME_NEXT_DAY;
	void time_setup();
	void time_up_Y();
	void time_up_M();
	void time_up_D();
	void time_setc(char *);
	uint32_t * time_set_count(uint32_t *, uint8_t *, uint8_t *, uint8_t *);
	char * time_getc();
	char * time_getc_c();
	uint8_t * time_cc2u(char *, uint8_t *);
	char * time_u2cc(uint8_t *, char *);
	char * time_uuuu2c(uint32_t *, char *);
	uint8_t * time_get_h(uint32_t *, uint8_t *);
	uint8_t * time_get_m(uint32_t *, uint8_t *);
	uint8_t * time_get_s(uint32_t *, uint8_t *);
	char * time_getc_h(uint32_t *, char *);
	char * time_getc_m(uint32_t *, char *);
	char * time_getc_s(uint32_t *, char *);
	void alarm_setup();
	void alarm_next();
	char * alarm_cmd();
	void alarm_resume(void * (*)(char *));
#endif
