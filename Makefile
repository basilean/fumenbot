PROGRAM = fumenbot
MCU = atmega328p
F_CPU = 16000000UL
CC = avr-gcc
OBJCOPY = avr-objcopy
CFLAGS += -Wall -g -Os -mmcu=$(MCU) -DF_CPU=$(F_CPU)
LDFLAGS += 
OBJS = main.o $(PROGRAM).o serial.o time.o i2c.o adc.o dio.o

ifneq ($(V),1)
Q := @
endif

all: $(PROGRAM).hex

$(PROGRAM).elf: $(OBJS)
	@printf " LD $(subst $(shell pwd)/,,$(@))\n"
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(PROGRAM).hex: $(PROGRAM).elf
	@printf " OBJCOPY $(subst $(shell pwd)/,,$(@))\n"
	$(Q)$(OBJCOPY) -O ihex $< $@

%.o: %.c
	@printf " CC $(subst $(shell pwd)/,,$(@))\n"
	$(Q)$(CC) $(CFLAGS) -o $@ -c $<

flash: $(PROGRAM).hex
	@printf " FLASH $(PROGRAM).hex\n"
	$(Q)avrdude -v -p atmega328p -c arduino -P /dev/ttyUSB0 -b 57600 -D -U flash:w:$(PROGRAM).hex:i

clean:
	@printf " CLEAN $(subst $(shell pwd)/,,$(OBJS))\n"
	$(Q)rm -f $(OBJS)
	@printf " CLEAN $(PROGRAM).elf\n"
	$(Q)rm -f *.elf
	@printf " CLEAN $(PROGRAM).hex\n"
	$(Q)rm -f *.hex

