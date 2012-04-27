# Load configuration for the part, library options, etc.

MCU = atmega328
F_CPU = 16000000UL
AVRDUDE_PART = m328p
PROGRAMMER = arduino
EEPROM_CONFIG_OFFSET = 0x00
COMPORT = com8
ISPSPEED = 57600

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
APP_OBJECTS = Datalogger1.o adc.o measurement.o uart.o

ARDUINO_LIBRARY_PATH = C:\Users\Arthur\Documents\arduino-1.0\hardware\arduino\cores\arduino
ARDUINO_PIN_PATH = C:\Users\Arthur\Documents\arduino-1.0\hardware\arduino\variants\standard
ARDUINO_SD_PATH = C:\Users\Arthur\Documents\arduino-1.0\libraries\SD

# Build tool configuration

CFLAGS=-mmcu=$(MCU) -Wall -DF_CPU=$(F_CPU) -I. -funsigned-char -funsigned-bitfields \
	-fpack-struct -fshort-enums -fno-strict-aliasing -Os
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

%.o: %.c $(RADIO_LIBRARY)
	$(CC) $(CFLAGS) -c $< -o $@

install: $(APP).hex
	$(AVRDUDE) -p $(AVRDUDE_PART) -c $(PROGRAMMER) -P $(COMPORT) -b $(ISPSPEED) -e -U flash:w:$<

fuses: 
	$(AVRDUDE) -p $(AVRDUDE_PART) -c $(PROGRAMMER) -P $(COMPORT) -b $(ISPSPEED) -e -U lfuse:w:$(LFUSE):m \
		-U hfuse:w:$(HFUSE):m 

clean:
	rm -rf include/radio
	rm -f lib/*.a
	rm -f *.o
	rm -f $(APP).{hex,elf}
