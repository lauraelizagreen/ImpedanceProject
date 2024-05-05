/* 
 * Project thermocouples
 * Author: Laura Green
 * Date: April 26, 2024
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "Particle.h"
#include <Wire.h>
#include <SparkFun_MCP9600.h>


const int HEAT=D6;
float tcTemp;
float ambTemp;
float deltaTemp;


MCP9600 tempSensor1;
MCP9600 tempSensor2;
Thermocouple_Type type = TYPE_T; //can change type here

// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(SEMI_AUTOMATIC);

// Run the application and system concurrently in separate threads
//SYSTEM_THREAD(ENABLED);


void setup() {
  Serial.begin(9600);
  Wire.begin();
  Wire.setClock(100000);
  tempSensor1.begin(0x67);
  tempSensor2.begin(0x60);
//check connection
  if(tempSensor1.isConnected()){
    Serial.printf("Device will acknowledge!");
  }
  else{
    Serial.printf("Device did not acknowledge! Freezing.");
    while(1);//stay here
  }
//check device id
  if(tempSensor1.checkDeviceID()){
        Serial.printf("Device ID is correct!");        
    }
    else {
        Serial.printf("Device 1 ID is not correct! Freezing.");
        while(1);
    }

    //change the thermocouple type being used
    Serial.printf("Setting Thermocouple Type!");
    tempSensor1.setThermocoupleType(type);

    //make sure the type was set correctly!
    if(tempSensor1.getThermocoupleType() == type){
        Serial.printf("Thermocouple 1 Type set sucessfully!");
    }

    else{
        Serial.printf("Setting Thermocouple 1 Type failed!");
}
////again with device 2
if(tempSensor2.isConnected()){
    Serial.printf("Device will acknowledge!");
  }
  else{
    Serial.printf("Device 2 did not acknowledge! Freezing.");
    while(1);//stay here
  }
//check device id
  if(tempSensor2.checkDeviceID()){
        Serial.printf("Device ID is correct!");        
    }
    else {
        Serial.printf("Device 2 ID is not correct! Freezing.");
        while(1);
    }

    //change the thermocouple type being used
    Serial.printf("Setting Thermocouple Type!");
    tempSensor2.setThermocoupleType(type);

    //make sure the type was set correctly!
    if(tempSensor2.getThermocoupleType() == type){
        Serial.printf("Thermocouple 2 Type set sucessfully!");
    }

    else{
        Serial.printf("Setting Thermocouple 2 Type failed!");
}

pinMode(HEAT,OUTPUT);
}
void loop(){

digitalWrite(HEAT,HIGH);//turn on heating pad maybe should be analog and control heat later?

  
if(tempSensor1.available()){
  tcTemp=tempSensor1.getThermocoupleTemp();
  ambTemp=tempSensor1.getAmbientTemp();
  deltaTemp=tempSensor1.getTempDelta();
        Serial.printf("TC1: Thermocouple:%0.2f°C\nAmbient:%0.2f°C\nDelta:%0.2f°C\n",tcTemp,ambTemp,deltaTemp);
       delay(2000); 
  }

  if(tempSensor2.available()){
  tcTemp=tempSensor2.getThermocoupleTemp();
  ambTemp=tempSensor2.getAmbientTemp();
  deltaTemp=tempSensor2.getTempDelta();
        Serial.printf("TC2: Thermocouple:%0.2f°C\nAmbient:%0.2f°C\nDelta:%0.2f°C\n",tcTemp,ambTemp,deltaTemp);
       delay(2000); 
  }
}
