/* 
 * Project Impedance AP style
 * Author: Laura Green
 * Date: April 13, 2024
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "Particle.h"//need to have credentials in ignore file
#include<math.h>
#include <Adafruit_MQTT.h>
#include "Adafruit_MQTT/Adafruit_MQTT_SPARK.h"
#include "Adafruit_MQTT/Adafruit_MQTT.h"
#include "credentials.h"

TCPClient TheClient; 

Adafruit_MQTT_SPARK mqtt(&TheClient,AIO_SERVER,AIO_SERVERPORT,AIO_USERNAME,AIO_KEY); 

Adafruit_MQTT_Publish pubFeedZDataFreq = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/ZDataFreq");
Adafruit_MQTT_Publish pubFeedZDataPulse = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/ZDataPulse");
Adafruit_MQTT_Publish pubFeedZDataPlant = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/ZDataPlant");
Adafruit_MQTT_Publish pubFeedZDataRatio = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/ZDataRatio");

const int PULSEPIN=A5;//PWM pin
const int PULSEREADPIN=A2;
const int PLANTREADPIN=A1;
const int AVSIG=127;//average signal applied to pulse pin 127 produces equal on off (square wave?)
int i;//counter to fill array at all frequecies
float hz;//start hz at 100 (like multipip range is 5-50,000 Hz) Adrian started with 500Hz
float pulse;
float plant;
float ratio;
float maxRatio;//output from read function
float sinwave;
float A;
float t;
float v;//frequency
float B;//offset
unsigned int lastTimePub;
/////////function to read AP style impedance (ratio)///////
float ratReadArray[100][2];//array will contain freq and max ratio for 100 freqs (steps of 500hz)

float ratAPRead(hz);//declare function

void MQTT_connect();
bool MQTT_ping();
// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(AUTOMATIC);

// Run the application and system concurrently in separate threads
//SYSTEM_THREAD(ENABLED);



void setup() {
  Serial.begin(9600);
  waitFor(Serial.isConnected, 10000);

  pinMode(PULSEPIN,OUTPUT);
  pinMode(PULSEREADPIN,INPUT);
  pinMode(PLANTREADPIN, INPUT);

  hz=500;
  A=255/2.0;//amplitude
  t=millis()/1000.0; //time in seconds
  v=500;//frequency
  B=255/2.0;//offset
  i=0;
}


void loop() {
   MQTT_connect();
  MQTT_ping();

  //sinwave=A* sin(2 * M_PI * v * t)+B; //for sin wave, but won't be able to get high enough frequencies

 
  if(i<100){//keep reading until array full
  maxRatio=ratAPRead(hz);
    ratReadArray[i][0]=hz;
    ratReadArray[i][1]=maxRatio;
    Serial.printf("ratioMax at %f=%.2f\n",hz,maxRatio);//after 1 sec print max ratio at that freq, just to check get rid of this later
    i++;
    hz=hz+500;
     }
     if((millis()-lastTimePub > 6000)) {//publishing (how often?)
    if(mqtt.Update()) {
      pubFeedZDataFreq.publish(hz);//publish Hz
      pubFeedZDataRatio.publish(maxRatio);//publish max Freq
      Serial.printf("Publishing %.2f at %.2f\n",maxRatio,hz); 
      } 
    lastTimePub = millis();
  }
  
  //analogWrite(PULSEPIN,AVSIG,hz);//replace with function
  //analogWrite(PULSEPIN,sinwave);//why is amplitude btw 0-255, but output 0-4096?
  /*
  pulse=analogRead(PULSEREADPIN);
  plant=analogRead(PLANTREADPIN);
  ratio=plant/pulse;
  Serial.printf("PULSE=%.2f\nPLANT=%.2f\nRATIO=%.2f\n",pulse,plant,ratio);//usually btw 0-3 occasionally higher (22..)
  delay(1000);
  */
  
}
float ratAPRead(float hz) {//function to measure max plant/pulse ratio at 100 frequencies and put into array to write to sd card
  const int READTIME=1000;
  unsigned int startRead;
  //int hz;
  float pulse;
  float plant;
  float ratio;
  float ratioMax;//max ratio at every one second read
  
ratioMax=0;
startRead=millis();
  while((millis()-startRead)<READTIME) {//read and calculate ratio over and over for 1 sec
    analogWrite(PULSEPIN,AVSIG,hz);
    pulse=analogRead(PULSEREADPIN);
    plant=analogRead(PLANTREADPIN);
    ratio=plant/pulse;

    if(ratio>ratioMax){
    ratioMax=ratio;
    startRead++;

  }
  }
  return ratioMax;
}
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

  



