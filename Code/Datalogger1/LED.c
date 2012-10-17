#include "LED.h"
#include <util/delay.h>
#include <avr/io.h>

//Blink the LED
//NOTE:  total delay ~= milliseconds * number
//			if <number> is odd, then the LED ends up toggled
//			if <milliseconds> is 0 then this just toggles the LED, no matter what number is
void BlinkLED(unsigned long milliseconds,int number){
	//Set up LED output
	LEDDDR |= _BV(LEDPIN);
	int i;
	if(!milliseconds){
		LEDPORT ^= _BV(LEDPIN);
	}else{
		for(i=0;i<number;i++){
			LEDPORT ^= _BV(LEDPIN);
			_delay_ms(milliseconds);
		}
	}
}