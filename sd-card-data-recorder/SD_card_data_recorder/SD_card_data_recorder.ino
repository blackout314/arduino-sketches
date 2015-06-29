/*
   www.sgprojects.co.in
   Project : Arduino SD card data recorder
   Writer: Sameer Gupta
   Date : March 2014
   Hardware : Arduino Uno on 16MHz XL
   SD card attachements : CS = pin 10, MOSI=pin 11, MISO=pin 12, CLK=pin 13
   LCD connections : RS=7, EN=6, D4=5, D5=4, D6=3, D7=2
   RTC connection : SDA=18(AN4), SCL=19(AN5)
 */

#include <SD.h>
#include <LiquidCrystal.h>
#include <Wire.h>
#include <EEPROM.h>

#include "DHT.h"
#define DHTPIN 15
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define DS1307_ADDRESS 0x68
byte zero = 0x00; //workaround for issue #527
File myFile;

int ChipSelect = 10;
int second, minute, hour, weekDay, monthDay, month, year;

#define Up 1
#define Down 2
#define Menu 3
#define OK 4

int buttonState=0, run=1, buffer=0, count=0, key=0, cardPresent=0, interval=1;
const int Up_Pin = 8, Down_Pin = 9, Menu_Pin = 17, OK_Pin = 16;
long time;
int voltage, temp, humidity;
int hundredsDigit,tensDigit,onesDigit;

LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

void setup()
{
  pinMode(Up_Pin, INPUT),pinMode(Down_Pin, INPUT),pinMode(Menu_Pin, INPUT),pinMode(OK_Pin, INPUT);
  Wire.begin();
  lcd.begin(16, 2);  // Initialize 16x2 LCD set up
  lcd.setCursor(0,1), lcd.print("SGPROJECTS.CO.IN");
  lcd.setCursor(0,0);
  if (!SD.begin(ChipSelect))lcd.print("Card Not Found"),cardPresent=0;
  else lcd.print("SD Card Active"),cardPresent=1;
  delay(1000);
  lcd.clear();
  time = millis();
  readInterval();
  dht.begin();
}

void loop()
{
   if(run==1){
    showData();
    printDate();
    RecordData();
    delay(450);
   }

  if(CheckButton()!=0){
    debounce();

    if(CheckButton()==Menu && count==0)Display(),delay(500),count++;
    if(CheckButton()==Menu && count!=0)count++;

    if(count==1){
         if(CheckButton()==Up && key==0)hour++;
         else if(CheckButton()==Down && key==0)hour--;
         else if(CheckButton()==OK && key==0)key=1,LCD(1,0,0),lcd.print("Now set minute"),delay(1000);
         else if(CheckButton()==Up && key==1)minute++;
         else if(CheckButton()==Down && key==1)minute--;
         else if(CheckButton()==OK && key==1)key=0,setDateTime(),LCD(1,0,0),lcd.print("Time Saved"),delay(1000),count++;
         setLimit();
    }

    if(count==2){
         if(CheckButton()==Up && key==0)monthDay++;
         else if(CheckButton()==Down && key==0)monthDay--;
         else if(CheckButton()==OK && key==0)key=1,LCD(1,0,0),lcd.print("Now set month"),delay(1000);
         else if(CheckButton()==Up && key==1)month++;
         else if(CheckButton()==Down && key==1)month--;
         else if(CheckButton()==OK && key==1)key=2,LCD(1,0,0),lcd.print("Now set year"),delay(1000);
         else if(CheckButton()==Up && key==2)year++;
         else if(CheckButton()==Down && key==2)year--;
         else if(CheckButton()==OK && key==2)key=0,setDateTime(),LCD(1,0,0),lcd.print("Date Saved"),delay(1000),count++;
         setLimit();
    }

    if(count==3){
         if(CheckButton()==Up)interval++;
         else if(CheckButton()==Down)interval--;
         else if(CheckButton()==OK)LCD(1,0,0),writeInterval(),lcd.print("Interval Saved"),delay(1000),count++;
         setLimit();
    }

    if(count==4){
         if(CheckButton()==OK && key==0)key=1,LCD(1,0,0),lcd.print("Confirm???"),lcd.setCursor(0,1),lcd.print("Press again"),delay(1000);
         else if(CheckButton()==OK && key==1)key=0,RefreshCard(),count++;
         setLimit();
    }

    Display();

  } //Finishing Button checking

}

void Display(){
   LCD(1,0,0);
   switch(count){
      case 1 : run=0;
               lcd.print("Set Time");
               LCD(0,0,1);
               lcd.print(hour),lcd.print(":"),lcd.print(minute);
               break;
      case 2 : lcd.print("Set Date");
               LCD(0,0,1);
               lcd.print(monthDay),lcd.print("/"),lcd.print(month),lcd.print("/"),lcd.print(year);
               break;
      case 3 : lcd.print("Set Interval");
               LCD(0,0,1);
               lcd.print(interval);
               break;
      case 4 : lcd.print("Refresh Card ?");
               LCD(0,0,1);
               lcd.print("Ok=Yes / Menu=No");
               break;
     default : count=0;
               run=1;
    }
}

void setDateTime(){

  second =      0; //0-59
/*minute =      36; //0-59
  hour =        13; //0-23
  weekDay =     6; //1-7
  monthDay =    21; //1-31
  month =       3; //1-12
  year  =       14; //0-99
*/
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(zero); //stop Oscillator

  Wire.write(decToBcd(second));
  Wire.write(decToBcd(minute));
  Wire.write(decToBcd(hour));
  Wire.write(decToBcd(weekDay));
  Wire.write(decToBcd(monthDay));
  Wire.write(decToBcd(month));
  Wire.write(decToBcd(year));

  Wire.write(zero); //start
  Wire.endTransmission();
}

void printDate(){
  // Reset the register pointer
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(zero);
  Wire.endTransmission();

  Wire.requestFrom(DS1307_ADDRESS, 7);

  second = bcdToDec(Wire.read());
  minute = bcdToDec(Wire.read());
  hour = bcdToDec(Wire.read() & 0b111111); //24 hour time
  weekDay = bcdToDec(Wire.read()); //0-6 -> sunday - Saturday
  monthDay = bcdToDec(Wire.read());
  month = bcdToDec(Wire.read());
  year = bcdToDec(Wire.read());

  LCD(0,0,1);
  lcd.print("TIME ");
  if(hour>12)lcd.print(hour-12);
  else if(hour==0)lcd.print("12");
  else lcd.print(hour);
  lcd.print(":"),lcd.print(minute),lcd.print(":"),lcd.print(second);
  if(hour>12)lcd.print(" PM");
  else lcd.print(" AM");
}


byte decToBcd(byte val){              // Convert normal decimal numbers to binary coded decimal
  return ( (val/10*16) + (val%10) );
}

byte bcdToDec(byte val){             // Convert binary coded decimal to normal decimal numbers
  return ( (val/16*10) + (val%16) );
}


int CheckButton(){
  buffer = 0;
  buttonState = digitalRead(Up_Pin);
  if(buttonState == LOW)buffer=1;
  buttonState = digitalRead(Down_Pin);
  if(buttonState == LOW)buffer=2;
  buttonState = digitalRead(Menu_Pin);
  if(buttonState == LOW)buffer=3;
  buttonState = digitalRead(OK_Pin);
  if(buttonState == LOW)buffer=4;
  return buffer;
}

void LCD(int clr, int column, int row){
 if(clr == 1)lcd.clear();
 lcd.setCursor(column, row);
}

void debounce(){
 delay(150);
}

int TimeDelay(){
 long t=millis()-time;
 t=t/1000;
 return t;
}

void showData(){
 humidity = dht.readHumidity();
 temp = dht.readTemperature();
 LCD(1,0,0);

 float volts = analogRead(0);
 if(volts != 0){
   volts = volts*5;   // For reading voltage 5V approximate to 300V
   volts = volts/1023;
   volts = volts*60;
   voltage = volts;
   lcd.print(voltage),lcd.print("V,");
 }

 if(humidity !=0 && temp != 0){
   lcd.print(temp),lcd.print((char)223),lcd.print("C,RH:");
   lcd.print(humidity);
   lcd.print("%");
 }
}

void RecordData(){
  if(cardPresent==1 && TimeDelay()>=interval){
    myFile = SD.open("sg.txt", FILE_WRITE);
    if (myFile) {
      myFile.print(monthDay),myFile.print("/"),myFile.print(month),myFile.print("/"),myFile.print(year),myFile.print(", ");
      myFile.print(hour),myFile.print(":"),myFile.print(minute),myFile.print(":"),myFile.print(second),myFile.print(", ");
      if(analogRead(0)!=0){myFile.print(voltage),myFile.print("Volts, ");}
      myFile.print(temp),myFile.print("*C, RH:"),myFile.print(humidity),myFile.print("%"),myFile.println();
      myFile.close();
   }
   else LCD(1,0,0),lcd.print("Card Error"),delay(1000);
   time=millis();
  }
}

void RefreshCard(){
 LCD(1,1,0);
 if(cardPresent==1)SD.remove("test.txt"),lcd.print("Card Refreshed");
 else lcd.print("Card Not Found");
 delay(1000);
}

void setLimit(){
  if(hour>23)hour=0;
  else if(hour<0)hour=23;
  else if(minute>59)hour=0;
  else if(minute<0)minute=59;
  else if(monthDay>31)monthDay=1;
  else if(monthDay<0)monthDay=31;
  else if(month>12)month=1;
  else if(month<1)month=12;
  else if(year>99)year=0;
  else if(year<0)year=99;
  else if(interval>300)interval=1;
  else if(interval<1)interval=300;
}

void readInterval(){
  onesDigit = EEPROM.read(1);
  tensDigit = EEPROM.read(2);
  hundredsDigit = EEPROM.read(3);
  interval = 100*hundredsDigit+10*tensDigit+onesDigit;
}

void writeInterval(){
  onesDigit = interval%10;
  tensDigit = (interval/10)%10;
  hundredsDigit = (interval/100)%10;
  EEPROM.write(1, onesDigit);
  EEPROM.write(2, tensDigit);
  EEPROM.write(3, hundredsDigit);
}


