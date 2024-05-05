/* 
 ** Project ThermoCouple Test
 * Author: Brian Rashap
 * Date: 03-MAY-2024
 */

#include "Particle.h"
const int TCADDR1=0x67;
const int TCADDR2=0x60;//grounded tc
byte x,y;
int16_t hotValue, deltValue, coldValue;
//declare function
int16_t shiftOr(byte msb,byte lsb);
SYSTEM_MODE(SEMI_AUTOMATIC);

void setup() {
  Serial.begin(9600);
  Wire.begin();
  Wire.beginTransmission(TCADDR1);
Wire.write(0x05);//register to set tc type
  Wire.write(0x20);//0x20 (32) for type T
   Wire.endTransmission(true);

    Wire.beginTransmission(TCADDR2);//for tc 2 too
Wire.write(0x05);//register to set tc type
  Wire.write(0x20);//0x20 (32) for type T
   Wire.endTransmission(true);



}

void loop() {
  Wire.beginTransmission(TCADDR1);//unaltered address
  Wire.write(0x00);//register for delta cold-hot (multiply by seebeck coefficient (0.041 near 25C))
  Wire.endTransmission(false);
  Wire.requestFrom(0x67, 6, true);//read 2 bytes
  x = Wire.read();
  y = Wire.read();
  hotValue = shiftOr(x,y);//call function to shift/or
  x=Wire.read();
  y = Wire.read();
  deltValue = shiftOr(x,y);
  x=Wire.read();
  y = Wire.read();
  coldValue = shiftOr(x,y);
  Serial.printf("TC1\nhot junction = %i, delta = %i, cold junction = %i\n",hotValue, deltValue, coldValue);
  delay(3000);

   Wire.beginTransmission(TCADDR2);//grounded address
  Wire.write(0x00);//register for delta cold-hot (multiply by seebeck coefficient (0.041 near 25C))
  Wire.endTransmission(false);
  Wire.requestFrom(0x60, 6, true);//read 2 bytes
  x = Wire.read();
  y = Wire.read();
  hotValue = shiftOr(x,y);//call function to shift/or
  x=Wire.read();
  y = Wire.read();
  deltValue = shiftOr(x,y);
  x=Wire.read();
  y = Wire.read();
  coldValue = shiftOr(x,y);
  Serial.printf("TC2\nhot junction = %i, delta = %i, cold junction = %i\n",hotValue, deltValue, coldValue);
  delay(3000);
  


} 
//define function
int16_t shiftOr(byte msb,byte lsb){
  int16_t value;
  value=msb<<8|lsb;
  return value;
}
