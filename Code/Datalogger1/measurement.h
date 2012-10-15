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
#include <stdint.h>
#include "adc.h"

//This is the expected voltage my chip is operating at in Volts (it may be different than Vref)
#define VCC 3.3

struct measurements {
	uint8_t pin;				//The pin we're measuring
	uint32_t totalAverage;		//This is the total value used when averaging
								//(can do this a Max of 4194304 times)
	uint32_t totalRMS;			//This is the total value used when calculating RMS
								//(can do this a Max of 4096 times)
	double average;				//The final average value
	double RMS;					//The final RMS value
	uint16_t numSamples;		//Number of samples measured
	uint16_t lastADCValue;		//The last value from the ADC
};

//If we're measuring something already lock, and don't prevent another measurement
static volatile uint8_t measurement_lock = 0;

void NewMeasurement(uint8_t inputPin, struct measurements *ourMeasurement);
void ResetMeasurement(struct measurements *ourMeasurement);
void SetMeasurementPin(uint8_t inputPin, struct measurements *ourMeasurement);

//Do Work While waiting for conversion to finish
void DoCalculations(struct measurements * ourMeasurement);
//This is needed for the ADC
void DoCalculations(void* ourMeasurement);

//Take a single measurement
void takeMeasurement(struct measurements *ourMeasurement);
//Take a single Amperage measurement
void takeAMeasurement(struct measurements *ourMeasurement);

//Calculate the final values from the measurements gathered
void Calculate_Results(struct measurements *ourMeasurement);
//Specialized for Amperage
void Calculate_A_Result(struct measurements *ourMeasurement);
//And for Voltage
void Calculate_V_Result(struct measurements *ourMeasurement);

//This takes <number_of_measurements> as fast as possible and calculates the results
void Measure(uint16_t number_of_measurements,struct measurements *ourMeasurement);

#endif