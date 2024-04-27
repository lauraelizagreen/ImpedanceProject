/* 
 * Project BME_LoRa_communication
 * Author: Laura Green
 * Date: April 26, 2024
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include <SPI.h>
#include <SdFat.h>
#include "Adafruit_BME280.h"

const int BMEADDRESS1=0x76;
const int BMEDELAY=15000;//btw bme readings
float tempC;
float tempF;
float pressinHg;
float pressPA;
float humidRH;
float BMEArray[4];//timestamp, temp, humidity, pressure

const int CS=A5;//to activate SD reader set low or SS (can be any digital pin)
const uint SDTIME=15000;//every 15 sec
const char FILE_BASE_NAME[]="Data";
int fileNumber;
const uint8_t BASE_NAME_SIZE=sizeof(FILE_BASE_NAME)-1;
char fileName[13];

int dataArray[2];//4 eventually: timestamp and 3 bmes

const int DATAINT=15000;//interval between data collection
int BMEDataTimer;//when to collect
///declare functions
void writeSD(float ratReadArray[100][2]);


//file system objects
SdFat sd;
SdFile file;
Adafruit_BME280 bme;//define BME object 
// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(SEMI_AUTOMATIC);

// Run the application and system concurrently in separate threads
//SYSTEM_THREAD(ENABLED);




void setup() {
  Serial.begin(9600);
  waitFor(Serial.isConnected, 5000);
  delay(1000);

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
    file.printf("TimeStamp, temp_C, humidity_RH , pressure_PA \n");  //header just bme 1 for now
  Serial.printf("\nLogging to: %s \n",fileName);

  file.close();//everytime line is opened, have to close
  Serial.printf("File headers set up\n");

//initialize BME
  status=bme.begin(HEXADDRESS);//"bme"is just name of object in this function
  if (status==false) {//little bit fancier initialization
    Serial.printf("BME280 at address 0x%02X failed to start",HEXADDRESS);
  }

  Particle.syncTime();//don't need time zone for unix
  dataTimer=millis();
  
}


void loop() {
///put readings into array
  tempC =bme.readTemperature();//deg C
  pressPA =bme.readPressure();//pascals
  humidRH = bme.readHumidity();//%RH

   tempF=map(tempC,0.0,100.0,32.0,212.0);//map function in math.h?
  pressinHg=map(pressPA,3386.38,33863.89,1.0,10.0);//inputs need to be floats (so .0) for float output 
  pixNoTemp=map((int)tempF,32,100,0,15);//to convert map temp to light scale
  pixNoHum=map((int)humidRH,0,100,0,15);// humid to light
  pixNoPres=map((int)pressinHg,22,32,0,15);

  
 
//if(bmeTimer.isTimerReady()){
  if((millis()-BMEDataTimer)>DELAYTIME){//or use instead of timer object  
  Serial.printf("temp=%0.2fC\npressure=%0.2f pascals\nhumidity=%0.2f%%RH\n",tempC,pressPA,humidRH);
  Serial.printf("temp=%0.2fF\npressure=%0.2f inHg\nhumidity=%0.2f%%RH\n",tempF,pressinHg,humidRH);
  }
  if((millis()-dataTimer)>DATAINT){
  dataArray[0]=(int)Time.now();//function to get unix time
  dataArray[1]=random(0,10);/this will be reading BMEs
  dataArray[2]=random(100,1000);
  writeSD(dataArray);//call function with array as argument
  dataTimer=millis();

}
  
}

///////function to write to SD Card in lines///////
void writeSD(int dataArray[3]){//imput will be impedance array however formatted
  if (!file.open(fileName, O_WRONLY | O_AT_END)) { // open file for printing and append to end
    Serial.println("File Failed to Open");
  }
  file.printf("%i,%i,%i\n",dataArray[0],dataArray[1],dataArray[2]);  // print header row
  Serial.printf("\nLogging to: %s \n",fileName);

  file.close();//everytime line is opened, have to close
  Serial.printf("Done \n");

}