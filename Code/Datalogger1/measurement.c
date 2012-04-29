//Power Measurement functions
//Copyright Arthur Moore 2012
#include "measurement.h"
#include <math.h>
//#include <arduino.h>	//Needed for the millis() function.

void SetMeasurementPin(uint8_t inputPin, struct measurements *ourMeasurement){
	if(inputPin <= 7){
		ourMeasurement->pin = inputPin;
	}
}

void NewMeasurement(uint8_t inputPin, struct measurements * ourMeasurement){
	ResetMeasurement(ourMeasurement);
	SetMeasurementPin(inputPin,ourMeasurement);
}

void ResetMeasurement(struct measurements * ourMeasurement){
	ourMeasurement->pin = 0;
	ourMeasurement->totalAverage = 0;
	ourMeasurement->totalRMS = 0;
	ourMeasurement->average = 0;
	ourMeasurement->RMS = 0;
	ourMeasurement->numSamples = 0;
	ourMeasurement->time = 0;
	ourMeasurement->lastADCValue = 0;
}

//My multiplyer wasn't working, so we're going to use this instead
/*
uint32_t square(uint16_t input){
	uint32_t output = 0;
	for(uint16_t i =0; i<input;i++){
		output+=input;
	}
}
*/

void takeMeasurement(struct measurements *ourMeasurement){
	//Lock to prevent more than one measurement messing with the ADC
	if(!measurement_lock){
		measurement_lock = 1;
		
		ADC_start(ourMeasurement->pin);
		
		//Do Work While waiting for conversion to finish
		ourMeasurement->numSamples++;			//Increment my number of samples
		
		uint16_t lastADCValue = ourMeasurement->lastADCValue;
		ourMeasurement->totalAverage += lastADCValue;
		//ourMeasurement->totalRMS += square(lastADCValue);
		ourMeasurement->totalRMS += lastADCValue*lastADCValue;
		
		//Wait untill the conversion is complete
		ADC_wait_done();
		
		ourMeasurement->lastADCValue = ADCResult;
		
		measurement_lock = 0;
	}else{
		for(;;){;}
	}
}

void Measure(uint16_t number_of_measurements,struct measurements *ourMeasurement){
	int i;
	for(i=0;i<number_of_measurements;i++){
		takeMeasurement(ourMeasurement);
	}
	Calculate_Results(ourMeasurement);
}

void Calculate_Results(struct measurements *ourMeasurement){
	//Find the average value
    ourMeasurement->average = ourMeasurement->totalAverage/ourMeasurement->numSamples;
	
	//Find the RMS value
	ourMeasurement->RMS = ourMeasurement->totalRMS/ourMeasurement->numSamples;
    ourMeasurement->RMS = sqrt(ourMeasurement->RMS);
}