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
	// dataFile.print("\noutput:  ");
	// dataFile.print(output);
	// dataFile.print('\n');
}
*/

void takeMeasurement(struct measurements *ourMeasurement){
	ADC_start(ourMeasurement->pin);
	
	//Do Work While waiting for conversion to finish
	ourMeasurement->numSamples++;			//Increment my number of samples
	
	uint16_t lastADCValue = ourMeasurement->lastADCValue;
	ourMeasurement->totalAverage += lastADCValue;
	//ourMeasurement->totalRMS += square(lastADCValue);
	ourMeasurement->totalRMS += lastADCValue*lastADCValue;
	
	//Wait untill the conversion is complete
	ADC_wait_done();
	
		//IF the VALUE IS IMPOSSIBLE TELL SOMEONE
		// if(ADCResult > 1023){
			// dataFile.print("WARNING:  IMPOSSIBLE VALUE:  ");
			// dataFile.print(ADCResult);
			// dataFile.print('\n');
		// }
	
	ourMeasurement->lastADCValue = ADCResult;
}

void Measure(struct measurements *ourMeasurement){
	/*
	uint32_t startTime = millis();
	//Sample for 30 milliseconds
    while((millis()-startTime)<30){
		takeMeasurement(ourMeasurement);
	}
	uint32_t endTime = millis();
    ourMeasurement->time = endTime-startTime;
	*/
	int i;
	for(i=0;i<300;i++){
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