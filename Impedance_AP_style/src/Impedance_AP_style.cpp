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
float hz;//start hz at 100 (like multipip range is 5-50,000 Hz) Adrian started with 500Hz
float pulse;
float plant;
float ratio;
float sinwave;
float A;
float t;
float v;//frequency
float B;//offset
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
}


void loop() {
  sinwave=A* sin(2 * M_PI * v * t)+B;
  //analogWrite(PULSEPIN,AVSIG,hz);
  analogWrite(PULSEPIN,sinwave);//why is amplitude btw 0-255, but output 0-4096?
  pulse=analogRead(PULSEREADPIN);
  plant=analogRead(PLANTREADPIN);
  ratio=plant/pulse;
  Serial.printf("PULSE=%.2f\nPLANT=%.2f\nRATIO=%.2f\n",pulse,plant,ratio);//usually btw 0-3 occasionally higher (22..)
  delay(1000);
  
}
