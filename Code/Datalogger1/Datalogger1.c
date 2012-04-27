/*
  SD card datalogger
 
  HEAVILY MODIFIED BY GROUP 10
  WARNING: analogRead and string functions and classes DO NOT WORK.
 	
 The circuit:
 * analog sensors on analog ins 0, 1, and 2
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11q
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 10
 
 created  24 Nov 2010
 updated 2 Dec 2010
 by Tom Igoe
 
 	 
 */
 
 
//Uncomment this to enable serial output
#define SERIALOUT 1
//Uncomment this to enable the SD Card
//#define SDENABLE 1
 
//My includes
#ifdef SDENABLE
	#include <SD.h>
#endif

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

//This is a workaround for printf's inability to print floating point numbers
long int temp, temph, templ;
#define ftoi(floating_point_number) temp = floating_point_number*100; temph = temp/100; templ = temp%100;

void BlinkLED(unsigned long milliseconds,int number);


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


#ifdef SDENABLE
File dataFile;								//This is the file on the SD card.
void SD_setup();
void SizeSafetyCheck(File fileToCheck);

void SD_setup(){
  #ifdef SERIALOUT
    sprint("Initializing SD card...\n\r");
  #endif
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  
  // see if the card is present and can be initialized:
  if (!SD.begin(CHIPSELECT)) {
    #ifdef SERIALOUT
      sprint("Card failed, or not present\n\r");
    #endif
	while(1){
		BlinkLED(50,100);
	}
  }else{
	  #ifdef SERIALOUT
		sprint("card initialized.\n\r");
	  #endif
  }
}


//If the File is near/over 3GB stop, close the file and blink the LED quickly
void SizeSafetyCheck(File fileToCheck){
  if(fileToCheck.size() > 24000000000.0){
	fileToCheck.close();
    while(1){
		BlinkLED(50,10);
    }
  }
}
#endif

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

  //Set up the SD card
	#ifdef SDENABLE
	pinMode(10, OUTPUT);        	//Needed for SD Library
	SD_setup();
	#endif
  
  //Set upt he ADC
  ADC_setup();
  
  BlinkLED(1000,1);
  #ifdef SERIALOUT
	sprint("Initalization Completed\n\r");
  #endif
}

void loop()
{
	#ifdef SDENABLE
  dataFile = SD.open("datalog.txt", FILE_WRITE);
  
  //File Size check, sanity keeper
  //SizeSafetyCheck(dataFile);
  

  // if the file is available, write to it:
  if (dataFile) {

	#endif
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
		
#ifdef SDENABLE
      dataFile.flush();
      dataFile.close();
  }  
  // if the file isn't open, pop up an error:
  else {
    #ifdef SERIALOUT
      sprint("error opening datalog.txt\n\r");
    #endif
    while(1){
      BlinkLED(50,100);
    }
  }
  #endif
}

int main(){
	setup();
	for(;;){
		loop();
	}
	return 0;
}
