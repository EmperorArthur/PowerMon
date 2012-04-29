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

//How many measurements we want
#define MAX_MEASUREMENTS 4086
volatile uint8_t enable_measurement = 0;

//This set's my Timer0 frequency (Timer1 counts up to 65535)
//Empirical evidence has shown that with current code, the max sampling frequency is 4086 HZ
#define Sampling_Frequency 4086
#define Prescaler 1
#define Target_Timer_Count (((F_CPU / Prescaler) / Sampling_Frequency) - 1)


// The pin my status indicator LED is on
//#define LEDPIN 5
#define LEDPIN PD5
#define LEDDDR DDRD
#define LEDPORT PORTD



void BlinkLED(unsigned long milliseconds,int number);

void timer_setup(){
	OCR1A = Target_Timer_Count;	//Timer counts up to this value
	TCCR1B = _BV(CS10);			//Set our Prescaler to 1
	TCCR1B |= _BV(WGM12);		//Count up to the value in OCR1A
	TIMSK1 = _BV(OCIE1A);		//Enable timer interrupt A

}

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
	while(number != 0){
		buf[i++] = number%10;
		number = number/10;
	}
	//And then work backwards
	while(i>0){
		uart_putchar(buf[--i]+48);
	}
	
}

//This doesn't work right.  It prints both 2.60 and 2.06 as 2.6!!!!!!
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
//NOTE:  total delay ~= milliseconds * number
//			if <number> is odd, then the LED ends up toggled
//			if <milliseconds> is 0 then this just toggles the LED, no matter what number is
void BlinkLED(unsigned long milliseconds,int number){
	int i;
	if(milliseconds){
		LEDPORT ^= _BV(LEDPIN);
	}else{
		for(i=0;i<number;i++){
			LEDPORT ^= _BV(LEDPIN);
			_delay_ms(milliseconds);
		}
	}
}

struct measurements Voltage;					//This is the struct that holds our voltage measurements
struct measurements Amperage;					//This is the struct that holds our amperage measurements


void setup()
{
  //Set up out inputs and outputs
  LEDDDR |= _BV(LEDPIN);
  
	// UART
	DDRD |= (1<<PD1);
	DDRD |= (1<<PD2);
    uart_setup();
    stdout = &uart_stream;
	
	timer_setup();
  
  BlinkLED(1000,1);

  
  //Set upt he ADC
  ADC_setup();
  
  BlinkLED(1000,1);
  #ifdef SERIALOUT
	printf("%li\n\r",Target_Timer_Count);
	printf("Sampling %i Measurements at %i HZ\n\r",MAX_MEASUREMENTS,Sampling_Frequency);
	sprint("Initalization Completed\n\r");
  #endif
  //Enable interupts
  sei();
}

void loop()
{
	//Print out the Reference voltage our ADC is using
	//It should be 3.3V
	#ifdef SERIALOUT
	ftoi(Get_Vref());
	printf("Reference Voltage is:  %li.%.2li\n\r",temph,templ);
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

		//Measure(MAX_MEASUREMENTS,&Voltage);
		//Measure(MAX_MEASUREMENTS,&Amperage);
		enable_measurement = 1;
		while(enable_measurement){;}
		Calculate_Results(&Voltage);
		Calculate_Results(&Amperage);
		#ifdef SERIALOUT
		
		ftoi(Voltage.average);
		printf("%li.%.2li V Average;",temph,templ);
		ftoi(Voltage.RMS);
		printf("%li.%.2li V RMS;",temph,templ);
		printf("%i samples for V; ",Voltage.numSamples);
		printf("%li milliseconds for V; ",Voltage.time);
		printf("%li totalAverage for V; ",Voltage.totalAverage);
		printf("%li totalRMS for V; ",Voltage.totalRMS);
		sprint("\n\r");
		
		ftoi(Amperage.average);
		printf("%li.%.2li A Average;",temph,templ);
		ftoi(Amperage.RMS);
		printf("%li.%.2li A RMS;",temph,templ);
		printf("%i samples for A; ",Amperage.numSamples);
		printf("%li milliseconds for A; ",Amperage.time);
		printf("%li totalAverage for A; ",Amperage.totalAverage);
		printf("%li totalRMS for A; ",Amperage.totalRMS);
		sprint("\n\r");
		
		#endif

		//Blink the LED to let us know that we're done with a cycle
		BlinkLED(0,1);
}
//This is triggered by my timer
//takeMeasurement uses the ADC interupt so this has to be nonblocking
ISR(TIMER1_COMPA_vect,ISR_NOBLOCK){
	//If we're sampling too fast, stop and tell the user.
	//Warning:  Sometimes this still doesn't catch it, and the takeMeasurement functions themselves do and block.
	if(measurement_lock){
		TCCR1B = 0;
		for(;;){
			printf("Warning:  Sampling frequency <%i> too fast",Sampling_Frequency);
			sprint("!!!");
			BlinkLED(100,20);
			_delay_ms(1000);
		}
	}
	if(enable_measurement){
		if(Voltage.numSamples < MAX_MEASUREMENTS){
			takeMeasurement(&Voltage);
			takeMeasurement(&Amperage);
		} else {
			enable_measurement = 0;
		}
	}
}

//This is in case something went wrong
ISR(BADISR_vect){
	cli();
	for(;;){
		sprint("Warning:  Uncaught Interupt Detected!!!");
		BlinkLED(100,20);
		_delay_ms(1000);
	}
}

int main(){
	setup();
	for(;;){
		loop();
	}
	return 0;
}
