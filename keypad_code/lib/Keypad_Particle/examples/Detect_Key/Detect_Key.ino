// Example usage for Keypad_Particle library by Mark Stanley, Alexander Brevig.

#include "Keypad_Particle.h"

// set up keypad buttons
const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = { D3, D2, D1, D0 };
byte colPins[COLS] = { D6, D5, D4 };

// create Keypad object variable called "keypad"
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

void setup(){
  Serial.begin(9600);
}
  
void loop(){
  char key = keypad.getKey();
  
  if (key){
    Serial.println(key);
  }
}