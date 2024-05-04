/* 
 ** Project ThermoCouple Test
 * Author: Brian Rashap
 * Date: 03-MAY-2024
 */

#include "Particle.h"

byte mbs, lbs;
int16_t value;

SYSTEM_MODE(SEMI_AUTOMATIC);

void setup() {
  Serial.begin(9600);
  Wire.begin();
}

void loop() {
  Wire.beginTransmission(0x67);
  Wire.write(0x02);
  Wire.endTransmission(false);
  Wire.requestFrom(0x67, 2, true);
  mbs = Wire.read();
  lbs = Wire.read();
  value = mbs << 8 | lbs;
  Serial.printf("mbs = %i, lbs = %i, value = %i\n",mbs, lbs, value);
  delay(3000);
} 