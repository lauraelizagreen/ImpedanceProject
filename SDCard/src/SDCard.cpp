/* 
 * Project SDCard
 * Author: Laura Green
 * Date: April 26, 2024
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include <SPI.h>
#include <SdFat.h>

const int CS=A5;//chip select (can be any digital pin)
const int DATAINT=15000;//interval btw data collection
const char FILE_BASE_NAME[]="Data";
const uint8_t BASE_NAME_SIZE=sizeof(FILE_BASE_NAME)-1;
char fileName[13];
int fileNumber;
int dataArray[3];//one-dimensional array to put each data line

unsigned int dataTimer;//timer to collect data

//declare SD objects
SdFat sd;
SdFile file;

void writeSD(int dataArray[3]);//declare function to write each data line to SD card as it's generated

// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(SEMI_AUTOMATIC);


void setup() {
  Serial.begin(9600);
  waitFor(Serial.isConnected,5000);
  delay(1000);

  if (!sd.begin(CS, SD_SCK_MHZ(10))) {
    Serial.printf("Error starting SD Module"); 
  }

  if(BASE_NAME_SIZE>6){
    Serial.println("FILE_BASE_NAME too long");
    while(1);//stop program
  }
  fileNumber=0;
  sprintf(fileName,"%s%02i.csv",FILE_BASE_NAME,fileNumber);//first file with number 0
   
  while (sd.exists(fileName)) {  //cycle through files until number not found for unwritten file
    fileNumber++;
    sprintf(fileName,"%s%02i.csv",FILE_BASE_NAME,fileNumber); //create numbered filename, (sprint prints to file that is 1st argument)
    Serial.printf("Filename is %s\n",fileName);//print to serial monitor
  }
  if (!file.open(fileName, O_WRONLY | O_CREAT | O_EXCL)) { // open file for printing
    Serial.println("File Failed to Open");
  }
  file.printf("TimeStamp, data 1, data 2 \n");  // print header row
  Serial.printf("\nLogging to: %s \n",fileName);

  file.close();//everytime line is opened, have to close
  Serial.printf("Done \n");

  Particle.syncTime();
  dataTimer=millis();
  
}


void loop() {
  if((millis()-dataTimer)>DATAINT){
  dataArray[0]=(int)Time.now();//function to get unix time
  dataArray[1]=random(0,10);
  dataArray[2]=random(100,1000);
  writeSD(dataArray);//call function with array as argument
  dataTimer=millis();

}
}
//////define function to write to SD card
void writeSD(int dataArray[3]){
  /*
  while (sd.exists(fileName)) {  //cycle through files until number not found
    fileNumber++;
    sprintf(fileName,"%s%02i.csv",FILE_BASE_NAME,fileNumber); //create numbered filename, (sprint prints to file that is 1st argument)
    Serial.printf("Filename is %s\n",fileName);//print to serial monitor
  }
  */
  
  
  if (!file.open(fileName, O_WRONLY | O_AT_END)) { // open file for printing and append to end
    Serial.println("File Failed to Open");
  }
  file.printf("%i,%i,%i\n",dataArray[0],dataArray[1],dataArray[2]);  // print header row
  Serial.printf("\nLogging to: %s \n",fileName);

  file.close();//everytime line is opened, have to close
  Serial.printf("Done \n");

}
