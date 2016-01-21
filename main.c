/*
 * FumenBot v0.71
 * http://fumenbot.sourceforge.net/
 *
 * Andres Basile GPLv3
 * http://www.gnu.org/licenses/gpl-3.0.en.html
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "main.h"
#include "fumenbot.h"

int main()
{
	fumenbot_setup();
	fumenbot_loop();
	return 0;
}

