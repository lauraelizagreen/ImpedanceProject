# Keypad_Particle

A Particle library for Keypad_Particle

Used to read keypads such as [12-button keypad from SparkFun](https://www.sparkfun.com/products/8653)

## Usage

Connect keypad hardware, add the Keypad_Particle library to your project, and follow this simple example:

```
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
Keypad_Particle keypad = Keypad_Particle( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

void setup(){
  Serial.begin(9600);
}
  
void loop(){
  char key = keypad.getKey();
  
  if (key){
    Serial.println(key);
  }
}
```

See the [examples](examples) folder for more details.

## LICENSE
Copyright 2015 Mark Stanley, Alexander Brevig

Licensed under the GNU General Public License
