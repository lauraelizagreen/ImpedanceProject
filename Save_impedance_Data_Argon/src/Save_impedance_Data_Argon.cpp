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
#include<Encoder.h>
#include<math.h>

///data types for SD Card reader
const int CS=D2;//to activate SD reader set low or SS (can be any digital pin)
//const int STARTPIN=D9;//if want button to log
const uint SDTIME=15000;//every 15 sec
const char FILE_BASE_NAME[]="Data";
int fileNumber;
const uint8_t BASE_NAME_SIZE=sizeof(FILE_BASE_NAME)-1;
char fileName[13];// why 13: data+4more char (.csv) one to mark end of array
////for AP style "impedance"
const int PULSEPIN=A2;//PWM pin
const int PULSEREADPIN=A5;
const int PLANTREADPIN=A1;
const int AVSIG=127;//average signal applied to pulse pin 127 produces equal on off (square wave?)
int i;//counter to fill array at all frequecies
int hz;//start hz at 100 (like multipip range is 5-50,000 Hz) Adrian started with 500Hz
float pulse;//these are local variables in ratAPRead function now
float plant;
//float ratio;
//float maxRatio;//output from read function going directly into array
float ratReadArray[100][2];//array will contain freq and max ratio for 100 freqs (steps of 500hz)
float impedArray[3]; //to contain pulse, plant, ratio(maxRatio)

const int DATAINT=15000;//timing of data collection-not sure what this will be yet.
int dataTimer;
 int logTime;
/*////needed if using sin wave
float sinwave;
float A;
float t;
float v;//frequency
float B;//offset
*/
////declare functions

float ratAPRead(int hz);
void writeSD(int logTime, int hz, float impedArray[3]);//just variables since not all same data type

//file system objects
SdFat sd;
SdFile file;//to put logged data into



SYSTEM_MODE(SEMI_AUTOMATIC);



// Run the application and system concurrently in separate threads
//SYSTEM_THREAD(ENABLED);




void setup(){
  Serial.begin(9600);
  waitFor(Serial.isConnected, 5000);
  delay(1000);

  //pinMode(STARTPIN,INPUT_PULLDOWN);

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

  pinMode(PULSEPIN,OUTPUT);
  pinMode(PULSEREADPIN,INPUT);
  pinMode(PLANTREADPIN, INPUT);

  hz=500;

  /*for sinwave
  A=255/2.0;//amplitude
  t=millis()/1000.0; //time in seconds
  v=500;//frequency
  B=255/2.0;//offset
  */
  i=0;
  
}


  



void loop() {

  //sinwave=A* sin(2 * M_PI * v * t)+B; //for sin wave, but won't be able to get high enough frequencies
//if scan button clicked (=scan mode) else in manual encoder to Hz and click (other button) then write data = inputted data interval
for(i=0;i<100;i++){//keep reading until array full -could just add to hz here
logTime=(int)Time.now();//unix time at reading

  impedArray[2]=ratAPRead(hz);//each iteration of this function takes 1 sec, so built in timer (100 sec for all)
  impedArray[0]=pulse;
  impedArray[1]=plant;
  writeSD(logTime,hz,impedArray);//call SD card function
    hz=hz+500;//increment frequency for next loop
}
Serial.printf("scan complete\n");

  //ratio=random(0,100);//just to stand in for real data for now
  //if ((millis()-sdTimer)>SDTIME){
    /*
 Serial.printf("Press button to log data\n"); may add button later
 logStart=digitalRead(STARTPIN);
 while(logStart==false){
  logStart=digitalRead(STARTPIN);
  delay(5);//why this delay?
  
 }
 */
/*
if((millis()-dataTimer)>DATAINT){
  ratReadArray[0]=(int)Time.now();//function to get unix time (and change array)
  dataArray[1]=random(0,10);
  dataArray[2]=random(100,1000);
  writeSD(dataArray);//call function with array as argument
  dataTimer=millis();

}
*/
}
  
  
  /*not sure if I'll want to loop through like this
  for (i=0;i<100;i++) {
    logTime = micros() - startTime;
    Serial.print("x");
    file.printf("%u , %i \n",logTime,random(0,100));  //print timestamp and random number to file
    */
  
  
  

  
///////function to write to SD Card in lines///////
void writeSD(int logTime, int hz, float impedArray[3]){//use individual universal parameters which will change for each loop
  if (!file.open(fileName, O_WRONLY | O_AT_END)) { // open file for printing and append to end
    Serial.println("File Failed to Open");
  }
  file.printf("%i,%i,%0.2f,%0.2f,%0.2f\n",logTime,hz,impedArray[0],impedArray[1],impedArray[2]);  
  Serial.printf("\nLogging to: %s \n",fileName);

  file.close();//everytime line is opened, have to close
  Serial.printf("Done \n");

}
//function to measure max plant/pulse ratio at 100 frequencies and put into array to write to sd card 
float ratAPRead(int hz) {
  const int READTIME=1000;//average over 1 sec
  unsigned int startRead;
  //int hz;
  //float pulse;
  //float plant;
  float ratio;
  float ratioMax;//max ratio at every one second read
  float functData[3];
  
ratioMax=0;
startRead=millis();
  while((millis()-startRead)<READTIME) {//read and calculate ratio over and over for 1 sec
    analogWrite(PULSEPIN,AVSIG,hz);
    pulse=analogRead(PULSEREADPIN);//pulse functData[0]
    plant=analogRead(PLANTREADPIN);//plant functData[1]
    ratio=plant/pulse;

    if(ratio>ratioMax){
    ratioMax=ratio;//functData[2]
   

  }
  }
  return ratioMax;//could whole function be returned with 3 data points?
}

