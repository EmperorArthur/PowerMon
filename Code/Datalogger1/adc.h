//ADC Functions
//Copyright Arthur Moore 2012

//Note:  The 10-bit ADC's resoultion is 1024 bits this transtlates into:
//		 Resolution = Vref/1024
//		 Accuracy   = +/- Resoultion/2
//		 @3.3V the ADC is accurate to about +/- 2mV
//		 @5V   the ADC is accurate to about +/- 3mV
//Note:  ADC = Vin*1024/Vref
//       To make use of that use this equation:
//       Vin = Vref*ADC/1024
//Note:	 The ADC takes 13 Cycles to run, or 25 if it's done at the same time that ADEN is set.
//		 This translates to either 1664, or 3200 clock cycles at 16MHZ.


#ifndef ADC_H
#define ADC_H 1

//The result from my ADC (0xFFFF lets me know the ADC isn't done yet)
volatile uint16_t ADCResult = 0xFFFF;
//The reference voltage the ADC is using (see associated function for how accurate it is)
volatile float Vref = 0;

//Set up my A/D Converter, setting the entirety of Port C to inputs
void ADC_setup();

//Read from the specified ADC pin
uint16_t ADC_read(unsigned char pin);

//Start an ADC Read from the specified ADC pin (ADCResult is set when read is done)
void ADC_start(unsigned char pin);
//Spinlock untill the ADC Read is done.
uint16_t ADC_wait_done();

//Use the internal 1.1V reference to determine what the Reference Voltage is
float Get_Vref();

//ISR(ADC_vect, ISR_NOBLOCK)

#endif