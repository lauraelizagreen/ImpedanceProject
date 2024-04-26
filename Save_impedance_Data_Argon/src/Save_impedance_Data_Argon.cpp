/* 
 * Project Save_impedance_Data_Argon
 * Author: Laura Green
 * Date: April 14, 2024
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
//#include "Particle.h"
#include <SPI.h>
#include <SdFat.h>

const int chipSelect=SS;//to activate SD reader?
const int STARTPIN=D9;
const uint SDTIME=5000;//every 5 sec
const char FILE_BASE_NAME[]="Data";
int fileNumber;
//file system objects
SdFat sd;
SdFile file;//to put logged data into
//timer for data record
unsigned int logTime;//where does logTime get defined?
unsigned int startTime;
unsigned int sdTimer;//when to write to SD card
int i;
float ratio;
bool logStart;//for button press

//const int STARTPIN=D9;//???is this for button?

const uint8_t BASE_NAME_SIZE=sizeof(FILE_BASE_NAME)-1;
char fileName[13];// why 13? data+4more char (.csv) one to mark end of array

SYSTEM_MODE(SEMI_AUTOMATIC);



// Run the application and system concurrently in separate threads
//SYSTEM_THREAD(ENABLED);




void setup(){
  Serial.begin(9600);
  waitFor(Serial.isConnected, 5000);
  delay(100);
  Serial.printf("Starting Up...");

  pinMode(STARTPIN,INPUT_PULLDOWN);

  if(!sd.begin(chipSelect,SD_SCK_MHZ(10))) {
    Serial.printf("Error starting SD Module");
  }
  if (BASE_NAME_SIZE >6){
    Serial.println("FILE_BASE_NAME too long");
    while(1);//stop program chip select (SS)??? 1 is off
  }
  fileNumber=0;
  sprintf(fileName,"%s%02i.csv",FILE_BASE_NAME,fileNumber);//not sure where this goes
  //sdTimer=millis();
}


void loop() {
  //ratio=random(0,100);//just to stand in for real data for now
  //if ((millis()-sdTimer)>SDTIME){
 Serial.printf("Press button to log data\n");
 logStart=digitalRead(STARTPIN);
 while(logStart==false){
  logStart=digitalRead(STARTPIN);
  delay(5);//why this delay?
 }

  Serial.printf("Starting Data Logging\n");
  while (sd.exists(fileName)){//????//if that file has already been created
    fileNumber++;
    sprintf(fileName, "%s02i.csv",FILE_BASE_NAME,fileNumber);//writes to sd card? string print "file name" is first argument where it's put
    Serial.printf("Filename is %s\n",fileName);//writes to serial monitor
  }
  if (!file.open(fileName,O_WRONLY | O_CREAT | O_EXCL)) {//writeread,create,exclusive (doesn't already exist)
    Serial.println("File Failed to Open");
  }
  file.printf("TimeStamp, ratio \n"); //just like serial print only to sd card? // print header row
  Serial.printf("\nLogging to: %s \n",fileName);
  startTime = micros();
  for (i=0;i<100;i++) {
    logTime = micros() - startTime;
    Serial.print("x");
    file.printf("%u , %i \n",logTime,random(0,100));  //print timestamp and random number to file
    delay(random(100,500));           //delays are bad, this delay is for demo purposes only
  }
  
  file.close();
  Serial.printf("Done \n");
  delay(5000);//every 5 sec

  
}

