# Configuration options for the radio library

MCU = atmega328
F_CPU = 16000000UL
AVRDUDE_PART = m328p
PROGRAMMER = arduino
EEPROM_CONFIG_OFFSET = 0x00
COMPORT = com9
ISPSPEED = 57600

# Fuse bits

HFUSE=0xDA
LFUSE=0xFF

# Location of the radio library source tree

RADIO_LIBRARY_SRC = ../AVR-Lib-AT86RF230/avrradio

# Default radio options (if 'make setaddr' is invoked with no options)

CHANNEL ?= 21
PAN ?= 0x8842
ADDR ?= 0x0001


# LED configuration

RADIO_LED_ACTIVE_LOW = 0
RADIO_LED_HAVE_DUAL = 0

RADIO_LED1 = PD5
RADIO_LED1_PORT = PORTD
RADIO_LED1_DDR = DDRD

# Pin configurations for radio interface

SLP_TR = PB0
DDR_SLP_TR = DDRB
PORT_SLP_TR = PORTB
PIN_SLP_TR = PINB

RST = PD6
DDR_RST = DDRD
PORT_RST = PORTD
PIN_RST = PIND

HAL_PORT_SPI = PORTB
HAL_DDR_SPI = DDRB

HAL_SS_PIN = PB2

HAL_DD_SS = PB2
HAL_DD_SCK = PB5
HAL_DD_MOSI = PB3
HAL_DD_MISO = PB4