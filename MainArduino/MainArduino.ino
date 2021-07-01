/*
  Arduino Soil Slave
  Reads data from capacitive sensors and a DS18B20 temperature sensor
  Outputs data using I2C
  
*/

//#define ENABLE_I2C // Uncomment this line if using I2C communication. Default is SERIAL
#define SERIAL_BAUD_RATE 9600
// First we include the libraries
#include <OneWire.h> 
#include <DallasTemperature.h>
#include <Wire.h>
#ifdef ENABLE_I2C
  #include <I2C_Anything.h>
#endif
/********************************************************************/
// Temp sensor bus Data wire is plugged into pin 2 on the Arduino 
#define ONE_WIRE_BUS 2 // Digital pin for the temp sensor
#define HUMID_AIR A0 // Humidity sensor for Air
#define HUMID_SOIL_1 A1
#define HUMID_SOIL_2 A2
#define HUMID_SOIL_3 A3
#define HUMID_SOIL_4 A4
#define HUMID_SOIL_5 A5
#define I2C_ADDR 0x6
#define SDA_PIN A4 // Not needed in code, but just to make sure it's not overwritten
#define SCL_PIN A5 // Not needed in code, but just to make sure it's not overwritten

/********************************************************************/
// Setup a oneWire instance to communicate with any OneWire devices  
// (not just Maxim/Dallas temperature ICs) 
OneWire oneWire(ONE_WIRE_BUS); 
/********************************************************************/
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// Initialize global variables
String ardStatus;
String buf; // Buf is global so it can be sent through I2C on interruption

// the setup routine runs once when you press reset:
void setup() {
  // join i2c bus and register events
  #ifdef ENABLE_I2C
    Wire.begin(I2C_ADDR);            
    Wire.onRequest(requestEvent); 
  #else
    Serial.begin(9600);
  #endif
  
}
void loop() {
  delay(30); 
  updateValues();
  if(Serial.available() > 0) {
    // Only send Data when Raspberry requests it
    String data = Serial.readStringUntil('\n');
    Serial.println(buf);
  }
}

void updateValues(){
  ardStatus = "OK"; // Change if there's some kind of alert // 2 bytes
  // read the temperature on digital sensor  
  sensors.requestTemperatures(); // Send the command to get temperature readings 
  float temp_soil = sensors.getTempCByIndex(0); // 5 bytes when sent (decimal point)
  float temp_ambient = sensors.getTempCByIndex(1); // 5 bytes when sent (decimal point)
  // Read humidity values
  int humidity_ambient = analogRead(HUMID_AIR); // 2 bytes
  int humidity_1 = analogRead(HUMID_SOIL_1); // 2 bytes
  int humidity_2 = analogRead(HUMID_SOIL_2); // 2 bytes
  int humidity_3 = analogRead(HUMID_SOIL_3); // 2 bytes
  #ifdef ENABLE_I2C
    // If I2C is enabled, two pins are taken for SDA/SCL
    int humidity_4 = -1; // 2 bytes
    int humidity_5 = -1; // 2 bytes
  #else
    int humidity_4 = analogRead(HUMID_SOIL_4); // 2 bytes
    int humidity_5 = analogRead(HUMID_SOIL_5); // 2 bytes
  #endif
  
  // STATUS;TEMP_SOIL;TEMP_AMBIENT;HUMIDITY_1;HUMIDITY_2;HUMIDITY_3;HUMIDITY_4;HUMIDITY_5
  // 3     1    5    1    5       1    2     1    2     1    2 
  // Total 24 bytes 
  buf = ardStatus + ";" + temp_soil + ";" + temp_ambient + ";" + humidity_1 + ";" + humidity_2 + ";" + humidity_3 + ";" + humidity_4 + ";" + humidity_5; 
  
}
void blinkLed(){
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);  
}
#ifdef ENABLE_I2C
void requestEvent() {
  // Check this, it sends some data but it doesn't make sense on the receiving end.
  //Wire.write(buf2.c_str()); //Send data*/
  char sendchar[32];
  buf.toCharArray(sendchar,32); // Always +1 and another +1 if you want the closing NUL
  I2C_writeAnything(sendchar);
  
}
#endif
