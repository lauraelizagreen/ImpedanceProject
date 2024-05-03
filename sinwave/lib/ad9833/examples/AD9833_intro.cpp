/* 
 * Project myProject
 * Author: Your Name
 * Date: 
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "Particle.h"
#include "AD9833.h"

SYSTEM_MODE(SEMI_AUTOMATIC);

const int AD9833_FSYNC = D14;
const int MASTER_CLOCK = 25000000;
const int MODE_SINE = 0;
unsigned long frequency;

AD9833 sineGen(AD9833_FSYNC, MASTER_CLOCK);

void setup() {
  Serial.begin(9600);
  Serial.printf("Starting Serial Monitor...\n");
  delay(5000);

  frequency = 1000; // initial frequency 1KHz

  sineGen.reset(1);           // Place ad9833 into reset
  Serial.printf("AD9833 in reset\n");
  sineGen.setFreq(frequency); // Set initial frequency to 1 KHz
  Serial.printf("AD9833 frequency set\n");
  sineGen.setPhase(0);        // Set initial phase offset to 0
  Serial.printf("AD9833 phase set\n");
  sineGen.setFPRegister(1);
  Serial.printf("AD9833 FP reg set to 1\n");
  sineGen.setFreq(frequency);
  Serial.printf("AD9833 frequency set\n");
  sineGen.setPhase(0);
  Serial.printf("AD9833 phase set\n");
  sineGen.setFPRegister(0);
  Serial.printf("AD9833 FP reg set to 0\n");
  sineGen.mode(MODE_SINE);    // Set output mode to sine wave
  Serial.printf("AD9833 mode set to sine wave\n");
  sineGen.reset(0);           // Take ad9833 out of reset
  Serial.printf("AD9833 out of reset\n");



}

void loop() {

}
