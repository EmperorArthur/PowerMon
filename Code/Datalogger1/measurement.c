//Power Measurement functions
//Copyright Arthur Moore 2012
#include "measurement.h"
#include <math.h>
#include <arduino.h>	//Needed for the millis() function.
/*
struct VoltageStruct{
  double Voltage;        //This will be my final value
  uint32_t megaVoltage;  //I'm constantly adding lastADCValue squared to this so I can use it to find RMS values
						 //(can do this a Max of 4104 times)
  uint16_t numSamples;   //Number of samples measured
  unsigned long time;    //The time it took me to do all of this
  //Measure the voltage assuming that it's a full rectified signal.
  //Measure the voltage on the selected pin
  void DoIt(unsigned char  pin){
    //Initalize variables
    Voltage = 0;
    megaVoltage = 0;
    numSamples = 0;
	
	uint16_t lastADCValue = 0;


    //Work and Sample at the same time 
    //unsigned long startTime = micros();
	unsigned long startTime = millis();
	while((millis()-startTime)<30){
		ADC_start(pin);
		
		//Do Work While waiting for conversion to finish
		numSamples++;			//Increment my number of samples
		//megaVoltage += lastADCValue * lastADCValue;
		megaVoltage += lastADCValue;
		
		//Wait untill the conversion is complete
		ADC_wait_done();
		
		lastADCValue = ADCResult;
    }
    //unsigned long endTime = micros();
	unsigned long endTime = millis();
    
    //Find RMS using megaVoltage
    Voltage = megaVoltage/numSamples;
    //Voltage = sqrt(Voltage);
    
    //Convert to real values
    //Should be Voltage = (ADCValue * (5V reference/1024 divisions) + 0.7V to compensate for our diode) * 31 for our voltage divider
    //But our diode is acting up, and is instead causing a massive voltage drop (2.8 V instead of 0.7V)
    Voltage = (Voltage * Vref/1024.0);// + 2.8) * 31;
    
    //Calculate total time
    time = endTime-startTime;
  }
};

struct AmperageStruct{
  double Amperage;        //This will be my final value
  uint32_t megaAmperage;  //I'm constantly adding a rectified lastADCValue^2 to this so I can use it to find RMS values
						  //(can do this a Max of 16448 times)
  uint16_t numSamples;   //Number of samples measured
  unsigned long time;    //The time it took me to do all of this
  //Measure the Amperage on the selected pin (using the output from an ACS712 20A Current sensor)
  void DoIt(unsigned char  pin){
    //Initalize variables
    Amperage = 0;
    megaAmperage = 0;
    numSamples = 0;
    
    uint16_t lastADCValue = 0; //The Last value of the ADC

    unsigned long startTime = millis();
	//Sample for 30 milliseconds
    while((millis()-startTime)<30){
		ADC_start(pin);
		
		//Do Work While waiting for conversion to finish
		numSamples++;			//Increment my number of samples
		
		//The hall effect sensor has 0A at Vcc/2
		//In order for the rms equation I'm using to work right I need to rectify the value
		//So I'm using a quick and dirty software rectifier
		//This cuts my ADC down to 512 bits resolution or about +/-3mv Accuracy @ 3.3V
		// if(lastADCValue >= 512){
			// lastADCValue -= 512;
		// }
		
		//megaAmperage += lastADCValue * lastADCValue;
		megaAmperage += lastADCValue;
    
		//Wait untill the conversion is complete
		ADC_wait_done();
	
		lastADCValue = ADCResult;
      
		//output highest value seen
		// lastADCValue = ADC_read(pin);
		// if(lastADCValue > megaAmperage)
			// megaAmperage = lastADCValue;
    }
    unsigned long endTime = millis();
    time = endTime-startTime;
	
	//Find RMS using megaAmperage
    Amperage = megaAmperage/numSamples;
    //Amperage = sqrt(Amperage);
    
    
    //Amperage = (megaAmperage * Vref/1024.0)
	
	//8 Amps/Volt from my current sensor, with an offset of Vcc/2
	//Amperage = megaAmperage;
  }
};
*/

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
	uint32_t startTime = millis();
	//Sample for 30 milliseconds
    while((millis()-startTime)<30){
		takeMeasurement(ourMeasurement);
	}
	uint32_t endTime = millis();
    ourMeasurement->time = endTime-startTime;
	
	Calculate_Results(ourMeasurement);
}

void Calculate_Results(struct measurements *ourMeasurement){
	//Find the average value
    ourMeasurement->average = ourMeasurement->totalAverage/ourMeasurement->numSamples;
	
	//Find the RMS value
	ourMeasurement->RMS = ourMeasurement->totalRMS/ourMeasurement->numSamples;
    ourMeasurement->RMS = sqrt(ourMeasurement->RMS);
}