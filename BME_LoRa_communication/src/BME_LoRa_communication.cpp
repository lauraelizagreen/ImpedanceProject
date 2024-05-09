/* 
 * Project BME_LoRa_communication to send data from one BME/microcontroller to another that has SD card reader or wifi access
 * Author: Laura Green
 * Date: April 26, 2024
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include <SPI.h>
#include <SdFat.h>
#include "Adafruit_BME280.h"

//const int BMEADDRESS1=0x76;//wont use yet
const int BMEADDRESS2=0x77;// address set to
const int BMEDELAY=15000;//btw bme readings
//float tempC1;
float tempC2;
//float pressPA1;
float pressPA2;
//float humidRH1;
float humidRH2;
int timeStamp;
bool status;
float BMEArray[6];//temp, humidity, pressure for both BME's (plus data coming from other microcontroller?)

const int CS=A5;//to activate SD reader set low or SS (can be any digital pin)
const uint SDTIME=15000;//every 15 sec
const char FILE_BASE_NAME[]="Data";
int fileNumber;
const uint8_t BASE_NAME_SIZE=sizeof(FILE_BASE_NAME)-1;
char fileName[13];
//timer variables
const int DATAINT=15000;//interval between data collection
int BMEDataTimer;//when to collect
///declare function
void writeSD(int timeStamp,float BMEArray[6]);

//LoRa network constants-how do I know how to set this up outside of FUSE???
const int RADIONETWORK = 8;    // range of 0-16 I'm using 8
//const int SENDADDRESS = 302;   // address of radio to be sent to for now I just want to recieve on this code 
const int LIGHTTIME=1000;//for D7 to light for 5 s when new LoRa data recieved-changed to 1s
const int LIGHTPIN=D7;//turning on, not off?? same for Argon?

// Define User and Credentials
String password = "RoofAV"; // AES128 password-can this be anything?
String name = "SSData";
const int RADIOADDRESS = 0x88;//my address This is where other photon will send data


const int TIMEZONE= -6;
const unsigned int UPDATE=30000;//this needs to be same as BMEDataTimer??

//declare LoRa functions
//void sendData(//);//only recieving with this code will need this for other microcontroller?
void reyaxSetup(String password); 



//file system objects
SdFat sd;
SdFile file;
Adafruit_BME280 bme1;//define BME object 
Adafruit_BME280 bme2;//define BME object 

// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(AUTOMATIC);

// Run the application and system concurrently in separate threads
//SYSTEM_THREAD(ENABLED);




void setup() {
  Serial.begin(9600);
  waitFor(Serial.isConnected, 5000);
  delay(1000);
  
//for LoRa network
  Serial1.begin(115200);//why this number?
  reyaxSetup(password);//call function for LoRa set up

  //pinMode(LIGHTPIN,OUTPUT);
  //digitalWrite(LIGHTPIN,LOW);//start off
/*//leave SD card reader off for now, but will be on Argon
//intializing SD Card Reader
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
    file.printf("TimeStamp, temp_C1, humidity_RH1 , pressure_PA1, temp_C2, humidity_RH2 , pressure_PA2  \n");  //header (just bme 1 for now)
  Serial.printf("\nLogging to: %s \n",fileName);

  file.close();//everytime line is opened, have to close
  Serial.printf("File headers set up\n");
*/
//initialize BME
/*
  status=bme1.begin(BMEADDRESS1);//"bme"is just name of object in this function
  if (status==false) {//little bit fancier initialization
    Serial.printf("BME280 at address 0x%02X failed to start",BMEADDRESS1);
  }
  */
//only one BME per microcontroller for now
  status=bme2.begin(BMEADDRESS2);//"bme"is just name of object in this function
  if (status==false) {//little bit fancier initialization
    Serial.printf("BME280 at address 0x%02X failed to start",BMEADDRESS2);
  }
  


  Particle.syncTime();//don't need time zone for unix
  BMEDataTimer=millis();
  
}


void loop() {//will need to have variables for both BME's on Argon

// listen for incoming lora messages 
  if (Serial1.available())  { // full incoming buffer: not sure how to do this part..// for FUSE data was:full incoming buffer: +RCV=203,50,35.08,9,-36,41 

////if want light when data coming....
//digitalWrite(LIGHTPIN,HIGH);//turn on D7 light for length of LIGHTTIME
//lightTimer.startTimer(LIGHTTIME);

//strings for incoming data--I don't understand this-not until parse 3 is data?
    
    String parse0 = Serial1.readStringUntil('=');  //+RCV
    //String parse1 = Serial1.readStringUntil(',');  // address received from
    //String parse2 = Serial1.readStringUntil(',');  // buffer length
    //String parse3 = Serial1.readStringUntil(',');  // fuseSound
    //String parse4 = Serial1.readStringUntil(',');  // fuseDust
    //String parse5 = Serial1.readStringUntil(',');  // rssi
    //String parse6 = Serial1.readStringUntil('\n'); // snr
    //String parse7 = Serial1.readString();          // extra

    

    Serial.printf("parse0: %s\n",parse0.c_str());//parse1: %s\nparse2: %s\nparse3: %s\nparse4: %s\nparse5: %s\nparse6: %s\nparse7: %s\n", parse0.c_str(), parse1.c_str(), parse2.c_str(), parse3.c_str(), parse4.c_str(), parse5.c_str(), parse6.c_str(), parse7.c_str());
    delay(100);

    //then put into array 1st will have to convert to floats (since array data type)



  }
  
  if((millis()-BMEDataTimer)>BMEDELAY){//or use instead of timer object  
  timeStamp=(int)Time.now();//function to get unix time
  //for(i=0;i<9;i++){}
  //BMEArray[i] =bme1.readTemperature();//deg C
  //BMEArray[0]=bme1.readTemperature();//deg C
 // BMEArray[1] = bme1.readHumidity();//%RH
 // BMEArray[2] =bme1.readPressure();//pascals
  BMEArray[3] =bme2.readTemperature();//deg C
  BMEArray[4] = bme2.readHumidity();//%RH
  BMEArray[5] =bme2.readPressure();//pascals
 // Serial.printf("time=%i\ntempC1=%0.2f\nhumidRH1=%0.2f\npressPA1=%0.2f\n",timeStamp,BMEArray[0],BMEArray[1],BMEArray[2]);
 Serial.printf("time=%i\ntempC1=%0.2f\nhumidRH1=%0.2f\npressPA1=%0.2f\n",timeStamp,BMEArray[3],BMEArray[4],BMEArray[5]);
  
 // writeSD(timeStamp,BMEArray);//call function with array as argument
  BMEDataTimer=millis();

}
  
}

///////function to write to SD Card in lines///////
void writeSD(int timeStamp,float BMEArray[6]){//imput will be array of data from all BME's
  if (!file.open(fileName, O_WRONLY | O_AT_END)) { // open file for printing and append to end
    Serial.println("File Failed to Open");
  }
  file.printf("%i,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f\n",timeStamp,BMEArray[0],BMEArray[1],BMEArray[2],BMEArray[3],BMEArray[4],BMEArray[5]);  // print next line of data to same file
  Serial.printf("\nLogging to: %s \n",fileName);

  file.close();//everytime line is opened, have to close
  Serial.printf("Done \n");

}

///function to recieve LoRa data
void reyaxSetup(String password) {
  // following four paramaters have most significant effect on range
  // recommended within 3 km: 10,7,1,7
  // recommended more than 3 km: 12,4,1,7
  const int SPREADINGFACTOR = 10;
  const int BANDWIDTH = 7;
  const int CODINGRATE = 1;
  const int PREAMBLE = 7;
  String reply; // string to store replies from module

  Serial1.printf("AT+ADDRESS=%i\r\n", RADIOADDRESS); // set the radio address
  delay(200);
  if (Serial1.available() > 0) {
    Serial.printf("Awaiting Reply from address\n");
    reply = Serial1.readStringUntil('\n');
    Serial.printf("Reply address: %s\n", reply.c_str());
  }

  Serial1.printf("AT+NETWORKID=%i\r\n", RADIONETWORK); // set the radio network
  delay(200);
  if (Serial1.available() > 0) {
    Serial.printf("Awaiting Reply from networkid\n");
    reply = Serial1.readStringUntil('\n');
    Serial.printf("Reply network: %s\n", reply.c_str());
  }
   Serial1.printf("AT+CPIN=%s\r\n", password.c_str());
  delay(200);
  if (Serial1.available() > 0) {
    Serial.printf("Awaiting Reply from password\n");
    reply = Serial1.readStringUntil('\n');
    Serial.printf("Reply: %s\n", reply.c_str());
  }

  Serial1.printf("AT+PARAMETER=%i,%i,%i,%i\r\n", SPREADINGFACTOR, BANDWIDTH, CODINGRATE, PREAMBLE);
  delay(200);
  if (Serial1.available() > 0) {
    reply = Serial1.readStringUntil('\n');
    Serial.printf("reply: %s\n", reply.c_str());
  }

  Serial1.printf("AT+ADDRESS?\r\n");
  delay(200);
  if (Serial1.available() > 0) {
    Serial.printf("Awaiting Reply\n");
    reply = Serial1.readStringUntil('\n');
    Serial.printf("Radio Address: %s\n", reply.c_str());
  }

  Serial1.printf("AT+NETWORKID?\r\n");
  delay(200);
  if (Serial1.available() > 0) {
    Serial.printf("Awaiting Reply\n");
    reply = Serial1.readStringUntil('\n');
    Serial.printf("Radio Network: %s\n", reply.c_str());
  }

  Serial1.printf("AT+PARAMETER?\r\n");
  delay(200);
  if (Serial1.available() > 0) {
    Serial.printf("Awaiting Reply\n");
    reply = Serial1.readStringUntil('\n');
    Serial.printf("RadioParameters: %s\n", reply.c_str());
  }

  Serial1.printf("AT+CPIN?\r\n");
  delay(200);
  if (Serial1.available() > 0) {
    Serial.printf("Awaiting Reply\n");
    reply = Serial1.readStringUntil('\n');
    Serial.printf("Radio Password: %s\n", reply.c_str());
  }
}

