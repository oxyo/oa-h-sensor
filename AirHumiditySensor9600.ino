/*
 * Air hummidity and dust sensor code by V.Gudaitis oxyo@smartsensors.lt
 * Version 1.2
 * 
 * HARDWARE
 * SENSORS: Hummidity - HIH5031; Temperature - DS18B20; Dust - Sharp GP2Y1010AU0F.
 * Smart Sensors Air quality sensor kit
 * https://smartsensors.lt
 * 
 * Library used:
 * Andrew Bythell <abythell@ieee.org> HIH4030 sensor Library
 * https://github.com/angryelectron/tweetpot/tree/master/arduino/HIH4030
 *
 * Dust sensor code from:
 * http://itp.nyu.edu/physcomp/sensors/Reports/GP2Y1010AU
 *
 */

#include <OneWire.h>
#include <DallasTemperature.h>
#include <HIH4030.h>

#define LED 8
#define ONE_WIRE_BUS 9		
#define HIH4030_PIN A0
#define HIH4030_SUPPLY_VOLTAGE  5.0

/*
 * Set the analog reference voltage.  Unless you have set it explicity using
 * analogReference(), this will be the same voltage as your Arduino, either 3.3 or 5.0
 */
#define ARDUINO_VCC 5.0

/*
 * Set Factory Calibration Values, if available.  Here we'll use the values from
 * the datasheet's Table 2: Example Data Printout
 */
#define SLOPE 0.03068
#define OFFSET 0.958

// Global variables
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress thermometer;
HIH4030 uncalibratedHygrometer(HIH4030_PIN, HIH4030_SUPPLY_VOLTAGE, ARDUINO_VCC);
HIH4030 calibratedHygrometer(HIH4030_PIN, HIH4030_SUPPLY_VOLTAGE, ARDUINO_VCC);

//Dust sensor setup
int dustPin=A2;
int dustVal=0;
int dustLedPower=A3;
int delayTime=280;
int delayTime2=60;
float offTime=9680;

void setup(void)
{
  pinMode(dustLedPower,OUTPUT);
  pinMode(LED,OUTPUT);  
  digitalWrite(LED,LOW); //Disable heartbeat LED
  digitalWrite(dustLedPower,HIGH); //Disable dust sensor LED

  Serial.begin(9600);
  delay(1000);

  sensors.begin();
  if (!sensors.getAddress(thermometer, 0)) Serial.println("Unable to find DS18B20"); 

  calibratedHygrometer.calibrate(SLOPE, OFFSET);
}

void loop(void)
{ 
  blinkLED();
  
  //Get DS18B20 temperature value  
  sensors.requestTemperatures(); 
  delay(1000);
  
  printHummidityValue();
  blinkLED(); 

 
}

//FUNCTIONS ====================================================

//MCU heartbeat indication 
void blinkLED(){
 digitalWrite(LED,HIGH);  
 delay(100);
 digitalWrite(LED,LOW); 
 delay(100);
 digitalWrite(LED,HIGH);  
 delay(100);
 digitalWrite(LED,LOW); 
 delay(100);
}


//Debug print all hummidity parameters
void printHummidityData(HIH4030 hygrometer, float temperature) {
  Serial.print("T = ");
  Serial.print(temperature);
  Serial.print("C V = ");
  Serial.print(hygrometer.vout());
  Serial.print("V RH = ");
  Serial.print(hygrometer.getSensorRH());
  Serial.print("% CRH = ");
  Serial.print(hygrometer.getTrueRH(temperature));
  Serial.println("%");
}

//Measure calibrated temperature compensated RH
void hValue(HIH4030 hygrometer, float temperature) { 
  Serial.print(hygrometer.getTrueRH(temperature) + 15);
}

//Print hummidity value
void printHummidityValue() {
  float temperature;
  temperature = sensors.getTempC(thermometer);
  //Serial.print("Uncalibrated: ");
  //printHummidityData(uncalibratedHygrometer, temperature); 
  Serial.print("aH0CRHP");
  hValue(calibratedHygrometer, temperature); 
  Serial.println(); 
  //delay(4000);
  //Serial.print("aT0TMPA");
  //Serial.print(temperature);  
  delay(59000);  
}

//Print air dust concentration value
void printDustValue(){  
  digitalWrite(dustLedPower,LOW); // power on the dust sensor LED
  delayMicroseconds(delayTime);
  dustVal=analogRead(dustPin); // read the dust value
  delayMicroseconds(delayTime2);
  digitalWrite(dustLedPower,HIGH); // turn the dust sensor LED off
  delayMicroseconds(offTime);
  //delay(1000);
  if (dustVal < 10){
    Serial.print("aD0DVAL0.00");
  } else if (dustVal < 100){
    Serial.print("aD0DVAL0.0");
  } else {  
    Serial.print("aD0DVAL0.");
  }
  Serial.print(dustVal);
}

//Measure V supply for precise ADC conversions
long readVcc() { 
  long result; // Read 1.1V reference against AVcc 
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1); 
  delay(2); // Wait for Vref to settle 
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC)); 
  result = ADCL; 
  result |= ADCH<<8; 
  result = 1125300L / result; // Back-calculate AVcc in mV 
  return result; 
} 

//Print VCC value
void printVCC(){
  Serial.print(" VCC:"); 
  Serial.print( readVcc(), DEC ); 
  Serial.println();
}

