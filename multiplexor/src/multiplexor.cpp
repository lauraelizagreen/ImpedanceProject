/* 
 * Project myProject
 * Author: Your Name
 * Date: 
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "Particle.h"

// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(AUTOMATIC);

// Run the application and system concurrently in separate threads
//SYSTEM_THREAD(ENABLED);

const int MULTIADDR=0x70;
uint8_t addr;
uint8_t t;
void tcaselect(int i);//declares function

void setup() {
  Serial.begin(9600);
  waitFor(Serial.isConnected, 5000);
  delay(1000);

  Wire.begin();
  Serial.begin(115200);//??
  Serial.printf("TCAScanner ready!\n");

  for(t=0;t<8;t++) {
    tcaselect(t);
    Serial.printf("TCA Port #%i\n",t);

    for(addr=0;addr<=127;addr++){
      if(addr==MULTIADDR) continue;

      Wire.beginTransmission(addr);
      if(!Wire.endTransmission()){
        Serial.printf("found 12C 0x %x\n",addr);
      }

  
    }
  }
  Serial.printf("done");
  
}


void loop() {
  
}
void tcaselect(int i){//defines function
  if(i>7){
    return;
  Wire.beginTransmission(MULTIADDR);
  Wire.write(1<<i);
  Wire.endTransmission();
  }
}
