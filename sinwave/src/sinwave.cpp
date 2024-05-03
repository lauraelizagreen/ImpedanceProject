/* 
 * Project Sinwave generator replacing analog write
 * Author: Laura Green  
 * Date: May 3, 2024
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "Particle.h"
#include "AD9833.h"
#include <SPI.h>
#include <SdFat.h>
#include <math.h>

SYSTEM_MODE(SEMI_AUTOMATIC);
/// for sin wave generator
const int AD9833_FSYNC = D14;//will replace pulse pin for sin wave output
const int MASTER_CLOCK = 25000000;//Hz
const int MODE_SINE = 0;//0=sin,1=triangle,2=square
unsigned long frequency;//desired freq in hz
////for SD card reader
const int CS=D6;//chip select to activate SD reader
const char FILE_BASE_NAME[]="Data";//character array for base file name that will be added to
int fileNumber;//used to indicate which file written to
const uint8_t BASE_NAME_SIZE=sizeof(FILE_BASE_NAME)-1;//defined as size of base name
char fileName[13];//to contain file name, number , .csv + 1 to mark end of array
///for "impedance"
const int  PULSEREADPIN=A3;
const int PLANTREADPIN=A1;
float pulse;
float plant;
int i;//counter to fill array for max ratio
///arrays
float ratReadArray[100][2];
float impedArray[3];

const int DATAINT=15000;//data collection interval
int dataTimer;
int logTime; //for unix time

////declare functions
float ratAPRead(unsigned long frequency);
void writeSD(int logTime, unsigned long frequency, float impedArray[3]);//only impedance variables in array since they're same data type

//declare objects
SdFat sd;
SdFile file;

AD9833 sineGen(AD9833_FSYNC, MASTER_CLOCK);

void setup() {
  Serial.begin(9600);
  Serial.printf("Starting Serial Monitor...\n");
  delay(5000);
//initalizing sin wave generator
  frequency = 1000; // initial frequency 1K ohms for now

  sineGen.reset(1);           // Place ad9833 into reset
  Serial.printf("AD9833 in reset\n");
  sineGen.setFreq(frequency); // Set initial frequency 
  Serial.printf("AD9833 frequency set\n");
  sineGen.setPhase(0);        // Set initial phase offset to 0
  Serial.printf("AD9833 phase set\n");
  sineGen.setFPRegister(1);
  Serial.printf("AD9833 FP reg set to 1\n");
  sineGen.setFreq(frequency);
  Serial.printf("AD9833 frequency set\n");
  sineGen.setPhase(0);
  Serial.printf("AD9833 phase set\n");
  sineGen.setFPRegister(0);
  Serial.printf("AD9833 FP reg set to 0\n");
  sineGen.mode(MODE_SINE);    // Set output mode to sine wave
  Serial.printf("AD9833 mode set to sine wave\n");
  sineGen.reset(0);           // Take ad9833 out of reset
  Serial.printf("AD9833 out of reset\n");

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

  i=0;



}

void loop() {
  //if scan button clicked (=scan mode) else in manual encoder to Hz and click (other button) then write data = inputted data interval
for(i=0;i<100;i++){//keep reading until array full -could just add to hz here
logTime=(int)Time.now();//unix time at reading

  impedArray[2]=ratAPRead(frequency);//call function here each iteration of this function takes 1 sec, so built in timer (100 sec for all)
  impedArray[0]=pulse;
  impedArray[1]=plant;
  writeSD(logTime,frequency,impedArray);//call SD card function
    frequency=frequency+500;//increment frequency for next loop
    sineGen.setFreq(frequency);//change frequency in sin wave generator
}
Serial.printf("scan complete\n");

}

///////function to write to SD Card in lines///////
void writeSD(int logTime, unsigned long frequency, float impedArray[3]){//use individual universal parameters which will change for each loop
  if (!file.open(fileName, O_WRONLY | O_AT_END)) { // open file for printing and append to end
    Serial.println("File Failed to Open");
  }
  file.printf("%i,%u,%0.2f,%0.2f,%0.2f\n",logTime,frequency,impedArray[0],impedArray[1],impedArray[2]);  
  Serial.printf("\nLogging to: %s \n",fileName);

  file.close();//everytime line is opened, have to close
  Serial.printf("Done \n");

}
//function to measure max plant/pulse ratio at 100 frequencies and put into array to write to sd card 
float ratAPRead(unsigned long hz) {
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


