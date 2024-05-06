/* 
 * Project Keypad for impedance instrument  
 * Author: Laura Green
 * Date: May 5, 2024
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "Particle.h"
#include "Keypad_Particle.h"

const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {//character 2D array filled with numerals which are characters
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = { D3, D2, D1, D0 };
byte colPins[COLS] = { D6, D5, D4 };

numberArray[7];
int placeCount;//how long number pressed is 
int i;//to count how many keys pressed

// create Keypad object variable called "keypad"
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

SYSTEM_MODE(SEMI_AUTOMATIC);

void setup(){
  Serial.begin(9600);
}
  
void loop(){
  char key = keypad.getKey();//function
  i=0;
  while(i<7){
  if (key){//when asterik is pressed, entry finished # length = array position of asterik
    Serial.println(key);
    if(key!=[3][0]){
      numberArray[key[][]]
      
    }
  }
}
}


