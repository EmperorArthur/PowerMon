//Wireless In home Power mOnitoring Datalogger
//Copyright Arthur Moore 2012
 
 
//Uncomment this to enable serial output
#define SERIALOUT 1

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include "adc.h"
#include "measurement.h"
#include "uart.h"


//This si the expected voltage my chip is operating at in Volts (it may be different than Vref)
#define VCC 3.3


// The pin my status indicator LED is on
//#define LEDPIN 5
#define LEDPIN PD5
#define LEDDDR DDRD
#define LEDPORT PORTD

//The chip select pin for the SD card
//#define CHIPSELECT 1  	//Use this one for the prototype board
#define CHIPSELECT 3	//Use this one for the Rev A Boards
//#define CHIPSELECT 10  //Use this one for the arduino
#define SD_CS_PIN PD3
#define SD_CS_DDR DDRD
#define SD_CS_PORT PORTD


void BlinkLED(unsigned long milliseconds,int number);

//This is a workaround for printf's inability to print floating point numbers
long int temp, temph, templ;
void ftoi(float floating_point_number) {temp = floating_point_number*100; temph = temp/100; templ = temp%100;}

void printU32(uint32_t number){
	//Max theoretical value of a uint32_t is 4294967295
	//That's 10 digits, plus a terminator
	char buf[11];
	int i = 0;
	//Put the smallest digit into the buffer and then remove it.
	while(number > 0){
		buf[i++] = number%10;
		number = number/10;
	}
	//And then work backwards
	while(i>0){
		uart_putchar(buf[--i]+48);
	}
	
}

void printLongInt(long int number){
	//Max theoretical value of a uint32_t is 4294967295
	//That's 10 digits, plus a terminator
	char buf[11];
	int i = 0;
	//Put the smallest digit into the buffer and then remove it.
	while(number > 0){
		buf[i++] = number%10;
		number = number/10;
	}
	//And then work backwards
	while(i>0){
		uart_putchar(buf[--i]+48);
	}
	
}

void printFloat(float number){
	ftoi(number);
	printLongInt(temph);
	uart_putchar('.');
	printLongInt(templ);
}

//This lets me store pure strings in flash instead of data.
#define sprint(string) nprintf(PSTR(string))
void nprintf (PGM_P s) {
        char c;
        while ((c = pgm_read_byte(s++)) != 0)
			uart_putchar(c);
}

static FILE uart_stream = FDEV_SETUP_STREAM(
    uart_putchar_f,
    uart_getchar_f,
    _FDEV_SETUP_RW
);

//Blink the LED
//NOTE:  total delay ~= 2 * milliseconds * number
void BlinkLED(unsigned long milliseconds,int number){
	int i;
	for(i=0;i<number;i++){
		LEDPORT ^= _BV(LEDPIN);
		_delay_ms(milliseconds);
	}
}

struct measurements Voltage;					//This is the struct that holds our voltage measurements
struct measurements Amperage;					//This is the struct that holds our amperage measurements


void setup()
{
  //Set up out inputs and outputs
  SD_CS_DDR |= _BV(SD_CS_PIN);
  LEDDDR |= _BV(LEDPIN);
  
	// UART
	DDRD |= (1<<PD1);
	DDRD |= (1<<PD2);
    uart_setup();
    stdout = &uart_stream;
	
  //Enable interupts
  sei();
  
  BlinkLED(1000,1);

  
  //Set upt he ADC
  ADC_setup();
  
  BlinkLED(1000,1);
  #ifdef SERIALOUT
	sprint("Initalization Completed\n\r");
  #endif
}

void loop()
{
	//Print out the Reference voltage our ADC is using
	//It should be 3.3V
	#ifdef SERIALOUT
	ftoi(Get_Vref());
	printf("Reference Voltage is:  %li.%li\n\r",temph,templ);
	#endif
	/*
	//read from the temperature sensor (This is a good way to check if our ADC is working)
	ADMUX  = _BV(REFS0) | _BV(REFS1);		//Set our reference Voltage to internal 1.1V for the temperature sensor to work right
	//int temperature = ADC_read(8);
	//This is the version using interupts
	ADC_start(8);
	ADC_wait_done();
	int temperature = ADCResult;

	dataFile.print("Temperature is:  ");
	dataFile.print(temperature);
	*/

		NewMeasurement(0,&Voltage);
		NewMeasurement(1,&Amperage);

		Measure(&Voltage);
		Measure(&Amperage);
		
		#ifdef SERIALOUT
		
		ftoi(Voltage.average);
		printf("%li.%li V Average;",temph,templ);
		ftoi(Voltage.RMS);
		printf("%li.%li V RMS;",temph,templ);
		printf("%i samples for V; ",Voltage.numSamples);
		printf("%li milliseconds for V; ",Voltage.time);
		printf("%li totalAverage for V; ",Voltage.totalAverage);
		printf("%li totalRMS for V; ",Voltage.totalRMS);
		sprint("\n\r");
		
		ftoi(Amperage.average);
		printf("%li.%li A Average;",temph,templ);
		ftoi(Amperage.RMS);
		printf("%li.%li A RMS;",temph,templ);
		printf("%i samples for A; ",Amperage.numSamples);
		printf("%li milliseconds for A; ",Amperage.time);
		printf("%li totalAverage for A; ",Amperage.totalAverage);
		printf("%li totalRMS for A; ",Amperage.totalRMS);
		sprint("\n\r");
		
		#endif

		//Blink the LED to let us know that we're done with a cycle
		BlinkLED(100,1);
}

int main(){
	setup();
	for(;;){
		loop();
	}
	return 0;
}
