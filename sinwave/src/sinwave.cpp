/* 
 * Project Sinwave generator replacing analog write
 * Author: Laura Green  
 * Date: May 3, 2024
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "AD9833.h"
#include <SPI.h>
#include <SdFat.h>
#include <math.h>
#include "IoTClassroom_CNM.h"
#include <Encoder.h>
#include "Adafruit_GFX.h"//order matters for OLED h files
#include "Adafruit_SSD1306.h"
#include <Adafruit_MQTT.h>
#include "Adafruit_MQTT/Adafruit_MQTT_SPARK.h"
#include "Adafruit_MQTT/Adafruit_MQTT.h"
#include "credentials.h"//be sure to add to ignore file

SYSTEM_MODE(SEMI_AUTOMATIC);

TCPClient TheClient; 

Adafruit_MQTT_SPARK mqtt(&TheClient,AIO_SERVER,AIO_SERVERPORT,AIO_USERNAME,AIO_KEY);
Adafruit_MQTT_Publish pubFeedZDataRatio = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/ZDataRatio");
/// for sin wave generator
const int AD9833_FSYNC = D14;//will replace pulse pin for sin wave output
const int MASTER_CLOCK = 25000000;//Hz
const int MODE_SINE = 0;//0=sin,1=triangle,2=square
unsigned int frequency;//desired freq in hz
////for SD card reader
const int CS=A4;//chip select to activate SD reader
const char FILE_BASE_NAME[]="Data";//character array for base file name that will be added to
int fileNumber;//used to indicate which file written to
const uint8_t BASE_NAME_SIZE=sizeof(FILE_BASE_NAME)-1;//defined as size of base name
char fileName[13];//to contain file name, number , .csv + 1 to mark end of array
///for "impedance"
const int  PULSEREADPIN=A3;
const int PLANTREADPIN=A1;
float pulse;//make these global so they can be called in code
float plant;
float manRatio;//for function return in manual mode
int i;//counter to fill array for max ratio
///arrays
float ratReadArray[200][2];//use to determine max and ratio at 0.5 of max
float impedArray[3];//for printing to SD Card
//encoder variables
const int ENCPINA=D9;
const int ENCPINB=D10;
const int ENCSWITCH=D4;//for manual/scan mode
const int ENCGREEN=D5;
const int ENCBLUE=D6;
int dialPosition1;//encoder read first set to 0 intially when defined or could initialize in setup
int dialPosition2;//encoder read second
int manFreq;
bool onOff;//for enc button click
//OLED constants
const int OLED_RESET=-1;
const int XPOS=0; 
const int YPOS=1;
const int DELTAY=2;

const int DATAINT=15000;//data collection interval
int dataTimer;
int logTime; //for unix time
unsigned int lastTimeMeas;//for measurement interval and  publishing to Adafruit

////declare functions
float ratAPRead();//function to read pulse, plant and calculate max ratio every second
void writeSD(int logTime, unsigned int frequency, float impedArray[3]);//only impedance variables in array since they're same data type
void nextSDFile();
void MQTT_connect();//funtions to connect and maintain connection to Adafruit io
bool MQTT_ping();
void encButtonClick();//for interrupt

//declare objects
SdFat sd;
SdFile file;
Encoder myEnc(ENCPINA,ENCPINB);
Button encSwitch(ENCSWITCH,FALSE);//false for internal pull-down (not pull-up) not needed as object if in interupt function???
AD9833 sineGen(AD9833_FSYNC, MASTER_CLOCK);//sine wave generator
Adafruit_SSD1306 display(OLED_RESET);

void setup() {
  Serial.begin(9600);
  Serial.printf("Starting Serial Monitor...\n");
  delay(5000);

   //OLED initialization
display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //initialize with 12C address
void setRotation(uint8_t rotation);
display.clearDisplay();   // clears the screen and buffer
//display.display();

//initalizing sin wave generator
  frequency = 500; // initial frequency for sweep
  sineGen.reset(1);           // Place ad9833 into reset
  //Serial.printf("AD9833 in reset\n");
  sineGen.setFreq(frequency); // Set initial frequency 
  //Serial.printf("AD9833 frequency set\n");
  sineGen.setPhase(0);        // Set initial phase offset to 0
  //Serial.printf("AD9833 phase set\n");
  sineGen.setFPRegister(1);
  //Serial.printf("AD9833 FP reg set to 1\n");
  sineGen.setFreq(frequency);//not sure why this has to be set again.....
  //Serial.printf("AD9833 frequency set\n");
  sineGen.setPhase(0);
  //Serial.printf("AD9833 phase set\n");
  sineGen.setFPRegister(0);
  //Serial.printf("AD9833 FP reg set to 0\n");
  sineGen.mode(MODE_SINE);    // Set output mode to sine wave
  //Serial.printf("AD9833 mode set to sine wave\n");
  sineGen.reset(0);           // Take ad9833 out of reset
  //Serial.printf("AD9833 out of reset\n");

  //initializing SD Card Reader

  if(!sd.begin(CS,SD_SCK_MHZ(10))) {
    Serial.printf("Error starting SD Module");
  }
  if (BASE_NAME_SIZE >6){
    Serial.println("FILE_BASE_NAME too long");
    while(1);//stop program 
  }
  fileNumber=0;
  sprintf(fileName,"%s%02i.csv",FILE_BASE_NAME,fileNumber);//writes fileName for first time 

while (sd.exists(fileName)) {  //cycle through files until number not found for unwritten file
    fileNumber++;
    sprintf(fileName,"%s%02i.csv",FILE_BASE_NAME,fileNumber); //create numbered filename, (sprint prints to file that is 1st argument)
    Serial.printf("Filename is %s\n",fileName);//print to serial monitor
  }
  if (!file.open(fileName, O_WRONLY | O_CREAT | O_EXCL)) { // open file for printing
    Serial.println("File Failed to Open");
  }
  file.printf("Timestamp, Frequency_Hz, Pulse, Plant, Ratio  \n");  // print header row not sure how I want to do this yet...timestamp frequency ratio?
  Serial.printf("\nLogging to: %s \n",fileName);

  file.close();//everytime line is opened, have to close
  Serial.printf("Done \n");

 
  Particle.syncTime();//don't need time zone for unix
  //dataTimer=millis();
  //sdTimer=millis();

  pinMode(PULSEREADPIN,INPUT);
  pinMode(PLANTREADPIN, INPUT);
  pinMode(ENCGREEN, OUTPUT);//lights on encoder switch
  pinMode(ENCBLUE,OUTPUT);
  onOff=true;//for encoder switch-start true since on is off (ground completes circuit)start in manual mode
  manFreq=500;
  pinMode(ENCSWITCH, INPUT_PULLDOWN); //should it be INPUT_PULLDOWN? maybe not in right pin type for that?
  attachInterrupt(ENCSWITCH,encButtonClick,CHANGE);//interupt for when mode button is clicked
//onOff true will be manual mode
  i=0;



}

void loop() {
  
  //MQTT_connect();
 // MQTT_ping();
  

  //for manual mode/scan mode
  /*
  if(encSwitch.isClicked()) {//using button heterofile with bool function isClicked make this an interrupt funciton?
    onOff=!onOff;//assigns onoff opposite of existing (toggles)
    lastTimeMeas=millis();//initially sets measurement interval timer
  } 
  */
  //manual mode
  
if(onOff==TRUE){//try arranging these here:
digitalWrite(ENCBLUE,HIGH);
digitalWrite(ENCGREEN,LOW);//low turns on/high off, so blue in manual mode(connects to ground to complete circuit)


  
//Serial.printf("onOff=%i\n",onOff);//un-comment to check
//delay(1000);
Serial.printf("manual mode: enter frequency with dial\n");
delay(1000);
Serial.printf("%i Hz\n",manFreq);//initial freq
delay(1000);

  display.clearDisplay();
  display.setTextSize(2);//
  display.setTextColor(WHITE);
  display.setCursor(0,10);
  display.printf("MANUALMODE");
  display.display();
  delay(1000);
  display.clearDisplay();
  display.setTextSize(2);//
  display.setTextColor(WHITE);
  display.setCursor(10,10);
  display.println("USE DIAL");
  display.display();
  delay(1000);
  display.clearDisplay();
  display.setCursor(15,10);
  display.println("TO SET");
  display.display();
  delay(500);
  display.clearDisplay();
  display.setCursor(10,10);
  display.println("FREQUENCY");
  display.display();
  delay(1000);

  

  display.clearDisplay();
  display.setTextSize(2);//
  display.setTextColor(WHITE);
  display.setCursor(5,10);
  display.printf("%i Hz\n",manFreq);
  display.display();
  delay(1000);


  

  dialPosition2=myEnc.read();
  
  if(dialPosition2>95){
    myEnc.write(95);
    dialPosition2=95;
  }
  if(dialPosition2<0){
    myEnc.write(0);
    dialPosition2=0;
  }
  
   if(dialPosition2!=dialPosition1) {//only prints if dial has been turned 
    

   dialPosition1=dialPosition2;//redefine to see furthur changes
   manFreq=map(dialPosition1,0,95,100,100000);//convert from dial position to frequency
   sineGen.setFreq(manFreq);//change sin wave freq
  
Serial.printf("%i Hz\n",manFreq);//print to serial monitor
delay(1000);

display.clearDisplay();//print frequency to OLED
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(5,10);
  display.printf("%0.2f HZ",manFreq);
 display.display();
  delay(3000);
  
    }
  
  
///every minute show ratio on OLED and publish to Adafruit

//could enter measurement interval on key pad?
  if((millis()-lastTimeMeas > 10000)) {//publishing (how often?)
  Serial.printf("measuring\n");//print to serial monitor
  delay(1000);
     logTime=(int)Time.now();//unix time at reading
     manRatio=ratAPRead();//call function to measure and calculate ratio every (sec?)
  Serial.printf("%i Z magnitude is %0.2f\n",logTime,manRatio);
  
  display.clearDisplay();//print frequency to OLED
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,5);
  display.printf("measuring");
  display.display();
  delay(1000);
  
  /*
    if(mqtt.Update()) {
       pubFeedZDataRatio.publish(manRatio);//publish max ratio
      Serial.printf("Publishing %.2f at %i\n",manRatio,manFreq);
       
      } 
      */
      
     lastTimeMeas=millis();
      }
      
}
    
  

else{

digitalWrite(ENCGREEN,HIGH);
digitalWrite(ENCBLUE,LOW);
Serial.printf("Scan mode");
delay(1000);

display.clearDisplay();//print frequency to OLED
display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,5);
  display.printf("SCAN MODE");
  display.display();
  delay(1000);
  
frequency=500;
  //if scan button clicked (=scan mode) else in manual encoder to Hz and click (other button) then write data = inputted data interval
for(i=0;i<200;i++){//keep reading until array full -could just add to hz here
logTime=(int)Time.now();//unix time at reading

  impedArray[2]=ratAPRead();//call function here each iteration of this function takes 1 sec, so built in timer (100 sec for all)
  impedArray[0]=pulse;
  impedArray[1]=plant;
  writeSD(logTime,frequency,impedArray);//call SD card function
   Serial.printf("%i,%u,%0.2f,%0.2f,%0.2f\n",logTime,frequency,impedArray[0],impedArray[1],impedArray[2]); 
   display.clearDisplay();//print frequency to OLED
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,5);
  display.printf("%i,%u,%0.2f,%0.2f,%0.2f\n",logTime,frequency,impedArray[0],impedArray[1],impedArray[2]);
  display.display();
  delay(1000);
    frequency=frequency+500;//increment frequency for next loop
    sineGen.setFreq(frequency);//change frequency in sin wave generator
    
}
Serial.printf("scan complete\n");
display.clearDisplay();//print frequency to OLED
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,5);
  display.printf("SCAN COMPLETE");
  display.display();
  delay(1000);
nextSDFile();//call function to move to next file
onOff=TRUE;//return to manual mode

}
}

///////function to write to SD Card in lines///////
void writeSD(int logTime, unsigned int frequency, float impedArray[3]){//use individual universal parameters which will change for each loop
  if (!file.open(fileName, O_WRONLY | O_AT_END)) { // open file for printing and append to end
    Serial.println("File Failed to Open");
  }
  file.printf("%i,%u,%0.2f,%0.2f,%0.2f\n",logTime,frequency,impedArray[0],impedArray[1],impedArray[2]);  
  Serial.printf("\nLogging to: %s \n",fileName);

  file.close();//everytime line is opened, have to close
  Serial.printf("Done \n");

}
void nextSDFile(){//function to move to next file 
while (sd.exists(fileName)) {  //cycle through files until number not found for unwritten file
    fileNumber++;
    sprintf(fileName,"%s%02i.csv",FILE_BASE_NAME,fileNumber); //create numbered filename, (sprint prints to file that is 1st argument)
    Serial.printf("Filename is %s\n",fileName);//print to serial monitor
  }
  if (!file.open(fileName, O_WRONLY | O_CREAT | O_EXCL)) { // open file for printing
    Serial.println("File Failed to Open");
  }
  file.printf("Timestamp, Frequency_Hz, Pulse, Plant, Ratio  \n");  // print header row not sure how I want to do this yet...timestamp frequency ratio?
  Serial.printf("\nLogging to: %s \n",fileName);

  file.close();//everytime line is opened, have to close
  Serial.printf("Done \n");
}
//function to measure max plant/pulse ratio at 100 frequencies and put into array to write to sd card 
float ratAPRead() {
  const int READTIME=1000;//average over 1 sec
  unsigned int startRead;
  //int hz;
  //float pulse;
  //float plant;
  float ratio;
  float ratioMax;//max ratio at every one second read
  //float functData[3];
 
ratioMax=0;
startRead=millis();
  while((millis()-startRead)<READTIME) {//read and calculate ratio over and over for 1 sec
  
    //analogWrite(PULSEPIN,AVSIG,hz);
    pulse=analogRead(PULSEREADPIN);//pulse functData[0] these are global variables
    plant=analogRead(PLANTREADPIN);//plant functData[1]
    ratio=plant/pulse;

    if(ratio>ratioMax){
    ratioMax=ratio;//functData[2]
   

  }
  }
  return ratioMax;//could whole function be returned with 3 data points?
}

void MQTT_connect() {//actually connects to server, if not connected stuck in loop
  int8_t ret;//photon 2 thinks of integers as 32bits
 
  // Return if already connected.
  if (mqtt.connected()) {
    return;
  }
 
  Serial.print("Connecting to MQTT... ");
 
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.printf("Error Code %s\n",mqtt.connectErrorString(ret));
       Serial.printf("Retrying MQTT connection in 5 seconds...\n");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds and try again
  }
  Serial.printf("MQTT Connected!\n");
}

bool MQTT_ping() {//broker will disconnect if doesn't hear anything, just reminding broker still here so don't disconnect
  static unsigned int last;
  bool pingStatus;

  if ((millis()-last)>120000) {
      Serial.printf("Pinging MQTT \n");
      pingStatus = mqtt.ping();
      if(!pingStatus) {
        Serial.printf("Disconnecting \n");
        mqtt.disconnect();
      }
      last = millis();
  }
  return pingStatus;
}

////super short function for button press (interupt)
void encButtonClick(){  
onOff=!onOff;
lastTimeMeas=millis();//initially sets measurement interval timer
}




