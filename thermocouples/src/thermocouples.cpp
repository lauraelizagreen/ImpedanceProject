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
#include <Adafruit_MQTT.h>
#include "Adafruit_MQTT/Adafruit_MQTT_SPARK.h"
#include "Adafruit_MQTT/Adafruit_MQTT.h"
#include "credentials.h"

TCPClient TheClient; 

Adafruit_MQTT_SPARK mqtt(&TheClient,AIO_SERVER,AIO_SERVERPORT,AIO_USERNAME,AIO_KEY);
Adafruit_MQTT_Publish pubFeedTCData1 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/tc1Data");
Adafruit_MQTT_Publish pubFeedTCData2 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/tc2Data");
MCP9600 tempSensor1;
MCP9600 tempSensor2;
Thermocouple_Type type = TYPE_T; //can change type here
const int HEAT=D6;
float tc1Temp;
float amb1Temp;
float delta1Temp;
float tc2Temp;
float amb2Temp;
float delta2Temp;

unsigned int lastTimeMeas;

//declare functions
void MQTT_connect();//funtions to connect and maintain connection to Adafruit io
bool MQTT_ping();

// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(AUTOMATIC);

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
lastTimeMeas=millis();
}
void loop(){
  MQTT_connect();
  MQTT_ping();

digitalWrite(HEAT,HIGH);//turn on heating pad maybe should be analog and control heat later?

 if((millis()-lastTimeMeas > 10000)) { 
if(tempSensor1.available()){
  tc1Temp=tempSensor1.getThermocoupleTemp();
  amb1Temp=tempSensor1.getAmbientTemp();
  delta1Temp=tempSensor1.getTempDelta();
        Serial.printf("TC1: Thermocouple:%0.2f°C\nAmbient:%0.2f°C\nDelta:%0.2f°C\n",tc1Temp,amb1Temp,delta1Temp);
       delay(2000); 
  }

  if(tempSensor2.available()){
  tc2Temp=tempSensor2.getThermocoupleTemp();
  amb2Temp=tempSensor2.getAmbientTemp();
  delta2Temp=tempSensor2.getTempDelta();
        Serial.printf("TC2: Thermocouple:%0.2f°C\nAmbient:%0.2f°C\nDelta:%0.2f°C\n",tc2Temp,amb2Temp,delta2Temp);
       delay(2000); 
  }
  if(mqtt.Update()) {
       pubFeedTCData1.publish(tc1Temp);//publish max ratio
      Serial.printf("Publishing tc1=%0.2fC\n",tc1Temp);
      pubFeedTCData2.publish(tc1Temp);//publish max ratio
      Serial.printf("Publishing tc1=%0.2fC\n",tc2Temp);
       
      } 
      
      
     lastTimeMeas=millis();

}
}
/// define functions
void MQTT_connect() {//actually connects to server, if not connected stuck in loop
  int8_t ret;//photon 2 thinks of integers as 32bits
 
  // Return if already connected.
  if (mqtt.connected()) {
    return;
  }
 
  Serial.print("Connecting to MQTT... ");
 
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.printf("Error Code %s\n",mqtt.connectErrorString(ret));
       Serial.printf("Retrying MQTT connection in 5 seconds...\n");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds and try again
  }
  Serial.printf("MQTT Connected!\n");
}

bool MQTT_ping() {//broker will disconnect if doesn't hear anything, just reminding broker still here so don't disconnect
  static unsigned int last;
  bool pingStatus;

  if ((millis()-last)>120000) {
      Serial.printf("Pinging MQTT \n");
      pingStatus = mqtt.ping();
      if(!pingStatus) {
        Serial.printf("Disconnecting \n");
        mqtt.disconnect();
      }
      last = millis();
  }
  return pingStatus;
}
