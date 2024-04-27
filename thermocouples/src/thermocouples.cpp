/* 
 * Project thermocouples
 * Author: Laura Green
 * Date: April 26, 2024
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "Particle.h"
#include <SparkFun_MCP9600.h>


float tcTemp;
float ambTemp;
float deltaTemp;

MCP9600 tempSensor;
Thermocouple_Type type = TYPE_K; //can change type here

// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(AUTOMATIC);

// Run the application and system concurrently in separate threads
//SYSTEM_THREAD(ENABLED);


void setup() {
  Serial.begin(9600);
  Wire.begin();
  Wire.setClock(100000);
  tempSensor.begin();
//check connection
  if(tempSensor.isConnected()){
    Serial.printf("Device will acknowledge!");
  }
  else{
    Serial.printf("Device did not acknowledge! Freezing.");
    while(1);//stay here
  }
//check device id
  if(tempSensor.checkDeviceID()){
        Serial.printf("Device ID is correct!");        
    }
    else {
        Serial.printf("Device ID is not correct! Freezing.");
        while(1);
    }

    //change the thermocouple type being used
    Serial.printf("Setting Thermocouple Type!");
    tempSensor.setThermocoupleType(type);

    //make sure the type was set correctly!
    if(tempSensor.getThermocoupleType() == type){
        Serial.printf("Thermocouple Type set sucessfully!");
    }

    else{
        Serial.printf("Setting Thermocouple Type failed!");
}
}
void loop(){

  
if(tempSensor.available()){
  tcTemp=tempSensor.getThermocoupleTemp();
  ambTemp=tempSensor.getAmbientTemp();
  deltaTemp=tempSensor.getTempDelta();
        Serial.printf("Thermocouple:%0.2f°C\nAmbient:%0.2f°C\nDelta:%0.2f°C\n",tcTemp,ambTemp,deltaTemp);
       delay(2000); 
  }
}
