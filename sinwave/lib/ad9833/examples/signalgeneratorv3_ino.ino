/*---------------------------------------------------------------------
 * AD9833 Function Generator
 * By John Bradnam
 * based on: 
 *  DIY Function/Waveform Generator by GreatScottLab
 *  (https://www.instructables.com/DIY-FunctionWaveform-Generator/)
 *  
 *  230114 - Rewrote code
 */
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include "AD9833.h"
#include "Rotary.h"

//Pins
#define ROTARY_A 3
#define ROTARY_B 2
#define SWITCH 4
#define LCD_D7 5
#define LCD_D6 6
#define LCD_D5 7
#define LCD_D4 8
#define LCD_EN 9
#define LCD_RS A0
#define AD9833_FSYNC 10
#define AD9833_SCLK 13
#define AD9833_SDATA 11

//Modules
AD9833 sigGen(AD9833_FSYNC, 24000000);// Initialise our AD9833 with FSYNC and a master clock frequency of 24MHz
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
Rotary encoder(ROTARY_A, ROTARY_B);	// Initialise the encoder on pins 2 and 3 (interrupt pins)

enum MenuEnum { X1, X10, X100, X1K, X10K, X100K, X1M, WAVEFORM };
enum WaveEnum { SINE, TRANGLE, SQUARE };
#define MENU_TEXT_LEN 8
const String menuText[] = {"x1Hz", "x10Hz", "x100Hz","x1kHz", "x10kHz", "x100kHz", "x1MHz", "Waveform"};
const String waveText[] = {"SIN", "TRI", "SQR"};

//EEPROM handling
//Uncomment next line to clear out EEPROM and reset
//#define RESET_EEPROM
#define EEPROM_ADDRESS 0
#define EEPROM_MAGIC 0x0BAD0DAD
typedef struct {
  uint32_t magic;
  MenuEnum menuState;
  WaveEnum waveState;
  long frequency;
} EEPROM_DATA;

EEPROM_DATA EepromData;      //Current EEPROM settings

#define EEPROM_UPDATE_TIME 60000  //Check for eprom update every minute
unsigned long eepromTimeout = 0;
bool eepromUpdate = false;

#define FREQ_MAX 14000000

volatile bool updateDisplay = false;

//--------------------------------------------------------------------
//Setup hardware
void setup() 
{
  //Serial.begin(57600);
  //Serial.println("Starting...");

  //Get last settings
  readEepromData();
  
  // Initialise the LCD, start the backlight and print a "bootup" message for two seconds
  lcd.begin(16, 2);
  lcd.print("     AD9833     ");
  lcd.setCursor(0, 1);
  lcd.print("Signal Generator");
  delay(2000);

  // Display initial set values
  lcd.clear();
  displayFrequency();
  displayMenu();
  displayWaveform();

  // Initialise the AD9833 with 1KHz sine output, no phase shift for both
  // registers and remain on the FREQ0 register
  // sigGen.lcdDebugInit(&lcd);
  sigGen.reset(1);
  sigGen.setFreq(EepromData.frequency);
  sigGen.setPhase(0);
  sigGen.setFPRegister(1);
  sigGen.setFreq(EepromData.frequency);
  sigGen.setPhase(0);
  sigGen.setFPRegister(0);
  sigGen.mode((int)EepromData.waveState);
  sigGen.reset(0);

  // Set pins A and B from encoder as interrupts
  attachInterrupt(digitalPinToInterrupt(ROTARY_A), encChange, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ROTARY_B), encChange, CHANGE);
  // Initialise pin as input with pull-up enabled and debounce variable for
  // encoder button
  pinMode(SWITCH, INPUT_PULLUP);
  // Set Cursor to initial possition
  lcd.setCursor(0, 1);

  eepromTimeout = millis() + EEPROM_UPDATE_TIME;
}

//--------------------------------------------------------------------
//Main loop
void loop() 
{
  // If button is pressed, change the menu
  if (testButton(true)) 
  {
    EepromData.menuState = (EepromData.menuState == WAVEFORM) ? X1 : (MenuEnum)((int)EepromData.menuState + 1);
    eepromUpdate = true;
    updateDisplay = true;
  }
    
  // Update display if needed
  if (updateDisplay) 
  {
    displayFrequency();
    displayMenu();
    displayWaveform();
    updateDisplay = false;
  }

  //Update EEPROM if settings changed
  if (millis() > eepromTimeout)
  {
    if (eepromUpdate)
    {
      writeEepromData();
      eepromUpdate = false;
    }
    eepromTimeout = millis() + EEPROM_UPDATE_TIME;
  }
}

//--------------------------------------------------------------------
// Test if button has been pressed
//  waitForRelease - True to wait until button gos up
//  Returns true if button is pressed
bool testButton(bool waitForRelease) 
{
  bool pressed = false;
  if (digitalRead(SWITCH) == LOW) 
  {
    pressed = true;
    while (waitForRelease && digitalRead(SWITCH) == LOW)
    {
      yield();
    }
  }
  return pressed;
}

//--------------------------------------------------------------------
// Encoder has been rotated
//  Change frequency or waveform based on menu selection
void encChange() 
{
  unsigned char state = encoder.process();
  if (state != DIR_NONE) 
  {
    switch (EepromData.menuState) 
    {
      case X1: updateFrequency(state,1); break;
      case X10: updateFrequency(state,10); break;
      case X100: updateFrequency(state,100); break;
      case X1K: updateFrequency(state,1000); break;
      case X10K: updateFrequency(state,10000); break;
      case X100K: updateFrequency(state,100000); break;
      case X1M: updateFrequency(state,1000000); break;
      case WAVEFORM: updateWaveform(state); break;
    }
  }
}

//--------------------------------------------------------------------
//Change the current frequency based on menu and stepValue
//  state - Either DIR_CW or DIR_CCW
//  stepValue - Current amount to change frequency by
void updateFrequency(unsigned char state, long stepValue)
{
  bool update = false;
  long old = EepromData.frequency;
  if (state == DIR_CW)
  {
    EepromData.frequency = min(EepromData.frequency + stepValue,FREQ_MAX);
  }
  else
  {
    EepromData.frequency = max(EepromData.frequency - stepValue,0);
  }
  if (old != EepromData.frequency)
  {
    sigGen.setFreq(EepromData.frequency);
    eepromUpdate = true;
    updateDisplay = true;
  }
}

//--------------------------------------------------------------------
//Change the current waveform
//  state - Either DIR_CW or DIR_CCW
void updateWaveform(unsigned char state)
{
  if (state == DIR_CW)
  {
    EepromData.waveState = (EepromData.waveState == SQUARE) ? SINE : (WaveEnum)((int)EepromData.waveState + 1);
  }
  else
  {
    EepromData.waveState = (EepromData.waveState == SINE) ? SQUARE : (WaveEnum)((int)EepromData.waveState - 1);
  }
  sigGen.mode((int)EepromData.waveState);
  eepromUpdate = true;
  updateDisplay = true;
}

//--------------------------------------------------------------------
//Display current frequency
void displayFrequency() 
{
  lcd.setCursor(0, 0);
  lcd.print(formatNumber(EepromData.frequency,"F=","Hz",16));
}

//--------------------------------------------------------------------
//Display active menu state
void displayMenu()
{
  lcd.setCursor(0, 1);
  lcd.print(padString(menuText[(int)EepromData.menuState],MENU_TEXT_LEN));
}

//--------------------------------------------------------------------
//Display active waveform
void displayWaveform()
{
  lcd.setCursor(13, 1);
  lcd.print(waveText[(int)EepromData.waveState]);
}

//-----------------------------------------------------------------------------------
//Display a number with comma seperators
// number - number to format
// prefix - String to prefix number with
// postfix - String to append to number
// pad - Add spaces to the right pad string
// returns String with formatted number
String formatNumber(long number, String prefix, String postfix, int pad)
{
  String s = "";
  bool space = true;
  for (uint8_t i = 0; i < 8; i++)
  {
    if ((i==3 || i==6) && !space && number > 0)
    {
      s = String(',') + s;
    }
    if (number > 0 || i == 0)
    {
      s = String((char)((number % 10) + 48)) + s;
      space = false;
    }
    else
    {
      space = true;
    }
    number = number / 10;
  }
  s = prefix + s + postfix;
  return padString(s, pad);
}

//-----------------------------------------------------------------------------------
//Pad string with spaces
// s - string to pad
// prefix - String to prefix number with
// returns padded String
String padString(String s, int pad)
{
  String sOut = String(s);
  int len = pad - s.length();
  for (uint8_t i = 0; i < len; i++)
  {
    sOut += ' ';
  }
  return sOut;
}

//--------------------------------------------------------------------
//Write the EepromData structure to EEPROM
void writeEepromData(void)
{
  //This function uses EEPROM.update() to perform the write, so does not rewrites the value if it didn't change.
  EEPROM.put(EEPROM_ADDRESS,EepromData);
}

//--------------------------------------------------------------------
//Read the EepromData structure from EEPROM, initialise if necessary
void readEepromData(void)
{
#ifndef RESET_EEPROM
  EEPROM.get(EEPROM_ADDRESS,EepromData);
  if (EepromData.magic != EEPROM_MAGIC)
  {
#endif  
    EepromData.magic = EEPROM_MAGIC;
    EepromData.menuState = WAVEFORM;
    EepromData.waveState = SINE;
    EepromData.frequency = 1000;
    writeEepromData();
#ifndef RESET_EEPROM
  }
#endif  
}
