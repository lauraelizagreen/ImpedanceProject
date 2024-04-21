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
/////////function to read AP style impedance (ratio)///////
float ratReadArray[100][2];//array will contain freq and max ratio for 100 freqs (steps of 500hz)

float ratAPRead(hz);//declare function
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
  //sinwave=A* sin(2 * M_PI * v * t)+B; //for sin wave, but won't be able to get high enough frequencies

 
  if(i<100){//keep reading until array full
  maxRatio=ratAPRead(hz);
    ratReadArray[i][0]=hz;
    ratReadArray[i][1]=maxRatio;
    Serial.printf("ratioMax at %f=%.2f\n",hz,maxRatio);//after 1 sec print max ratio at that freq, just to check get rid of this later
    i++;
    hz=hz+500;
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
  



