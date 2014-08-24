//Master Script for SPPB Balance Test 
//Authors: Nicholas Marco, Yicheng Bai
//Last Updated: 7/7/2014

#include "Wire.h"
#include "I2Cdev.h"
#include "MPU6050.h"
#include "SoftwareSerial.h"
#include "IRremote.h"
#include "Nrf2401.h"

Nrf2401 Radio;

SoftwareSerial WiFlySerial(11,12);//(5,6); //RX  TX
SoftwareSerial portTwo(8,9);
int buttonPin = 10;//2; 
int buzzer = 7;  
int RFIDResetPin = 13;
int score;

#define NUM_BUTTONS 9
const uint16_t BUTTON_POWER = 0xD827; // for remote buttons
const uint16_t BUTTON_A = 0xF807;
const uint16_t BUTTON_B = 0x7887;
const uint16_t BUTTON_C = 0x58A7;
const uint16_t BUTTON_UP = 0xA05F;
const uint16_t BUTTON_DOWN = 0x00FF;
const uint16_t BUTTON_LEFT = 0x10EF;
const uint16_t BUTTON_RIGHT = 0x807F;
const uint16_t BUTTON_CIRCLE = 0x20DF;
IRrecv irrecv(buttonPin);
decode_results results; // This will store our IR received codes
uint16_t lastCode = 0; // This keeps track of the last code RX'd


MPU6050 accelgyro;
int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t mx, my, mz;


String StartSignal = "start";    
String StopSignal = "stop";
String Test1Complete = "first";
String Test2Complete = "second";
String Test3Complete = "third";
int Received = 0;

boolean flag_RFID = true;
boolean first_time = true;
boolean flag_check = false;
boolean flag_test = false;


int i,num,curtime,stime,ttime = 0;
int ax1,ax2,ay1,ay2,mag = 0;
int threshold = 15000;


void setup(void)
{
  Wire.begin();
  Serial.begin(9600);
  portTwo.begin(9600);
  WiFlySerial.begin(9600);
  portTwo.listen();
  irrecv.enableIRIn();
  
  pinMode(RFIDResetPin, OUTPUT);
  digitalWrite(RFIDResetPin, HIGH);
  
  pinMode(buzzer,OUTPUT);
  digitalWrite(buzzer,0);
  
  Serial.println("Initializing I2C devices...");
  accelgyro.initialize();
  Serial.println("Testing device connections...");
  Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");
 
}



void WiFiSend(String data_send)
{
  WiFlySerial.print(data_send);
  WiFlySerial.println("");
  WiFlySerial.println("");
}

void WiFiSend(unsigned int data_send)
{ 
  WiFlySerial.print(data_send);
  WiFlySerial.println("");
  WiFlySerial.println("");
}

//Function to read RFID and send through WIFI
void new_rfid()
{
  char tagString[15];
  int index = 0;
  boolean reading = false;
  while(portTwo.available()){
    int readByte = portTwo.read(); //read next available byte

    if(readByte == 2) reading = true; //begining of tag
    if(readByte == 3) reading = false; //end of tag

    if(reading && readByte != 2 && readByte != 10 && readByte != 13){
      //store the tag
      tagString[index] = readByte;
      index ++;
       if(index ==12){
        Serial.println(tagString);
        WiFiSend("RFID_Tag");
        delay(1000);
        tagString[12] = 'B';
        tagString[13] = 'T';        
        tagString[14] = '\0';
        WiFiSend(tagString);       
        delay(400);
        WiFlySerial.listen();
        flag_RFID = false;
        digitalWrite(buzzer,HIGH);
        delay(500);
        digitalWrite(buzzer,0);
        }
     }
  }
  clearTag(tagString);
  resetReader(); //reset the RFID reader
}

void clearTag(char one[]){
///////////////////////////////////
//clear the char array by filling with null - ASCII 0
//Will think same tag has been read otherwise
///////////////////////////////////
  for(int i = 0; i < strlen(one); i++){
    one[i] = 0;
  }
}
void resetReader(){
///////////////////////////////////
//Reset the RFID reader to read again.
///////////////////////////////////
  digitalWrite(RFIDResetPin, LOW);
  digitalWrite(RFIDResetPin, HIGH);
  delay(150);
}


void button_case()
{

  if (irrecv.decode(&results)) 
  {
    delay(200);
    /* read the RX'd IR into a 16-bit variable: */
    uint16_t resultCode = (results.value & 0xFFFF);
    
    
    if (resultCode == 0xFFFF)
    {
      resultCode = lastCode;
    }
    else
    {
      lastCode = resultCode;
    }
    
    switch (resultCode)
    {
      case BUTTON_POWER:
        Serial.println("Power");
          WiFiSend(StartSignal);
          flag_test = true;
          stime = millis();
        break;
      default:
        Serial.print("Unrecognized code received");
        break;        
    }    
    irrecv.resume(); // Receive the next value
  }
}

int16_t Radio_check()
{
    while(!Radio.available()) {
    //Serial.println("Reading...");
    };
    
    Radio.read();     
    
    if(Radio.data[0]=0x55)
      return true;
    else
      return false;
}

void getHeadingDegrees() {
  if (first_time == true)
  {
   ax1 = ax;
   ay1 = ay; 
   first_time = false;
  }
  
  ax2 = ax-ax1;
  ay2 = ay-ay1;
  mag = abs(ax2)+abs(ay2);
  //Serial.println(mag);
  if (mag >= threshold)
  {
   flag_check = true; 
  }
  ax1 = ax;
  ay1 = ay;
}

void SendScore() {
  
  if (num == 0)
    {
     score = 0;
    String Title = "BalanceScore";
    String myScore = String(score);
    String stringscore = String(Title + myScore);
    WiFiSend(stringscore); 
    delay(500);
    String First = "BalanceTimeFirst";
    String Sending = (First + ttime);
    WiFiSend(Sending);
    delay(500);
    }
   else if (num == 1)
    {
    score = 1;
    String Title = "BalanceScore";
    String myScore = String(score);
    String stringscore = String(Title + myScore);
    WiFiSend(stringscore);
    delay(500);
    String Second = "BalanceTimeSecond";
    String Sending = (Second + ttime);
    WiFiSend(Sending);
    delay(500);
     }
    else if (num == 2 && ttime>=3000)
     {
      score = 3;
     String Title = "BalanceScore";
     String myScore = String(score);
     String stringscore = String(Title + myScore);
     WiFiSend(stringscore);
     delay(500);
    String Third = "BalanceTimeThird";
    String Sending = (Third + ttime);
    WiFiSend(Sending);
    delay(500);
     }
      else 
     {
       score = 2;
      String Title = "BalanceScore";
      String myScore = String(score);
      String stringscore = String(Title + myScore);
      WiFiSend(stringscore);
      delay(500);
       String Third = "BalanceTimeThird";
       String Sending = (Third + ttime);
       WiFiSend(Sending);
       delay(500);
     }

  
}

void loop(void)
{
  
  if(WiFlySerial.available()) {
    Received = WiFlySerial.read();
    Serial.println(Received);
  }
 
 //Checks for RFID tag and transmits
 while (flag_RFID == true)
 {
   new_rfid();  
 }
  
  //Prospective Button Code 
//  while (flag_test == false)
//  {
//    button_case();  
//  }
  do{
    flag_test = Radio_check();
  }while(flag_test == false);
  
  //Motion Sensor. Checks for foot movement
  accelgyro.getMotion9(&ax, &ay, &az, &gx, &gy, &gz, &mx, &my, &mz);
  getHeadingDegrees();
  curtime = millis();
  //Received = WiFlySerial.read();
  
  //When movement, signals to slave
  if (flag_check == true)
  {
     WiFiSend(StopSignal);
     delay(100);
     Serial.println("Master Movement");
     flag_check = false;
     first_time = true;
     flag_test = false;
     flag_RFID = true;
     ttime = curtime-stime;
   SendScore();
     num = 0; 
     portTwo.listen();
  }
  
  if (Received == 49)      
  {
    Serial.println("Slave Movement");
    flag_test = false;
    first_time = true;
    flag_RFID = true;
    ttime = curtime-stime;
    SendScore();
    num = 0; 
    Received = 0;
    portTwo.listen();
  }
  
 
  //complete test if last 10 seconds
  if (curtime-stime >10000 && flag_test == true)
  {
    Serial.println("Test Completed");
    if(num == 0)
    {
      WiFiSend(Test1Complete);
    }
    if(num == 1)
    {
      WiFiSend(Test2Complete);
    }
    if(num == 2){
      WiFiSend(Test3Complete);
      delay(2000);
    }
    num = num+1;
    first_time = true;
    flag_test = false;
  }
  
  //when tests are complete reset module
  if (num == 3)
  {
   Serial.println("All Done");
   num = 4;
   score = 4;
   delay(2000);
   String Title = "BalanceScore";
  String myScore = String(score);
  String stringscore = String(Title + myScore);
  WiFiSend(stringscore);
   delay(500);
   num = 0;
   score = 0;
   flag_RFID = true;
   portTwo.listen();
  }

}
