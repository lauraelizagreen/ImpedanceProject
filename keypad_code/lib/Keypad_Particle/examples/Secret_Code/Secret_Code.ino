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

// change to any secret code or command of any length
const int codeLength = 4;
char secretCode[codeLength] = {'8','3','2','4'}; // 8324 = TECH

int correctCount = 0;
boolean correctCode = false;

void setup(){
  Serial.begin(9600);
}
  
void loop(){

    checkCode();

    if (correctCode) {

        // add instructions to do something if secret code entered correctly
		Serial.println("Correct code entered")
    }
}

void checkCode() {
    char key = keypad.getKey();
    // if key pressed, then check which key
    if (key) {
        // optional: play sound as feedback when key pressed
        // tone(speakerPin, 3000, 25);

        // if key matches next key in secret code, add one to correct count
        if (key == secretCode[correctCount]) {
            correctCount++; 
        } else {
            // else wrong key was entered (reset correct count)
            correctCount = 0; 
        }
    }
    // check to see if complete secret code entered correctly
    if (correctCount == codeLength) {
        correctCode = true;
    }
    // need slight delay between key readings
    delay(100);
}