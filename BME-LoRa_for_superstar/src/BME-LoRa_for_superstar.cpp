/* 
 * Project Code to send BME data to argon for writing to SD card
 * Author: Laura Green
 * Date: May 9, 2024
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "Particle.h"
#include "Adafruit_BME280.h"

const int BMEADDRESS2=0x77;//original address
const int BMEDELAY=15000;//btw bme readings
//float tempC1SS;
//float tempC2;
//float pressPA1SS;
//float pressPA2;
//float humidRH1SS;
//float humidRH2;
int timeStamp;
bool status;
float SSBMEArray[3];//temp, humidity, pressure for both BME's (plus data coming from other microcontroller?)
const int DATAINT=15000;//interval between data collection
int BMEDataTimer;//when to collect

//LoRa network constants-how do I know how to set this up outside of FUSE???
const int RADIONETWORK = 8;    // range of 0-16 I'm using 8
const int SENDADDRESS = 0x88;   // address of radio to be sent to for now I just want to recieve on this code
//const int LIGHTTIME=1000;//for D7 to light for 5 s when new LoRa data recieved-changed to 1s
//const int LIGHTPIN=D7;//turning on, not off?? same for Argon?
//declare functions
void sendData(int timeStamp, float SSBMEArray);



// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(SEMI_AUTOMATIC);//semi?

// Run the application and system concurrently in separate threads
SYSTEM_THREAD(ENABLED);




void setup() {
  Serial.begin(9600);
  waitFor(Serial.isConnected, 5000);
  delay(1000);
  
//for LoRa network
  Serial1.begin(115200);//why this number?
  reyaxSetup(password);//call function for LoRa set up

  //initialize BME
  status=bme1.begin(BMEADDRESS2);//"bme"is just name of object in this function
  if (status==false) {//little bit fancier initialization
    Serial.printf("BME280 at address 0x%02X failed to start",BMEADDRESS1);
  }

  Particle.syncTime();//don't need time zone for unix
  BMEDataTimer=millis();

  
}


void loop() {
  //read data from BME and put into array to send?
  if((millis()-BMEDataTimer)>BMEDELAY){//or use instead of timer object  
  timeStamp=(int)Time.now();//function to get unix time
  //for(i=0;i<9;i++){}
  //BMEArray[i] =bme1.readTemperature();//deg C
  //BMEArray[0]=bme1.readTemperature();//deg C
 // BMEArray[1] = bme1.readHumidity();//%RH
 // BMEArray[2] =bme1.readPressure();//pascals
  SSBMEArray[0] =bme2.readTemperature();//deg C
  SSBMEArray[1] = bme2.readHumidity();//%RH
  SSBMEArray[2] =bme2.readPressure();//pascals
 // Serial.printf("time=%i\ntempC1=%0.2f\nhumidRH1=%0.2f\npressPA1=%0.2f\n",timeStamp,BMEArray[0],BMEArray[1],BMEArray[2]);
 Serial.printf("time=%i\ntempC1=%0.2f\nhumidRH1=%0.2f\npressPA1=%0.2f\n",timeStamp,SSBMEArray[0],SSBMEArray[1],SSBMEArray[2]);
  
 // writeSD(timeStamp,BMEArray);//call function with array as argument
  BMEDataTimer=millis();

}

  if (Serial1.available())  {

  sendData(timeStamp,SSBMEArray[0],SSBMEArray[1],SSBMEArray[2]);
  
  }
  
  
}
void sendData(int timeStamp,float SSBMEArray) {
  char buffer[60];//declare empty buffer within function
  sprintf(buffer, "AT+SEND=%i,%f,%f,%f\r\n", timeStamp, SSBMEArray[0], SSBMEArray[1],SSBMEArray[2]);//what is \r? write data to buffer
  Serial1.printf("%s",buffer);
  //Serial1.println(buffer); 
  delay(1000);
  if (Serial1.available() > 0)
  {
    Serial.printf("Awaiting Reply from send\n");
    String reply = Serial1.readStringUntil('\n');//dont need if only sending one way?
    Serial.printf("Send reply: %s\n", reply.c_str());//not sure what reply will be???
  }
}

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