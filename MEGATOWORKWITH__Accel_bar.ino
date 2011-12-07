/*
    Web client
 
 This sketch connects to a website (http://www.google.com)
 using an Arduino Wiznet Ethernet shield. 
 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 
 created 18 Dec 2009
 by David A. Mellis
 
 */
#include <Wire.h>
#include <SPI.h>
#include <Ethernet.h>
#include "RTClib.h"
#include <SD.h>


#define BMP085_ADDRESS 0x77  // I2C address of BMP085
const unsigned char OSS = 0;  // Oversampling Setting
// Calibration values
int ac1;
int ac2; 
int ac3; 
unsigned int ac4;
unsigned int ac5;
unsigned int ac6;
int b1; 
int b2;
int mb;
int mc;
int md;
// b5 is calculated in bmp085GetTemperature(...), this variable is also used in bmp085GetPressure(...)
// so ...Temperature(...) must be called before ...Pressure(...).
long b5; 

short temperature;
long pressure;  

byte gps;
int pinX = 3;
int pinY = 2;
unsigned long serialTimer = millis();
unsigned long xAcc = 0;
unsigned long yAcc = 0;
boolean flipflop;

int rxPin = 0;                    // RX PIN 
int txPin = 1;                    // TX TX

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x03 };
IPAddress server(69,89,31,63); // my IP server
const int requestInterval = 30000;
long lastAttemptTime = 0;            // last time you connected to the server, in milliseconds
boolean requested;
const int resetLED = 13;
float temp;
float voltage;
// Initialize the Ethernet client library
// with the IP address and port of the server 
// that you want to connect to (port 80 is default for HTTP):


const int chipSelect = 53;//changed from 8
const int LOCATION_FILE_NUMBER_LSB = 0x00;
const int LOCATION_FILE_NUMBER_MSB = 0x01;

File dataFile;
RTC_DS1307 RTC;
EthernetClient client;
DateTime now;

void setup() {
  // start the serial library:
  Serial.begin(38400);
    pinMode(pinX, INPUT);
  pinMode(pinY, INPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  // A2 is the ground, A3 is the power:
  digitalWrite(A2, LOW);
  digitalWrite(A3, HIGH);
    pinMode(chipSelect, OUTPUT);

  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:

  }
  Serial.println("card initialized.");


  Wire.begin();
  RTC.begin();
  
   bmp085Calibration();
  delay(50);
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
   // RTC.adjust(DateTime(__DATE__, __TIME__));
 }
  dataFile = SD.open("data.txt", FILE_WRITE);
  delay(500);

  // start the Ethernet connection:
  Ethernet.begin(mac);
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
//    for(;;)
//      ;
  }



  // connectToServer();
  // give the Ethernet shield a second to initialize:
  delay(1500);
  blink(resetLED, 3);
  Serial.println("connecting...");
  connectToServer();
}
void bmp085Calibration()
{
  ac1 = bmp085ReadInt(0xAA);
  ac2 = bmp085ReadInt(0xAC);
  ac3 = bmp085ReadInt(0xAE);
  ac4 = bmp085ReadInt(0xB0);
  ac5 = bmp085ReadInt(0xB2);
  ac6 = bmp085ReadInt(0xB4);
  b1 = bmp085ReadInt(0xB6);
  b2 = bmp085ReadInt(0xB8);
  mb = bmp085ReadInt(0xBA);
  mc = bmp085ReadInt(0xBC);
  md = bmp085ReadInt(0xBE);
}
void loop()
{
  accel();
if(Serial.available()>0){
     gps=Serial.read(); //echo incoming gps data 
  Serial.write(gps);
}
  now = RTC.now(); 
  if(client.connected()){
    if(!requested){

      requested = makeRequest(); 
   //   Serial.println("requesting!");
    }
    if(millis() - lastAttemptTime>requestInterval){
      //if youre not connected and two minutes have passed, attempt to connect again
      client.stop();
    //  Serial.println("stopping and reconnecting!");

     // getData();
      delay(1500);
      //connectToServer();
    }
    // if there are incoming bytes available 
    // from the server, read them and print them:

  }

  if (client.available()) {
    char c = client.read();
  //  Serial.print(c);
  }

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
   // Serial.println();
  //  Serial.println("disconnecting.");

    client.stop();
    delay(1500);
 if(millis() - lastAttemptTime>requestInterval){
      //if youre not connected and two minutes have passed, attempt to connect again
    connectToServer();
    //try to reconnect here...
 }
  }


}

void accel(){
   if (flipflop == true) {
     xAcc = pulseIn(pinX, HIGH);
     flipflop = false;
    } else {
	 yAcc = pulseIn(pinY, HIGH);
	 flipflop = true;
    }

  if ((millis() - serialTimer) > 150 ) {
//    Serial.print("X ");
//    Serial.println(xAcc);
//   // Serial.print("  ");
//    Serial.print("Y ");
//    Serial.println(yAcc);


  }
}
void getData(){
  voltage = 5 * analogRead(A0) / 1024.0;
  //float temp = 5 * analogRead(A1) / 1024.0;
  temp=(analogRead(A1))/10;

//  Serial.print(voltage);
//  Serial.print(F(","));
//  Serial.print(temp);
//  Serial.print(F(","));
//  Serial.println("   ");
//  
    if (dataFile) {
          DateTime now = RTC.now();
      // dataFile.write(gps);
  dataFile.print(now.month());
  dataFile.print('/');
  dataFile.print(now.day());
  dataFile.print('/');
  dataFile.print(now.year());
  dataFile.print(F(","));
  dataFile.print(now.hour());
  dataFile.print(F(":"));
  dataFile.print(now.minute());
  dataFile.print(F(":"));
  dataFile.print(now.second());
  dataFile.print(F(","));
  dataFile.print(voltage);
  dataFile.print(F(","));
  dataFile.print(temp);
    dataFile.print(F(","));
      dataFile.print(xAcc);
     dataFile.print(F(","));
   // Serial.print("Y ");
    dataFile.print(yAcc);
  dataFile.println();
          
    }
      dataFile.flush();

}

void connectToServer(){
 // Serial.println("connecting to server...");
  if (client.connect(server, 80)) {
    requested = false;
  }
  lastAttemptTime = millis();
}

boolean makeRequest() {
 // Serial.println("requesting");
  getData();
  // Make a HTTP request:
 
  client.print("GET /understanding_networks/dataLogger_Mega.php?data=");
  //client.write(gps);
  client.print(now.month());
  client.print('/');
  client.print(now.day());
  client.print('/');
  client.print(now.year());
  client.print(F(","));
  client.print(now.hour());
  client.print(F(":"));
  client.print(now.minute());
  client.print(F(":"));
  client.print(now.second());
  client.print(F(","));
  client.print(voltage);
  client.print(F(","));
  client.print(temp);
    client.print(F(","));
  client.print(xAcc);
   client.print(F(","));

    client.print(yAcc);
  client.println(" HTTP/1.1 ");
  client.println("HOST: www.levinegabriella.com");

  client.println();
  return true;

}


// Calculate temperature given ut.
// Value returned will be in units of 0.1 deg C
short bmp085GetTemperature(unsigned int ut)
{
  long x1, x2;

  x1 = (((long)ut - (long)ac6)*(long)ac5) >> 15;
  x2 = ((long)mc << 11)/(x1 + md);
  b5 = x1 + x2;

  return ((b5 + 8)>>4);  
}

// Calculate pressure given up
// calibration values must be known
// b5 is also required so bmp085GetTemperature(...) must be called first.
// Value returned will be pressure in units of Pa.
long bmp085GetPressure(unsigned long up)
{
  long x1, x2, x3, b3, b6, p;
  unsigned long b4, b7;

  b6 = b5 - 4000;
  // Calculate B3
  x1 = (b2 * (b6 * b6)>>12)>>11;
  x2 = (ac2 * b6)>>11;
  x3 = x1 + x2;
  b3 = (((((long)ac1)*4 + x3)<<OSS) + 2)>>2;

  // Calculate B4
  x1 = (ac3 * b6)>>13;
  x2 = (b1 * ((b6 * b6)>>12))>>16;
  x3 = ((x1 + x2) + 2)>>2;
  b4 = (ac4 * (unsigned long)(x3 + 32768))>>15;

  b7 = ((unsigned long)(up - b3) * (50000>>OSS));
  if (b7 < 0x80000000)
    p = (b7<<1)/b4;
  else
    p = (b7/b4)<<1;

  x1 = (p>>8) * (p>>8);
  x1 = (x1 * 3038)>>16;
  x2 = (-7357 * p)>>16;
  p += (x1 + x2 + 3791)>>4;

  return p;
}

// Read 1 byte from the BMP085 at 'address'
char bmp085Read(unsigned char address)
{
  unsigned char data;

  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();

  Wire.requestFrom(BMP085_ADDRESS, 1);
  while(!Wire.available())
    ;

  return Wire.read();
}

// Read 2 bytes from the BMP085
// First byte will be from 'address'
// Second byte will be from 'address'+1
int bmp085ReadInt(unsigned char address)
{
  unsigned char msb, lsb;

  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();

  Wire.requestFrom(BMP085_ADDRESS, 2);
  while(Wire.available()<2)
    ;
  msb = Wire.read();
  lsb = Wire.read();

  return (int) msb<<8 | lsb;
}

// Read the uncompensated temperature value
unsigned int bmp085ReadUT()
{
  unsigned int ut;

  // Write 0x2E into Register 0xF4
  // This requests a temperature reading
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(0xF4);
  Wire.write(0x2E);
  Wire.endTransmission();

  // Wait at least 4.5ms
  delay(5);

  // Read two bytes from registers 0xF6 and 0xF7
  ut = bmp085ReadInt(0xF6);
  return ut;
}

// Read the uncompensated pressure value
unsigned long bmp085ReadUP()
{
  unsigned char msb, lsb, xlsb;
  unsigned long up = 0;

  // Write 0x34+(OSS<<6) into register 0xF4
  // Request a pressure reading w/ oversampling setting
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(0xF4);
  Wire.write(0x34 + (OSS<<6));
  Wire.endTransmission();

  // Wait for conversion, delay time dependent on OSS
  delay(2 + (3<<OSS));

  // Read register 0xF6 (MSB), 0xF7 (LSB), and 0xF8 (XLSB)
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(0xF6);
  Wire.endTransmission();
  Wire.requestFrom(BMP085_ADDRESS, 3);

  // Wait for data to become available
  while(Wire.available() < 3)
    ;
  msb = Wire.read();
  lsb = Wire.read();
  xlsb = Wire.read();

  up = (((unsigned long) msb << 16) | ((unsigned long) lsb << 8) | (unsigned long) xlsb) >> (8-OSS);

  return up;
}






void blink(int thisPin, int howManyTimes){
  for (int blinks = 0;blinks<howManyTimes;blinks++){
    digitalWrite(thisPin, HIGH);
    delay(200);
    digitalWrite(thisPin, LOW);
    delay(200);
  } 
}

//questions: what other sensors will be good? 
//what is too much data
//i seem to be making my request twice


//when my request interval is more than 5000 i get 
//10/13/2011,11:14:56,3.67,15.00
//10/13/2011,11:15:5,3.65,15.00
//10/13/2011,11:15:16,0.00,0.00
//10/13/2011,11:15:44,0.00,0.00
//why

//can you go back a directory
//why if one thing works noting does - that's bad
//does my sketch NEED to be in 28400 baud for gps to work
//ALSO GET BAROMETER WORKING
