#include "communication.h"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "uart.h"
#include "radio/radio-uart.h"


//This converts all spaces in a string to zeros
void SpaceToZero(char* str,int length){
	int i;
	for(i=0;i<length;i++){
		if (str[i]==' '){
			str[i]='0';
		}
	}
}

//This prints out a c string
void cprint(char * str){
	int i;
	for(i=0;str[i] != '\0';i++){
		uart_putchar(str[i]);
		radio_putchar(str[i]);
	}
}

//This lets me store pure strings in flash instead of data.
void nprintf (PGM_P s) {
        char c;
        while ((c = pgm_read_byte(s++)) != 0){
		uart_putchar(c);
		radio_putchar(c);
	}
}

#ifdef __cplusplus
extern "C"{
	FILE * uart_stream;
}
#else
static FILE uart_stream = FDEV_SETUP_STREAM(
	#ifdef RADIOOUT
	radio_putchar_f,
	#else
	uart_putchar_f,
	#endif
	uart_getchar_f,
	_FDEV_SETUP_RW
);
#endif



//Set up both the UART and the Radio
int communication_setup(){
	// UART
	DDRD |= (1<<PD1);
	DDRD |= (1<<PD2);
	uart_setup();
	
	#ifdef __cplusplus
	uart_stream = fdevopen(
		#ifdef RADIOOUT
		radio_putchar_f,
		#else
		uart_putchar_f,
		#endif
		uart_getchar_f);
	stdout = stdin = uart_stream;
	#else
	stdout = &uart_stream;
	#endif
	int radio_failed = radio_setup();
	if(radio_failed){
		sprint("ERROR:  Radio initalization FAILED!!!!\n\r");
        for(;;);
	}
	return 0;
}