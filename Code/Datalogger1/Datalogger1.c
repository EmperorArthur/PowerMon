//Wireless In home Power mOnitoring Datalogger
//Copyright Arthur Moore 2012
 

//Uncomment this to enable aditional debugging output
//#define DEBUGOUT 1

// The pin my status indicator LED is on
#define LEDPIN PD5
#define LEDDDR DDRD
#define LEDPORT PORTD

//How many measurements we want
#define MAX_MEASUREMENTS 1000


//This set's my Timer0 frequency (Timer1 counts up to 65535)
//Empirical evidence has shown that with current code, the max sampling frequency is 4086 HZ
#define SAMPLING_FREQUENCY 3000
#define PRESCALER 1
#define TARGET_TIMER_COUNT (((F_CPU / PRESCALER) / SAMPLING_FREQUENCY) - 1)

//macros to turn these #defines into strings when needed (the precompiler is weird)
#define STRINGIFY(str) TOSTR(str)
#define TOSTR(str) #str

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <stdlib.h>
#include "adc.h"
#include "measurement.h"
#include "communication.h"

volatile uint16_t cyclesCompleted;

void timer_setup(){
	#if TARGET_TIMER_COUNT > 65535
		#error TARGET_TIMER_COUNT is too large
	#endif
	#if PRESCALER == 1
		TCCR1B = _BV(CS10);						//Set our Prescaler to 1
	#elif PRESCALER == 8
		TCCR1B = _BV(CS11);						//Set our Prescaler to 8
	#elif PRESCALER == 64
		TCCR1B = _BV(CS10) | _BV(CS11);			//Set our Prescaler to 64
	#elif PRESCALER == 256
		TCCR1B = _BV(CS12)						//Set our Prescaler to 256
	#elif PRESCALER == 1024
		TCCR1B = _BV(CS10) | _BV(CS12);			//Set our Prescaler to 1024
	#else
		#error Prescaler value is incorrect
	#endif
	
		
	OCR1A = TARGET_TIMER_COUNT;	//Timer counts up to this value
	TCCR1B |= _BV(WGM12);		//Count up to the value in OCR1A
	TIMSK1 = _BV(OCIE1A);		//Enable timer interrupt A

}

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
	//Disable interupts during setup
	cli();
  //Set up out inputs and outputs
	LEDDDR |= _BV(LEDPIN);
	//Set up communication
	communication_setup();
	//Set up the ADC
	ADC_setup();
	
	//Clear the structs in preparation of the first cycle
	NewMeasurement(0,&Voltage);
	NewMeasurement(1,&Amperage);
	//Start timer1
	cyclesCompleted = 0;
	timer_setup();
  
	BlinkLED(1000,1);

	#ifdef DEBUGOUT
	printf("TARGET_TIMER_COUNT: %li\n\r",TARGET_TIMER_COUNT); //Can't stringify this without figuring out how to get the preprocessor to evaluate it
	sprint("Sampling "STRINGIFY(MAX_MEASUREMENTS)" Measurements at "STRINGIFY(SAMPLING_FREQUENCY)" HZ\n\r");
	sprint("Initalization Completed\n\r");
	#ifdef RADIOOUT
	radio_transmit();
	#endif
	#endif
	//Enable interupts (mainly the timer, and ADC interupts)
	sei();
}

void sendInfo(){	
	char buffer[8];
	dtostrf(Amperage.average,5,2,buffer);
	SpaceToZero(buffer,8);
	sprint("A=");
	cprint(buffer);
	dtostrf(Voltage.average,6,2,buffer);
	SpaceToZero(buffer,8);
	sprint("&V=");
	cprint(buffer);
	dtostrf(Voltage.average * Amperage.average,7,2,buffer);
	SpaceToZero(buffer,8);
	sprint("&W=");
	cprint(buffer);
	sprint(";\n\r");
	#ifdef RADIOOUT
	radio_transmit();
	#endif
}

#ifdef DEBUGOUT
void debugInfo(){
	char buffer[8];
	//Print out the Reference voltage our ADC is using
	//It should be 3.3V
	dtostrf(Get_Vref(),4,2,buffer);
	printf("Reference Voltage is:  %s\n\r",buffer);
	sprint("\n\rBegining Sampling Sequence\n\r");
	#ifdef RADIOOUT
	radio_transmit();
	#endif
	
	sprint("Sampling Completed\n\r");
	#ifdef RADIOOUT
	radio_transmit();
	#endif
	dtostrf(Voltage.average,6,2,buffer);
	printf("%s V Average;",buffer);
	dtostrf(Voltage.RMS,6,2,buffer);
	printf("%s V RMS;",buffer);
	printf("%i samples for V; ",Voltage.numSamples);
	printf("%li totalAverage for V; ",Voltage.totalAverage);
	printf("%li totalRMS for V; ",Voltage.totalRMS);
	sprint("\n\r");
	#ifdef RADIOOUT
	radio_transmit();
	#endif
	dtostrf(Amperage.average,5,2,buffer);
	printf("%s A Average;",buffer);
	dtostrf(Amperage.RMS,5,2,buffer);
	printf("%s A RMS;",buffer);
	printf("%i samples for A; ",Amperage.numSamples);
	printf("%li totalAverage for A; ",Amperage.totalAverage);
	printf("%li totalRMS for A; ",Amperage.totalRMS);
	sprint("\n\r");
	#ifdef RADIOOUT
	radio_transmit();
	#endif
}
#endif

//A quick mutex so I can track whether I'm going to fast or not.
volatile int measurement_lock = 0;

//This is triggered by my timer
ISR(TIMER1_COMPA_vect){
//ISR(TIMER1_COMPA_vect,ISR_NOBLOCK){
	//sprint(".");
	measurement_lock++;
	if(cyclesCompleted < MAX_MEASUREMENTS){
		//If we're sampling too fast, stop and tell the user.
		//Warning:  Sometimes this still doesn't catch it, and the takeMeasurement functions themselves do and block/hang forever
		if(measurement_lock > 1){
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
				sprint("Warning:  Sampling frequency <"STRINGIFY(SAMPLING_FREQUENCY)"> too fast!!!\n\r");
				BlinkLED(100,20);
				_delay_ms(1000);
			}
		}else{
			takeMeasurement(&Voltage);
			takeAMeasurement(&Amperage);
			cyclesCompleted++;
		}
	} else {
		if(measurement_lock == 1){
			//Calculate the results
			Calculate_V_Result(&Voltage);
			Calculate_A_Result(&Amperage);
			
			//Send the results
			#ifdef DEBUGOUT
			debugInfo();
			#endif
			sendInfo();
			
			//Clear the structs in preparation for a new cycle
			NewMeasurement(0,&Voltage);
			NewMeasurement(1,&Amperage);
			
			//Toggle the LED to let us know that we're done with a cycle
			BlinkLED(100,1);
			
			//Reset the number of cycles completed
			cyclesCompleted = 0;
		}
	}
	measurement_lock--;
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
	for(;;){}
	return 0;
}
