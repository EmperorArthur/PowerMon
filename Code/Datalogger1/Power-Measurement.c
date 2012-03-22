//Power Measurement functions
//Copyright Arthur Moore 2012

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