# Load configuration for the part, library options, etc.

include config.mk

# Fuse bits

HFUSE=0xDA
LFUSE=0xFF

# Build tools (defaults assume crosspack, winavr, etc. is in your PATH)

CC = avr-gcc
OBJCOPY = avr-objcopy
AR = avr-ar
AVR_SIZE = avr-size
AVRDUDE = avrdude

# Application configuration

APP = Datalogger1
APP_OBJECTS = Datalogger1.o adc.o measurement.o uart.o radio/radio-uart.o radio/hal_avr.o radio/tat.o radio/errcode.o

ARDUINO_LIBRARY_PATH = C:\Users\Arthur\Documents\arduino-1.0\hardware\arduino\cores\arduino
ARDUINO_PIN_PATH = C:\Users\Arthur\Documents\arduino-1.0\hardware\arduino\variants\standard
ARDUINO_SD_PATH = C:\Users\Arthur\Documents\arduino-1.0\libraries\SD

# Build tool configuration

CFLAGS=-mmcu=$(MCU) -Wall -DF_CPU=$(F_CPU) -I. -funsigned-char -funsigned-bitfields \
	-fpack-struct -fshort-enums -fno-strict-aliasing -Os \
	-DEEPROM_CONFIG_OFFSET=$(EEPROM_CONFIG_OFFSET)
#    -I$(ARDUINO_LIBRARY_PATH) -I$(ARDUINO_PIN_PATH) -I$(ARDUINO_SD_PATH)
LDFLAGS=-mmcu=$(MCU) 

.PHONY: clean app all fuses

all: app

# Rules for building the application

app: $(APP).hex

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

$(APP).elf: $(APP_OBJECTS) 
	$(CC) $(LDFLAGS) -o $@ $^
	$(AVR_SIZE) -C --mcu=$(MCU) $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

install: $(APP).hex
	$(AVRDUDE) -p $(AVRDUDE_PART) -c $(PROGRAMMER) -P $(COMPORT) -b $(ISPSPEED) -e -U flash:w:$<

fuses: 
	$(AVRDUDE) -p $(AVRDUDE_PART) -c $(PROGRAMMER) -P $(COMPORT) -b $(ISPSPEED) -e -U lfuse:w:$(LFUSE):m \
		-U hfuse:w:$(HFUSE):m 

clean:
	rm -rf radio/*.o
	rm -f radio/*.a
	rm -f *.o
	rm -f $(APP).{hex,elf}
