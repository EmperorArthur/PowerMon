//ADC Functions
//Copyright Arthur Moore 2012

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "adc.h"

//The result from my ADC (0xFFFF lets me know the ADC isn't done yet)
volatile uint16_t ADCResult;
//The reference voltage the ADC is using (see associated function for how accurate it is)
volatile float Vref;

//Get the value from the ADC
//This funciton is used everywhere.
//While it's normally a single line, that one line takes some explaining.
uint16_t get_ADCResult(){
	//I need to do it this way because the ATMega does register locking.
	//This means that these two instructions are atomic if done in this order
	//Also, ADCL is undefined if ADCH is read first.
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		ADCResult = ADCL|(ADCH << 8);
	}
	return ADCResult;
}

//Set up my A/D Converter
//Note:  The ADC is on port C, this code just assumes that the whole port is used exclusively by the ADC
void ADC_setup(){
  DDRC = 0;      //All of Port C is an input
  PORTC = 0;     //With All of my pull up resistors Disabled
  
  //Reset whatever Arduino has done, and set our reference voltage to AVcc (Don't forget to set ADMUX before starting the ADC)
  ADMUX  = _BV(REFS0);		//Set our reference Voltage to AVcc
  
  //Enable the ADC, while at the same time removing whatever the Arduino stuff has done to everything.
  ADCSRA = _BV(ADEN);		//ADEN = Enable the ADC
  //ADC needs it's clock to be between 50KHz to 200KHz.
  ADCSRA |= _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);	//Prescalar = Fcpu/128 (16MHZ CPU so, 125KHz ADC);
  
  //Many functions that are waiting on the ADC interupt are expecting this untill it's done.
  ADCResult = 0xFFFF;
  
  //Go ahead and determine what the actual reference voltage is (even if it's not that accurate)
  Get_Vref();
	
}

//Read from the specified ADC pin
//This spinlocks untill the conversion is complete
//It has the advantage of working even with interupts disabled
uint16_t ADC_read(unsigned char pin){
  //Select which pin I'm going to be reading (if it isn't already selected)
  //See table 24-4 on P.265 for the extra things the ADC can read.
  if( (ADMUX & 0x0f) != pin ){
	ADMUX = (ADMUX & 0xf0) | (pin & 0x0f);
  }
  
  //Disable the ADC Interupt vector
  ADCSRA &= ~_BV(ADIE);
  //Start the conversion
  ADCSRA |= _BV(ADSC);
  
  //Keep checking and waiting until the conversion is complete
  while(!(ADCSRA & _BV(ADIF))){;}
  
  //Return the result
  return get_ADCResult();
}

//Start an ADC Read from the specified ADC pin (Interupt is triggered when read is done)
//NOTE:  Global interupts must be enabled for this to work
void ADC_start(unsigned char pin){
  //Select which pin I'm going to be reading (if it isn't already selected)
  //See table 24-4 on P.265 for the extra things the ADC can read.
  if( (ADMUX & 0x0f) != pin ){
	ADMUX = (ADMUX & 0xf0) | (pin & 0x0f);
  }
  
  ADCSRA |= _BV(ADIE);			//Enable the ADC Interupt vector
  ADCSRA |= _BV(ADSC);			//Start the ADC

  //Many functions that are waiting on the ADC interupt are expecting this untill it's done.
  ADCResult = 0xFFFF;
}

//This is the ADC interupt vector
//This is executed after the ADC is done
ISR(ADC_vect) {
	//Don't do anything if something else is faster.
	if(ADCResult == 0xFFFF)
		get_ADCResult();
}

//This waits until ADCResult is set.
//Use it after ADC_start(...)
uint16_t ADC_wait_done(){
	//Keep checking and waiting until the conversion is complete
	while(ADCResult == 0xFFFF){;}
	return ADCResult;
}

//Read from the specified ADC pin, and execute a function while waiting for the result.
//WARNING:  This will not return with a result untill the function is done executing.
uint16_t ADC_execute_read (unsigned char pin, void (*function)(void*),void* function_paramaters){
  //Select which pin I'm going to be reading (if it isn't already selected)
  //See table 24-4 on P.265 for the extra things the ADC can read.
  if( (ADMUX & 0x0f) != pin ){
	ADMUX = (ADMUX & 0xf0) | (pin & 0x0f);
  }
  
  //Disable the ADC Interupt vector
  ADCSRA &= ~_BV(ADIE);
  //Start the conversion
  ADCSRA |= _BV(ADSC);
  
  //Execute the function
  (*function)(function_paramaters);
  //Keep checking and waiting until the conversion is complete
  while(!(ADCSRA & _BV(ADIF))){;}
	
  //Return the result
  return get_ADCResult();
}

//This function uses the internal 1.1V reference to determine what the Reference Voltage is
//We can use this to correct for any errors and dynamically use a reference voltage.
//NOTE:  It doesn't work if the reference voltage is set to the internal 1.1V :(
//NOTE:  The Resolution of this isn't the best.
//			Resolution = (1.1*1024/(ADC-1)) - (1.1*1024/ADC)
//			Accuracy   = +/- Resoultion/2
//			@near 3.3V it's accurate to about +/- 5mV
//			@near 5V   it's accurate to about +/- 12mV
float Get_Vref(){
	//This function gives nonsensical values if I don't read the 1.1 Vref multiple times
	ADC_read(14);
	ADC_read(14);
	ADC_read(14);
	ADC_read(14);
	ADC_read(14);
	ADC_read(14);
	ADC_read(14);
	ADC_read(14);
	ADC_read(14);
	//Read the 1.1V Reference, and use this equation to tell us our result
	Vref = 1.1 * 1024 / ADC_read(14);
	return Vref;
}
