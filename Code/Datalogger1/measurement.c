//Power Measurement functions
//Copyright Arthur Moore 2012
#include "measurement.h"
#include <math.h>
#include <stdlib.h>
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
	ourMeasurement->lastADCValue = 0;
}

//Do Work While waiting for conversion to finish
void DoCalculations(struct measurements * ourMeasurement){
		ourMeasurement->numSamples++;
		ourMeasurement->totalAverage += ourMeasurement->lastADCValue;
		//ourMeasurement->totalRMS += ourMeasurement->lastADCValue * ourMeasurement->lastADCValue;
}
//This is needed for the ADC
void DoCalculations(void* ourMeasurement){
	DoCalculations((measurements*) ourMeasurement);
}

void takeMeasurement(struct measurements *ourMeasurement){
	//Do Work While waiting for conversion to finish, and store the result as the last value seen
	ourMeasurement->lastADCValue = ADC_execute_read(ourMeasurement->pin,DoCalculations,(void*)(ourMeasurement));
}

void takeAMeasurement(struct measurements *ourMeasurement){
	//Do Work While waiting for conversion to finish, and store the result as the last value seen
	//Our Hall effect sensor has a 0 value of 503, so we're going off that.
	ourMeasurement->lastADCValue = labs((int)ADC_execute_read(ourMeasurement->pin,DoCalculations,(void*)(ourMeasurement)) - 507);
}

void Measure(uint16_t number_of_measurements,struct measurements *ourMeasurement){
	int i;
	for(i=0;i<number_of_measurements;i++){
		takeMeasurement(ourMeasurement);
	}
	Calculate_Results(ourMeasurement);
}

void Calculate_Results(struct measurements *ourMeasurement){
	//numSamples is always one too high
	//This is because DoCalculations is run the first time with 0 as an input.
	ourMeasurement->numSamples--;
	//Find the average value
    ourMeasurement->average = ourMeasurement->totalAverage/ourMeasurement->numSamples;
	
	//Find the RMS value
	//ourMeasurement->RMS = ourMeasurement->totalRMS/ourMeasurement->numSamples;
    //ourMeasurement->RMS = sqrt(ourMeasurement->RMS);
}

void Calculate_A_Result(struct measurements *ourMeasurement){
	Calculate_Results(ourMeasurement);
	
	//And turn it into a usable number
	//0.068 came from real world testing
	//ourMeasurement->average = ourMeasurement->average * 0.068;
	//512 divisions for 20A
	ourMeasurement->average = 20*(ourMeasurement->average)/512;
	
}

void Calculate_V_Result(struct measurements *ourMeasurement){
	Calculate_Results(ourMeasurement);
	
	//And turn it into a usable number
	//4.74 is the resistor divider
	//+1 is the diode bridge
	//23 is the transformer
	//20 is a constant to make real world values match theoretical ones
	ourMeasurement->average = ((4.74*VCC*(ourMeasurement->average)/1024)+1)*23-20;
	
}