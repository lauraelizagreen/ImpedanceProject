/* 
 * Project Impedance AP style
 * Author: Laura Green
 * Date: April 13, 2024
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "Particle.h"

const int PULSEPIN=A1;
const int PULSEREADPIN=A2;
const int PLANTREADPIN=A5;
const int AVVOLT=127;//average voltage applied to pulse pin 127 produces equal on off (square wave?)
float hz;//start hz at 100 (like multipip range is 5-50,000 Hz)
float pulse;
float plant;
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
}


void loop() {
  analogWrite(PULSEPIN,AVVOLT,hz);
  pulse=analogRead(PULSEREADPIN);
  plant=analogRead(PLANTREADPIN);
  Serial.printf("PULSE=%f\nPLANT=%f\n",pulse,plant);
  delay(1000);
  
}
