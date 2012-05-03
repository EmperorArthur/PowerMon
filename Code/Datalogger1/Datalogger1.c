//Wireless In home Power mOnitoring Datalogger
//Copyright Arthur Moore 2012
 
 
//Uncomment this to enable serial/radio output
#define SERIALOUT 1
//Uncomment this to send to the radio as well as/instead of the serial interface
#define RADIOOUT 1
//Uncomment this to enable aditional debugging output
//#define DEBUGOUT 1

// The pin my status indicator LED is on
#define LEDPIN PD5
#define LEDDDR DDRD
#define LEDPORT PORTD

//How many measurements we want
#define MAX_MEASUREMENTS 300


//This set's my Timer0 frequency (Timer1 counts up to 65535)
//Empirical evidence has shown that with current code, the max sampling frequency is 4086 HZ
#define Sampling_Frequency 3000
#define Prescaler 1
#define Target_Timer_Count (((F_CPU / Prescaler) / Sampling_Frequency) - 1)

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include "adc.h"
#include "measurement.h"
#ifdef SERIALOUT
#include "uart.h"
#ifdef RADIOOUT
#include "radio/radio-uart.h"
#endif
#endif


volatile uint8_t enable_measurement = 0;

void timer_setup(){
	#if Target_Timer_Count > 65535
		#error Target_Timer_Count is too large
	#endif
	#if Prescaler == 1
		TCCR1B = _BV(CS10);						//Set our Prescaler to 1
	#elif Prescaler == 8
		TCCR1B = _BV(CS11);						//Set our Prescaler to 8
	#elif Prescaler == 64
		TCCR1B = _BV(CS10) | _BV(CS11);			//Set our Prescaler to 64
	#elif Prescaler == 256
		TCCR1B = _BV(CS12)						//Set our Prescaler to 256
	#elif Prescaler == 1024
		TCCR1B = _BV(CS10) | _BV(CS12);			//Set our Prescaler to 1024
	#else
		#error Prescaler value is incorrect
	#endif
	
		
	OCR1A = Target_Timer_Count;	//Timer counts up to this value
	TCCR1B |= _BV(WGM12);		//Count up to the value in OCR1A
	TIMSK1 = _BV(OCIE1A);		//Enable timer interrupt A

}

//This is a workaround for printf's inability to print floating point numbers
long int temp, temph, templ;
void ftoi(float floating_point_number) {temp = floating_point_number*100; temph = temp/100; templ = temp%100;}

//This lets me store pure strings in flash instead of data.
#define sprint(string) nprintf(PSTR(string))
void nprintf (PGM_P s) {
        char c;
        while ((c = pgm_read_byte(s++)) != 0){
			#ifdef SERIALOUT
			uart_putchar(c);
			#endif
			#ifdef RADIOOUT
			radio_putchar(c);
			#endif
		}
		#ifdef RADIOOUT
		radio_transmit();
		#endif
}

#ifdef SERIALOUT
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
	LEDDDR |= _BV(LEDPIN);\
	
	// UART
	#ifdef SERIALOUT
	DDRD |= (1<<PD1);
	DDRD |= (1<<PD2);
	uart_setup();
	#ifdef RADIOOUT
	radio_setup();
	#endif
	stdout = &uart_stream;
	#endif
	

	//Set upt he ADC
	ADC_setup();
	//Start timer1
	timer_setup();
  
	BlinkLED(1000,1);

  
  
  //BlinkLED(1000,1);
	#ifdef SERIALOUT
	#ifdef DEBUGOUT
	printf("%li\n\r",Target_Timer_Count);
	printf("Sampling %i Measurements at %i HZ\n\r",MAX_MEASUREMENTS,Sampling_Frequency);
	sprint("Initalization Completed\n\r");
	radio_transmit();
	#endif
	#endif
  //Enable interupts (mainly the timer, and ADC interupts)
  sei();
}

void loop()
{
	//Print out the Reference voltage our ADC is using
	//It should be 3.3V
	#ifdef SERIALOUT
	#ifdef DEBUGOUT
	ftoi(Get_Vref());
	printf("Reference Voltage is:  %li.%.2li\n\r",temph,templ);
	sprint("Begining Sampling Sequence\n\r");
	radio_transmit();
	#endif
	#endif

		NewMeasurement(0,&Voltage);
		NewMeasurement(1,&Amperage);

		//Measure(MAX_MEASUREMENTS,&Voltage);
		//Measure(MAX_MEASUREMENTS,&Amperage);
		enable_measurement = 1;
		while(enable_measurement){;}
		Calculate_V_Result(&Voltage);
		Calculate_A_Result(&Amperage);
		#ifdef SERIALOUT
		#ifdef DEBUGOUT
		sprint("Sampling Completed\n\r");
		ftoi(Voltage.average);
		printf("%li.%.2li V Average;",temph,templ);
		ftoi(Voltage.RMS);
		printf("%li.%.2li V RMS;",temph,templ);
		printf("%i samples for V; ",Voltage.numSamples);
		printf("%li totalAverage for V; ",Voltage.totalAverage);
		printf("%li totalRMS for V; ",Voltage.totalRMS);
		sprint("\n\r");
		radio_transmit();
		ftoi(Amperage.average);
		printf("%li.%.2li A Average;",temph,templ);
		ftoi(Amperage.RMS);
		printf("%li.%.2li A RMS;",temph,templ);
		printf("%i samples for A; ",Amperage.numSamples);
		printf("%li totalAverage for A; ",Amperage.totalAverage);
		printf("%li totalRMS for A; ",Amperage.totalRMS);
		sprint("\n\r");
		#endif
		
		ftoi(Amperage.average);
		printf("%li.%.2liA; ",temph,templ);
		ftoi(Voltage.average);
		printf("%li.%.2liV; ",temph,templ);
		ftoi(Voltage.average * Amperage.average);
		printf("%li.%.2liW;;",temph,templ);
		#ifdef RADIOOUT
		radio_transmit();
		#else
		sprint("\n\r");
		#endif
		#endif
		//Toggle the LED to let us know that we're done with a cycle
		BlinkLED(100,1);
}
//This is triggered by my timer
//takeMeasurement uses the ADC interupt so this has to be nonblocking
ISR(TIMER1_COMPA_vect,ISR_NOBLOCK){
	//sprint(".");
	//If we're sampling too fast, stop and tell the user.
	//Warning:  Sometimes this still doesn't catch it, and the takeMeasurement functions themselves do and block/hang forever
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
			takeAMeasurement(&Amperage);
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
