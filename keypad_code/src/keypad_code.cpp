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
#include "math.h"

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
char numberArray[10];//to put entry for data collection interval
const uint8_t ARRAY_SIZE=sizeof(numberArray)-1;//defined as size of base name
int placeCount;//how long number pressed is 
int i;//to count how many keys pressed
int digits;//total digits in number
int dig;//digit to add
int number; //to store function return

// create Keypad object variable called "keypad"
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

SYSTEM_MODE(SEMI_AUTOMATIC);

void setup(){
  Serial.begin(9600);
}
  
void loop(){
 
  i=0;
  while(i<10){
    key = keypad.getKey();//function
  if (key){//when asterik is pressed, entry finished # length = array position of asterik
      numberArray[i]=key;//array of characters
      Serial.printf("%c|n",numberArray[i]);
      if(numberArray[i]=="*"){
        break;
      }
      i++;
    }
    Serial.printf("Number entered is%s with %i digits\n",numberArray,sizeof(numberArray));
    digits=(sizeof(numberArray)-1);
  }
  for(j=digits;j>=0;j--){
    dig=(atoi(numberArray[j]))*(pow(10,(j-1)));//multiply each digit by power of 10
  }
number=dig;

//or 
number=strtol(numberArray,"*",10);//string to long?
}
int getNumber{
  int num=0;
  char key=keypad.getKey();
}



