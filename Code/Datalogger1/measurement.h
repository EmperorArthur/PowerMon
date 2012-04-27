//Power Measurement functions
//Copyright Arthur Moore 2012

//Usage:
//measurements var_name;
//NewMeasurement(var_name);
//while(...)
//	takeMeasurement(var_name);
//Calculate_Results(var_name);

#ifndef MEASUREMENT_H
#define MEASUREMENT_H 1
#include "adc.h"

struct measurements{
	int pin;					//The pin we're measuring
	uint32_t totalAverage;		//This is the total value used when averaging
								//(can do this a Max of 4194304 times)
	uint32_t totalRMS;			//This is the total value used when calculating RMS
								//(can do this a Max of 4096 times)
	double average;				//The final average value
	double RMS;					//The final RMS value
	uint16_t numSamples;		//Number of samples measured
	unsigned long time;    		//The time it took me to do all of this
	uint16_t lastADCValue;		//The last value from the ADC
};

void NewMeasurement(int inputPin,measurements * ourMeasurement);
void NewMeasurement(measurements * ourMeasurement);

//My multiplyer wasn't working, so we're going to use this instead
uint32_t square(uint16_t input);

//Take a single measurement
void takeMeasurement(measurements * ourMeasurement);

//Calculate the final values from the measurements gathered
void Calculate_Results(measurements * ourMeasurement);

//This measures for 30 milliseconds and calculates the results
void Measure(measurements * ourMeasurement);

#endif