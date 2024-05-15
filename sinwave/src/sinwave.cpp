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

SYSTEM_MODE(AUTOMATIC);//to manually set credentials

TCPClient TheClient; 

Adafruit_MQTT_SPARK mqtt(&TheClient,AIO_SERVER,AIO_SERVERPORT,AIO_USERNAME,AIO_KEY);
Adafruit_MQTT_Publish pubFeedZHighRatio = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/highRatio");
Adafruit_MQTT_Publish pubFeedZLowRatio = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/lowRatio");
Adafruit_MQTT_Publish pubFeedZCornerFreq = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/cornerFreq");
Adafruit_MQTT_Publish pubFeedZCornerRatio = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/cornerRatio");
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
float maxSweep;//for highest ratio in each sweep to calculate corner
float cornerFrequency;
float cornerFreqRatio;//closest 0.5*max ratio
float manRatio;//for function return in manual mode
float ratioLow;//ratios at highest and lowest freq
float ratioHigh;
int i;//counter to fill array for max ratio
///arrays
float ratReadArray[200][2];//use to determine max and ratio at 0.5 of max and frequency
float impedArray[3];//for printing to SD Card
float cornerArray[200][2];//2 dimensional array to hold ratio and corresponding freq for use in cornerfreq function
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
float ratioRead();//function to read pulse, plant and calculate max ratio every second
float cornerFreq(float cornerArray[200][2]);//to calculate at what frequency ratio =.5
float cornerRatio(float cornerArray[200][2]);//to return that ratio
void writeSD(int logTime, unsigned int frequency, float impedArray[3]);//only impedance variables in array since they're same data type
void nextSDFile();
void MQTT_connect();//funtions to connect and maintain connection to Adafruit io
bool MQTT_ping();
void encButtonClick();//for interrupt

//declare objects
SdFat sd;
SdFile file;
Encoder myEnc(ENCPINA,ENCPINB);
//Button encSwitch(ENCSWITCH,FALSE);//false for internal pull-down (not pull-up) not needed as object if in interupt function???
AD9833 sineGen(AD9833_FSYNC, MASTER_CLOCK);//sine wave generator
Adafruit_SSD1306 display(OLED_RESET);

void setup() {
  Serial.begin(9600);
  Serial.printf("Starting Serial Monitor...\n");
  delay(5000);
/*
   WiFi.on();//to manually set credentials
  WiFi.clearCredentials();
  WiFi.setCredentials("IoTNetwork");

  WiFi.connect();
  while(WiFi.connecting()){
    Serial.printf(".");
  }
  Serial.printf("\n\n");
  */
/*
  WiFi.on();
  WiFi.connect();
  while(WiFi.connecting()) {
    Serial.printf(".");
  }
  Serial.printf("\n\n");
*/
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
//Serial.printf("manual mode: enter frequency with dial\n");
//delay(1000);
//Serial.printf("%i Hz\n",manFreq);//initial freq
//delay(1000);

  display.clearDisplay();
  display.setTextSize(2);//
  display.setTextColor(WHITE);
  display.setCursor(0,10);
  display.printf("MANUALMODE");
  display.display();//this is the part that takes time to light/not light each pixel
  delay(1000);
  display.clearDisplay();
  display.setTextSize(2);//
  display.setTextColor(WHITE);
  display.setCursor(15,10);
  display.println("USE DIAL");
  display.display();
  delay(1000);
  display.clearDisplay();
  display.setCursor(22,10);
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


  

  dialPosition2=-(myEnc.read());
  
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
   manFreq=map(dialPosition1,0,95,2000,48000);//convert from dial position to frequency-can this move in increments of 50?
   sineGen.setFreq(manFreq);//change sin wave freq
  
//Serial.printf("%i Hz\n",manFreq);//print to serial monitor
//delay(1000);

display.clearDisplay();//print frequency to OLED
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(5,10);
  display.printf("%i Hz",manFreq);
 display.display();
  delay(3000);
  
    }
  
  
///every minute show ratio on OLED and publish to Adafruit

//could enter measurement interval on key pad?
  if((millis()-lastTimeMeas > 10000)) {//publishing (how often?)
  //Serial.printf("measuring\n");//print to serial monitor
  //delay(1000);
     logTime=(int)Time.now();//unix time at reading
     manRatio=ratAPRead();//call function to measure and calculate ratio every (sec?)
  //Serial.printf("%i Z magnitude is %0.2f\n",logTime,manRatio);
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,5);
  display.printf("measuring");
  display.display();
  delay(1000);
      
      
     lastTimeMeas=millis();
      }
      
}
    
  

else{

digitalWrite(ENCGREEN,HIGH);
digitalWrite(ENCBLUE,LOW);
//Serial.printf("Scan mode");
//delay(1000);

display.clearDisplay();//print frequency to OLED
display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,5);
  display.printf("SCAN MODE"); 
  display.display();
  delay(1000);
 display.clearDisplay();
 display.setCursor(0,5);
  display.printf("logging to:");
  display.display();
  delay(500);
  display.clearDisplay();
  display.printf("%s",fileName);
  display.display();
  delay(1000);
  
frequency=500;
maxSweep=0;//global variable initialized for each loop
  //if scan button clicked (=scan mode) else in manual encoder to Hz and click (other button) then write data = inputted data interval
for(i=0;i<200;i++){//keep reading until array full -could just add to hz here(200 could be constant that could change with increment)
logTime=(int)Time.now();//unix time at reading
  impedArray[2]=ratioRead();//call function here each iteration of this function takes 1 sec, so built in timer (100 sec for all)
  impedArray[0]=pulse;
  impedArray[1]=plant;
  writeSD(logTime,frequency,impedArray);//call SD card function
   //Serial.printf("%i,%u,%0.2f,%0.2f,%0.2f\n",logTime,frequency,impedArray[0],impedArray[1],impedArray[2]); 
   display.clearDisplay();//print frequency to OLED
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,5);
  display.printf(" %u Hz=",frequency);
  display.display();
  delay(500);
  display.clearDisplay();
  display.setCursor(0,5);
  display.printf("%0.2f",impedArray[2]);
  display.display();
  delay(500);
  /////Resistance (real component of Z)=R0*((1/max Ratio)-1) where R0=resistance of voltage divider (known resistance going into circuit)
  if (impedArray[2]>maxSweep){//global maxSweep for highest ratio found here
    maxSweep=impedArray[2];
    Serial.printf("sweep max=%0.2f",maxSweep);//max ratio of whole scan where resistance is greatest component (for real part of impedance calc)
  }
  cornerArray[i][1]=impedArray[2];//fill array with frequency and ratios to calculate which at which freq ratio is closest to 0.5
  cornerArray[i][0]=(frequency/1.0);//to convert int to float so it can be in array
  frequency=frequency+500;//increment frequency for next loop
    sineGen.setFreq(frequency);//change frequency in sin wave generator
    
}
ratioLow=cornerArray[0][1];//ratio at lowest and highest freq measured (to publish)
ratioHigh=cornerArray[199][1];
////Reactance(imaginary component of Z)=2pi/corner freq?
////so |Z|= square root(Reactance^2+Resistance^2)
////at corner frequency could calculate Z (with imaginary component) since phase angle also known????
cornerFrequency=cornerFreq(cornerArray);//call functions to find (frequency at) ratio closest to max *.5 (where phase angle = 45 degrees)
cornerFreqRatio=cornerRatio(cornerArray);
Serial.printf("scan complete\n");
delay(1000);
Serial.printf("ratioLow=%0.2f\nratioHigh=%0.2f\ncornerFreq=%0.2f\ncornerRatio=%0.2f\n",ratioLow,ratioHigh,cornerFrequency,cornerFreqRatio);
MQTT_connect();
MQTT_ping();
//publish to Adafruit every scan
//if((millis()-lastTimeMeas > 10000)) {//publishing (how often?)just for every sweep now
 if(mqtt.Update()) {//these were published but very delayed....
       pubFeedZHighRatio.publish(ratioHigh);// ratio at highest Freq
      pubFeedZLowRatio.publish(ratioLow);//publish ratio at lowest freq
      pubFeedZCornerFreq.publish(cornerFrequency);//publish corner freq
      pubFeedZCornerRatio.publish(cornerFreqRatio);//publish corner ratio
      Serial.printf("Publishing to adafruit\n");
        
     }
        //lastTimeMeas=millis();

      
 
Serial.printf("corner frequency=%0.02f\n",cornerFrequency);
delay(1000);
display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10,5);
  display.printf("SCAN"); 
  display.display();
  delay(500);
  display.clearDisplay();
  display.setCursor(0,5);
  display.printf("COMPLETE");
  display.display();
  delay(500);
  display.clearDisplay();
  display.setCursor(0,5);
  display.printf("Corner freq=");//what should this be called?
  display.display();
  delay(1000);
  display.clearDisplay();
display.printf("%0.2f",cornerFrequency);
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
float ratioRead() {
  const int READTIME=1000;//average over 1 sec
  unsigned int startRead;
  float plantMax;
  float plantMin;
  float pulseMax;
  float pulseMin;
  //int hz;
  //float pulse;
  //float plant;
  float ratio;
  //float ratioMax;//max ratio at every one second read
  //float functData[3];
 
plantMax=0;
plantMin=4095;
pulseMax=0;
pulseMin=4095;

startRead=millis();
  while((millis()-startRead)<READTIME) {//read and calculate ratio over and over for 1 sec
  
    //analogWrite(PULSEPIN,AVSIG,hz);
    pulse=analogRead(PULSEREADPIN);//pulse functData[0] these are global variables
    plant=analogRead(PLANTREADPIN);//plant functData[1]
    //ratio=plant/pulse;
    if(plant>plantMax){
      plantMax=plant;
      Serial.printf("plantMax=%0.2f\n",plantMax);//print these to check why these seem off.
    }
    if(plant<plantMin){
      plantMin=plant;
      Serial.printf("plantMin=%0.2f\n",plantMin);
    }
    if(pulse>pulseMax){
      pulseMax=pulse;
      Serial.printf("pulseMax=%0.2f\n",pulseMax);
    }
    if(pulse<pulseMin){
      pulseMin=pulse;
      Serial.printf("pulseMin=%0.2f\n",pulseMin);
    }
  }
ratio=(plantMax-plantMin)/(pulseMax-pulseMin);
return ratio;

/*
    if(ratio>ratioMax){
    ratioMax=ratio;//functData[2]//could make pointer to return 2 values?
   */

  
  
  //return ratioMax;//could whole function be returned with 3 data points?
}

///function to calculate shoulder of magnitude curve (ratio = 0.5) here increments of 500 Hz, maybe will want smaller?
float cornerFreq(float cornerArray[200][2]){//will be built as code loops through ratAPReads, so called after that loop
  float halfRatio;
  float halfRatioFreq;
  halfRatio=cornerArray[0][1];//initialize with first ratio value
  for(i=0;i<200;i++){//loop through all ratio values
  if ((abs(cornerArray[i][1]-(0.5*maxSweep)))<(abs(halfRatio-(0.5*maxSweep)))){//current ratio difference compared to value assigned to half ratio.
   halfRatio=cornerArray[i][1];//reassign halfRatio to new closer value
   Serial.printf("halfRatio=%0.2f\n",halfRatio);//for check 
   halfRatioFreq=cornerArray[i][0];//reassign frequency at i that contains ratio closest to 0.5 
   Serial.printf("halfRatioFreq=%0.2f\n",halfRatioFreq);//for check
  }
  }
return halfRatioFreq;
//return halfRatio; //to check if calculated correctly


}

//not sure how to return 2 variables so making separate function for corner ratio
float cornerRatio(float cornerArray[200][2]){//will be built as code loops through ratAPReads, so called after that loop
  float halfRatio;
  float halfRatioFreq;
  halfRatio=cornerArray[0][1];//initialize with first ratio value
  for(i=0;i<200;i++){//loop through all ratio values
  if ((abs(cornerArray[i][1]-(0.5*maxSweep)))<(abs(halfRatio-(0.5*maxSweep)))){//current ratio difference compared to value assigned to half ratio.
   halfRatio=cornerArray[i][1];//reassign halfRatio to new closer value
   Serial.printf("halfRatio=%0.2f\n",halfRatio);//for check 
   halfRatioFreq=cornerArray[i][0];//reassign frequency at i that contains ratio closest to 0.5 
   Serial.printf("halfRatioFreq=%0.2f\n",halfRatioFreq);//for check
  }
  }
return halfRatio;
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




