/*
  SD card datalogger
 
  HEAVILY MODIFIED BY GROUP 10
  WARNING: analogRead and string functions and classes DO NOT WORK.
 
 This example shows how to log data from three analog sensors 
 to an SD card using the SD library.
 
 	
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
#define CHIPSELECT 10;  //Use this one for the arduino

//Global Variables
int flipflop=LOW;  //This is used to let me switch my LED on and off easily.


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
  
  //Return the value
  return (ADCH << 8)  | ADCL;
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


//If the File is near/over 3GB stop, close the file and blink the LED slowly
void SizeSafetyCheck(File fileToCheck){
  if(fileToCheck.size() > 24000000000.0){
	fileToCheck.close();
    while(1){
      if(flipflop==HIGH){
        flipflop = LOW;
      }else{
        flipflop = HIGH;
      }
      digitalWrite(LEDPIN, flipflop);   // set the LED
    delay(10000);
    }
  }
}

void BlinkLED(unsigned long milliseconds,int number){
  for(int i=0;i<number;i++){
    digitalWrite(LEDPIN,HIGH);
    delay(milliseconds);
    digitalWrite(LEDPIN,LOW);
    delay(milliseconds);
  }
}

struct VoltageStruct{
  double Voltage;        //This will be my final value
  uint32_t megaVoltage;  //I'm constantly adding lastADCValue squared to this so I can use it to find RMS values
  uint16_t numSamples;   //Number of samples measured
  unsigned long time;    //The time it took me to do all of this
  //Measure the voltage assuming that it's a half rectified signal.
  //Measure the voltage on the selected pin
  void DoIt(unsigned char  pin){
    //Initalize variables
    Voltage = 0;
    megaVoltage = 0;
    numSamples = 0;
    
    uint16_t lastADCValue = 1023; //This needs to be max for my first while loop. 
    uint16_t threashhold = 35;  //Below this value we assume we are only seeing noise.

    //If we start in the middle of a wave, wait untill the next wave before continuing
    while(lastADCValue > threashhold){
      lastADCValue = ADC_read(0);
    }
    while(lastADCValue < threashhold){
      lastADCValue = ADC_read(0);
    }

    //Work and Sample at the same time 
    unsigned long startTime = micros();
    while(lastADCValue > threashhold){
      //Select which pin I'm going to be reading
      unsigned char temp = (ADMUX & 0xf0) | (pin & 0x0f);
      ADMUX = temp;
      //Start the conversion
      ADCSRA |= _BV(ADSC);
      
      //Do Work While waiting for conversion to finish
      megaVoltage += lastADCValue * lastADCValue;
      
      //Keep checking and waiting until the conversion is complete
      while(!(ADCSRA&_BV(ADIF))){
        _NOP();
      }
      //Convert the Conversion into one number
      unsigned char low = ADCL;
      unsigned char high = ADCH;
      lastADCValue = (high << 8)  | low;
      
      //Increment my number of samples
      numSamples++;
    }
    unsigned long endTime = micros();


    //Sanity check, in case noise set it off
    if(numSamples < 60){
      Voltage = 0;
      return;
    }
    
    //Find RMS using megaVoltage
    Voltage = megaVoltage/numSamples;
    Voltage = sqrt(Voltage);
    
    //Convert to real values
    //Should be Voltage = (ADCValue * (5V reference/1024 divisions) + 0.7V to compensate for our diode) * 31 for our voltage divider
    //But our diode is acting up, and is instead causing a massive voltage drop (2.8 V instead of 0.7V)
    Voltage = (Voltage * 5/1024.0 + 2.8) * 31;
    
    //Calculate total time
    time = endTime-startTime;
  }
};

struct AmperageStruct{
  double Amperage;        //This will be my final value
  uint32_t megaAmperage;  //Highest Value Seen from my Hall Effect Sensor
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
	//Sample for 30 milliseconds & output highest vlue seen
    while((millis()-startTime)<30){
      lastADCValue = ADC_read(1);
      if(lastADCValue > megaAmperage)
        megaAmperage = lastADCValue;
      numSamples++;
    }
    unsigned long endTime = millis();
    time = endTime-startTime;
    
    //8 Amps/Volt from my current sensor, with an offset of 2.5V
    Amperage = (megaAmperage * 5/1024.0 - 2.5) * 8;
  }
};

void setup()
{
  //Set up out inputs and outputs
  pinMode(10, OUTPUT);        	//Needed for SD Library
  pinMode(LEDPIN, OUTPUT);
  pinMode(CHIPSELECT,OUTPUT);
  
  //Enable interupts
  ///sei();
  
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
  
  BlinkLED(500,3);
  //File Size check, sanity keeper
  SizeSafetyCheck(dataFile);
  

  // if the file is available, write to it:
  if (dataFile) {
  
    //Measure Voltage
    VoltageStruct measureVoltage;
    measureVoltage.DoIt(0);
    
    //Measure Amperage
    AmperageStruct measureAmperage;
    measureAmperage.DoIt(1);
    
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
    if(flipflop==HIGH){
      flipflop = LOW;
    }else{
      flipflop = HIGH;
    }
      digitalWrite(LEDPIN, flipflop);   // set the LED
    //delay(10000);
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
