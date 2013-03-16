This is the code and Eagle files for a wireless power monitoring system.
All code and schematics are Copyright Arthur Moore 2013 GPLV3
	If this license is too restrictive for your use, feel free to contact me.

The wireless power monitor works using two boards with IEE 802.15.4 Radios.
One board monitors and transmits power usage data, while the other recieves the data, and sends it on to a computer.

Currently there is one board that does everything.  It uses an ATMega328 microcontroller with an AT86RF231 radio module.

This repository was taken from an SVN snapshot, then sanitized for public release.


The code is in ./Code/Datalogger1/

Important files are:
ADC.h/c:				These are the Analog to Digital Converter functions.
LED.h/c:				Simple functions to controll a LED
Communication.h/c:		Bridge functions, allowing cout to work with both serial and the radio.
UART.h/c:				Serial I/O for cout and cin
Measurements.h/c:		Where the actual measurements are done and calculated.
Datalogger1.c:			The main file.  This is where the important stuff takes place.

Contrary to any common sense, the main board is in ./Eagle Files/Main Board with Headers/

The only header on the board is a SPI programming header.
I normally use an arduino bootloader with the FTDI chip, so it's mostly unused.
There is one wire missing for this configuration.  A wire from the FTDI chip to the reset pin with a small capacitor in series.

I was orriginally using an older version of the radio library provided here:  http://simonetti.media.mit.edu/dev/projects/avrradio
Given my issues with the library documentation and microcontroller space issues, I had to hack the provided library to pieces.
I eventually ended up creating my own library from scratch for this thing.  See my github page for it.

To Do:
	Replace hacked together radio library with the one I created.
	Re-organize the repository so It's easy to understand, and doesn't have as many misc files.
	Completely redesign board for smaller footprint, less component count, and higher acuracy.
	Potentially seperate board into two seperate boards.

