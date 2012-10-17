//LED Library
//Coypright Arthur Moore 2012
//GPL V3

//Need:
// The pin my status indicator LED is on
#define LEDPIN PD5
#define LEDDDR DDRD
#define LEDPORT PORTD

#ifndef LED_H
#define LED_H
//Blink the LED
//NOTE:  total delay ~= milliseconds * number
//			if <number> is odd, then the LED ends up toggled
//			if <milliseconds> is 0 then this just toggles the LED, no matter what number is
void BlinkLED(unsigned long milliseconds,int number);
#endif