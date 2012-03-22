/*
  SD card datalogger
 
  HEAVILY MODIFIED BY GROUP 10
  WARNING: analogRead and string functions and classes DO NOT WORK.
 	
 The circuit:
 * analog sensors on analog ins 0, 1, and 2
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11q
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 10
 
 created  24 Nov 2010
 updated 2 Dec 2010
 by Tom Igoe
 
 	 
 */
 
//My includes
#include <SD.h>
#include <avr/io.h>
#include <avr/interrupt.h>
 
//We need a nop function
#define _NOP() __asm__ __volatile__("nop")

//Uncomment this to enable serial output (WARNING, serial output currently does not work with SD card, not enough RAM)
//#define TESTSERIALOUT 1;

// The pin my status indicator LED is on
//#define LEDPIN 7
#define LEDPIN 5

//The chip select pin for the SD card
#define CHIPSELECT 1  	//Use this one for the prototype board
//#define CHIPSELECT 10  //Use this one for the arduino

//Global Variables
int flipflop=LOW;			//This is used to let me switch my LED on and off easily.
uint16_t ADCResult = 0;		//This is the result from my ADC


//Set up my A/D Converter
//Note:  The ADC is on port C, this code just assumes that the whole port is used exclusively by the ADC
//		 The ADC takes 13 Cycles to run, or 25 if it's done at the same time that ADEN is set.
//		 This translates to 1664 clock cycles.
void ADC_setup(){
  DDRC = 0;      //All of Port C is an input
  PORTC = 0;     //With All of my pull up resistors Disabled
  
  //Reset whatever Arduino has done, and set our reference voltage to AVcc (Don't forget to set ADMUX before starting the ADC)
  ADMUX  = _BV(REFS0);		//Set our reference Voltage to AVcc
  
  //Enable the ADC, while at the same time removing whatever the Arduino stuff has done to everything.
  ADCSRA = _BV(ADEN);		//ADEN = Enable the ADC
  //ADC needs it's clock to be between 50KHz to 200KHz.
  ADCSRA |= _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);	//Prescalar = Fcpu/128 (16MHZ CPU so, 125KHz ADC);
	
}

//Read from the specified ADC pin
uint16_t ADC_read(unsigned char pin){
  //Select which pin I'm going to be reading
  //See table 24-4 on P.265 for the extra things the ADC can read.
  ADMUX = (ADMUX & 0xf0) | (pin & 0x0f);
  
  //Start the conversion
  ADCSRA |= _BV(ADSC);
  
  //Keep checking and waiting until the conversion is complete
  while(!(ADCSRA & _BV(ADIF))){
    _NOP();
  }
  
  //I need to do it this way because the ATMega does register locking.
  //This means that these two instructions are atomic if done in this order
  //Also, ADCL is undefined if ADCH is read first.
  return ADCL|(ADCH << 8);
}

//Start an ADC Read from the specified ADC pin (Interupt is triggered when read is done)
//NOTE:  Global interupts must be enabled for this to work
void ADC_start(unsigned char pin){
  //Select which pin I'm going to be reading
  //See table 24-4 on P.265 for the extra things the ADC can read.
  ADMUX = (ADMUX & 0xf0) | (pin & 0x0f);
  
  ADCSRA |= _BV(ADIE);			//Enable the ADC Interupt vector
  ADCSRA |= _BV(ADSC);			//Start the ADC

}

//This ADC interupt vector
//This is executed after the ADC is done
//NOTE:  To get the LED to blink I've allowed interupts inside this interupt
//ISR(ADC_vect) {
ISR(ADC_vect, ISR_NOBLOCK) {

	
	//I need to do it this way because the ATMega does register locking.
	//This means that these two instructions are atomic if done in this order
	//Also, ADCL is undefined if ADCH is read first.
	ADCResult = ADCL | (ADCH << 8);

}

void SD_setup(){
  #ifdef TESTSERIALOUT
    Serial.println("Initializing SD card...");
  #endif
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  
  // see if the card is present and can be initialized:
  if (!SD.begin(CHIPSELECT)) {
    #ifdef TESTSERIALOUT
      Serial.println("Card failed, or not present");
    #endif
    BlinkLED(100,30);
  }else{
	  #ifdef TESTSERIALOUT
		Serial.println("card initialized.");
	  #endif
  }
}


//If the File is near/over 3GB stop, close the file and blink the LED quickly
void SizeSafetyCheck(File fileToCheck){
  if(fileToCheck.size() > 24000000000.0){
	fileToCheck.close();
    while(1){
		BlinkLED(100,30);
    }
  }
}

//Blink the LED
//NOTE:  total delay ~= 2 * milliseconds * number
void BlinkLED(unsigned long milliseconds,int number){
  for(int i=0;i<number;i++){
    digitalWrite(LEDPIN,HIGH);
    delay(milliseconds);
    digitalWrite(LEDPIN,LOW);
    delay(milliseconds);
  }
}

//This is me cheating
//#include <power-measurement.c>

void setup()
{
  //Set up out inputs and outputs
  pinMode(10, OUTPUT);        	//Needed for SD Library
  pinMode(LEDPIN, OUTPUT);
  pinMode(CHIPSELECT,OUTPUT);
  
  //Enable interupts
  sei();
  
  BlinkLED(1000,3);
  
  #ifdef TESTSERIALOUT
    Serial.begin(9600);
  #endif
  
  //Set up the SD card
  SD_setup();
  
  //Set upt he ADC
  ADC_setup();
  
  BlinkLED(1000,3);
}

void loop()
{
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile;
  dataFile = SD.open("datalog.txt", FILE_WRITE);
  
  //File Size check, sanity keeper
  SizeSafetyCheck(dataFile);
  

  // if the file is available, write to it:
  if (dataFile) {
  
	//read from the temperature sensor (This is a good way to check if our ADC is working)
	ADMUX  = _BV(REFS0) | _BV(REFS1);		//Set our reference Voltage to internal 1.1V for the temperature sensor to work right
	int temperature = ADC_read(8);
	/*
	ADC_start(8);
	while(ADCResult == 0){
		_NOP();
	}
	int temperature = ADCResult;
	ADCResult = 0;
	/*
	dataFile.print("Temperature is:  ");
	dataFile.print(temperature);

/*
    //Measure Voltage
    VoltageStruct measureVoltage;
    //measureVoltage.DoIt(0);
    
    //Measure Amperage
    AmperageStruct measureAmperage;
    //measureAmperage.DoIt(1);
    
      //Output our data/results
      dataFile.print(measureVoltage.Voltage);
      dataFile.print(" V RMS; ");
      dataFile.print(measureAmperage.Amperage);
      dataFile.print(" A RMS; ");
      dataFile.print(measureAmperage.Amperage*measureVoltage.Voltage);
      dataFile.print(" W RMS; ");
//      dataFile.print(measureVoltage.numSamples);
//      dataFile.print(" samples for V; ");
//      dataFile.print(measureAmperage.numSamples);
//      dataFile.print(" samples for A; ");
//      dataFile.print(measureVoltage.time);
//      dataFile.print(" microseconds for V");
//      dataFile.print(measureAmperage.time);
//      dataFile.print(" milliseconds for A");
*/
      dataFile.write('\n');
      dataFile.flush();
      dataFile.close();
      
      // print to the serial port too:
      #ifdef TESTSERIALOUT
        Serial.print(measureVoltage.Voltage);
        Serial.print(" V RMS; ");
        Serial.print(measureAmperage.Amperage);
        Serial.print(" A RMS; ");
        Serial.print(measureAmperage.Amperage*measureVoltage.Voltage);
        Serial.print(" W RMS; ");
        Serial.print("\n");
      #endif
    
    
    
    //Blink the LED to let us know that we're done with a cycle
    BlinkLED(1000,1);
  }  
  // if the file isn't open, pop up an error:
  else {
    #ifdef TESTSERIALOUT
      Serial.println("error opening datalog.txt");
    #endif
    while(1){
      BlinkLED(100,100);
    }
  }
}
