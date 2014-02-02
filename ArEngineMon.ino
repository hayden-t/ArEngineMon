/*
Copyright (C) 2013 Hayden Thring www.httech.com.au

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.

*/


const int Version = 104;//change to force load default settings and save them to eeprom

#include <LCD.h>
#include <LiquidCrystal.h>
#include <buttons.h>
#include <MENWIZ.h>
#include <RunningAverage.h>
#include <EEPROM.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>

Adafruit_BMP085 bmp;

const int UP_BOTTON_PIN      = 3;
const int DOWN_BOTTON_PIN    = 4;
const int CONFIRM_BOTTON_PIN = 2;
const int ESCAPE_BOTTON_PIN  = 5;

const int HOME_TIMEOUT  = 4000;
const int SPLASH_TIMEOUT  = 5000;

const int LED_PIN = 13;
int LED_ON = 200;
int LED_OFF = 100;

const int BUZZER_PIN = 6;
int BUZZER_ON = 60;//millis on for
int BUZZER_OFF= 2000;//millis off for

int STARTUP_DELAY = 5000; //delay before record max or check for alert

const int SENSOR1_PIN = A0;
int SENSOR1;//raw reading
float SENSOR1_LOW = 2.2;
float SENSOR1_HIGH = 0;
int SENSOR1_ALARM = 90;
double SENSOR1_RECORD = 0;
float SENSOR1_VOLTAGE;
double SENSOR1_PERCENT;
boolean SENSOR1_ALERT = false;
RunningAverage SENSOR1_AVERAGE(10);

const int SENSOR2_PIN = A7;
int SENSOR2;//raw reading
double SENSOR2_ALARM = 1.00;
float SENSOR2_VOLTAGE;
boolean SENSOR2_ALERT = false;

const int SENSOR3_PIN = A6;
int SENSOR3;//raw reading
int SENSOR3_ALARM = 90;
float SENSOR3_VOLTAGE;
boolean SENSOR3_ALERT = false;

int BARO_ALT;

const int SENSOR_INTERVAL = 1000;//delay between sensor reads

int DEBUG_MODE = 0;//show raw sensor values instead of graph

const double lcdNumCols = 16.0; //why does this need to be a double ?
const double lcdNumRows = 2.0;

menwiz menu;
// create lcd obj using LiquidCrystal lib
LiquidCrystal lcd ( 12, 11, 10, 9, 8, 7 );

byte p1[8] = {  0x10,  0x10,  0x10,  0x10,  0x10,  0x10,  0x10,  0x10};
byte p2[8] = {  0x18,  0x18,  0x18,  0x18,  0x18,  0x18,  0x18,  0x18};
byte p3[8] = {  0x1C,  0x1C,  0x1C,  0x1C,  0x1C,  0x1C,  0x1C,  0x1C};
byte p4[8] = {  0x1E,  0x1E,  0x1E,  0x1E,  0x1E,  0x1E,  0x1E,  0x1E};
byte p5[8] = {  0x1F,  0x1F,  0x1F,  0x1F,  0x1F,  0x1F,  0x1F,  0x1F};
byte c1[8] = { B10000, B10010, B10010, B00010, B00010, B10000, B10010, B10000 };

void setup(){

  Serial.begin(9600);
  //Serial.println(sensorValue);
  
  bmp.begin();
  
  _menu *r,*s1,*s2;

  int  mem;

  pinMode(LED_PIN, OUTPUT);
  
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, HIGH);

  
  pinMode(SENSOR1_PIN, INPUT);           // set pin to input
  pinMode(SENSOR2_PIN, INPUT);           // set pin to input
  pinMode(SENSOR3_PIN, INPUT);           // set pin to input

  menu.begin(&lcd,lcdNumCols,lcdNumRows); //declare lcd object and screen size to menwiz lib

  r=menu.addMenu(MW_ROOT,NULL,F("Settings"));
    
  s1=menu.addMenu(MW_SUBMENU,r, F("ECT Sensor"));    
      s2=menu.addMenu(MW_VAR,s1,F("ECT Alarm %"));
          s2->addVar(MW_AUTO_INT,&SENSOR1_ALARM,0,100,1);
      s2=menu.addMenu(MW_VAR,s1,F("ECT Low"));
          s2->addVar(MW_AUTO_FLOAT,&SENSOR1_LOW,0,5,0.1);
      s2=menu.addMenu(MW_VAR,s1,F("ECT High"));
          s2->addVar(MW_AUTO_FLOAT,&SENSOR1_HIGH,0,5,0.1);
          
  s1=menu.addMenu(MW_VAR,r, F("Buzzer Duty"));    
      s1->addVar(MW_AUTO_INT,&BUZZER_ON,20,1000,20);
  
  s1=menu.addMenu(MW_VAR,r,F("Debug Mode"));              
        s1->addVar(MW_LIST,&DEBUG_MODE); 
        s1->addItem(MW_LIST, F("Disabled"));
        s1->addItem(MW_LIST, F("ECT Sensor"));
        s1->addItem(MW_LIST, F("Sensor 2 & 3"));
      
   s1=menu.addMenu(MW_VAR,r,F("Save"));
      s1->addVar(MW_ACTION,save);      
      

  menu.navButtons(UP_BOTTON_PIN,DOWN_BOTTON_PIN,ESCAPE_BOTTON_PIN,CONFIRM_BOTTON_PIN);  
  

 // char splash[] = "test \n hello \n";
 // menu.addSplash( splash, SPLASH_TIMEOUT);

  menu.addUsrScreen(show_stats, HOME_TIMEOUT);


  lcd.createChar(0, p1);
  lcd.createChar(1, p2);
  lcd.createChar(2, p3);
  lcd.createChar(3, p4);
  lcd.createChar(4, p5);
  lcd.createChar(5, c1);

  
  int flashedVersion = EEPROM.read(1023);//last eeprom address on nano
  
  if(flashedVersion == Version){
      menu.readEeprom();
  }else{
      save();
      EEPROM.write(1023, Version);//last eeprom address on nano
  }

}

void loop(){
  
  read_stats();
  check_stats();
  menu.draw();
  
  
  //Serial.println(SENSOR1_VOLTAGE, 3);
  //Serial.println(SENSOR1);
}

unsigned long lastRead = 0;

void read_stats(){
  
  if(millis() - lastRead < SENSOR_INTERVAL)return;

//SENSOR1
  // read the input on analog pin
  SENSOR1_AVERAGE.addValue(analogRead(SENSOR1_PIN));
  SENSOR1 = SENSOR1_AVERAGE.getAverage();

  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  SENSOR1_VOLTAGE = SENSOR1 * (5.0 / 1023.0); 

  if(SENSOR1_HIGH > SENSOR1_LOW){//percentage adjusted for sensor zero and relative to voltage range    
     
     SENSOR1_PERCENT = (SENSOR1_VOLTAGE - SENSOR1_LOW) / (SENSOR1_HIGH - SENSOR1_LOW) * 100.0;
  }
  else if(SENSOR1_HIGH < SENSOR1_LOW){//invert, ie higher voltage = lower value (5v = 0%, 0v = 100%)
      
      SENSOR1_PERCENT = (SENSOR1_LOW - SENSOR1_VOLTAGE) / (SENSOR1_LOW - SENSOR1_HIGH) * 100.0;
      
  }
  else SENSOR1_PERCENT = 0;

 
  SENSOR1_PERCENT = max(SENSOR1_PERCENT, 0);//limit graph to above 0%
  SENSOR1_PERCENT = min(SENSOR1_PERCENT, 100);//limit graph to below 100% 
  
  if(SENSOR1_PERCENT > SENSOR1_RECORD && millis() > STARTUP_DELAY)SENSOR1_RECORD = SENSOR1_PERCENT;//store highest reading, after 5 sec startup delay
//SENSOR1


//SENSOR2
  // read the input on analog pin
  SENSOR2 = analogRead(SENSOR2_PIN);
  
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  SENSOR2_VOLTAGE = SENSOR2 * (5.0 / 1023.0);
//SENSOR2


//SENSOR3
  // read the input on analog pin
  SENSOR3 = analogRead(SENSOR3_PIN);
  
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  SENSOR3_VOLTAGE = SENSOR3 * (5.0 / 1023.0);
//SENSOR3
 
 
  BARO_ALT = bmp.readAltitude();
 
 
  lastRead = millis();

}


void check_stats(){
  
  if(millis() < STARTUP_DELAY)return;
  
  if(SENSOR1_PERCENT >= SENSOR1_ALARM)SENSOR1_ALERT = true;
  else  SENSOR1_ALERT = false;
  
  if(SENSOR2_VOLTAGE < SENSOR2_ALARM)SENSOR2_ALERT = true;
  else  SENSOR2_ALERT = false;

  if(SENSOR1_ALERT || SENSOR2_ALERT){
    led(ON);
    buzzer(ON);
  }
  else{
    led(OFF);
    buzzer(OFF);
  }


}


// user defined default screen

boolean cleared = false;


void show_stats(){

  
  if(!cleared){
      lcd.clear();
      cleared = true;     
  }  

  lcd.setCursor(0, 0);

  lcd.print("Tmp: ");
  lcd.print(SENSOR1_PERCENT, 0);
  lcd.print("/");
  lcd.print(SENSOR1_RECORD, 0);
  lcd.print("/");
  lcd.print(SENSOR1_ALARM);
  lcd.print("%");  

  lcd.print("     ");

  lcd.setCursor(0,1);

if(DEBUG_MODE == 0){

  // drawing black rectangles on LCD
  
  unsigned char b;
  unsigned int piece;
  
  double a = lcdNumCols / 100 * SENSOR1_PERCENT;

  if (a >= 1) {

    for (int i=1;i<a;i++) {

      lcd.write(4);

      b=i;
    }

    a=a-b;

  }


  piece=a*5;

  //lcd.print(piece); 

  // drawing charater's colums

  switch (piece) {

  case 0:
    break;

  case 1:
    lcd.print((char)0);
    break;

  case 2:
    lcd.print((char)1);
    break;

  case 3:
    lcd.print((char)2);
    break;

  case 4:
    lcd.print((char)3);
    break;

  case 5:
    lcd.print((char)4);
    break;

  }
     
  int alarmMarker = lcdNumCols / 100 * SENSOR1_ALARM;

  //piece = part block
  //b = whole blocks

  int pos = b;
  if(piece != 0)pos = b + 1;
  
  for (int i =0;i<(lcdNumCols-b);i++) {
    
    if((pos + i) == alarmMarker)lcd.print((char)5);
    else lcd.print(" ");
    
  }
  
}else{
  
  if(DEBUG_MODE == 1){      

     lcd.print("Oil:"); 
     
     if(SENSOR2_ALERT){
        lcd.print("LOW ");
     }else{
         lcd.print("OK ");
     }
     
     //lcd.print(SENSOR1_VOLTAGE, 3); 
     //lcd.print("v ");
     
     lcd.print("Alt:");
     lcd.print(BARO_ALT);
     lcd.print("m ");
     
     
    lcd.print("     "); 
  
  }else if(DEBUG_MODE == 2){
    
      lcd.print(SENSOR2_VOLTAGE, 3); 
      lcd.print("v "); 
      
      lcd.print(SENSOR3_VOLTAGE, 3); 
      lcd.print("v ");
      
      lcd.print("     ");
      
  }
  
}
  
}

boolean buzzerStatus = OFF;
unsigned long buzzerTimer;

void buzzer(boolean on){
  
  if(!on){
      digitalWrite(BUZZER_PIN, HIGH);
      buzzerStatus = OFF;
      buzzerTimer = 0;
  }
  else if(on && !buzzerStatus && buzzerTimer == 0){//turn buzzer on
      digitalWrite(BUZZER_PIN, LOW);
      buzzerTimer = millis();
      buzzerStatus = ON;
  }
  else if(buzzerStatus && (millis() - buzzerTimer) > BUZZER_ON){//turn buzzer off and pause
      digitalWrite(BUZZER_PIN, HIGH);
      buzzerStatus = OFF;
      buzzerTimer = millis();
  }
  else if(!buzzerStatus && (millis() - buzzerTimer) > BUZZER_OFF){//reset buzzer
      buzzerTimer = 0;
  }

}

boolean ledStatus = OFF;
unsigned long ledTimer;

void led(boolean on){
  
  if(!on){
      digitalWrite(LED_PIN, LOW);
      ledStatus = OFF;
      ledTimer = 0;
  }
  else if(on && !ledStatus && ledTimer == 0){//turn buzzer on
      digitalWrite(LED_PIN, HIGH);
      ledTimer = millis();
      ledStatus = ON;
  }
  else if(ledStatus && (millis() - ledTimer) > LED_ON){//turn buzzer off and pause
      digitalWrite(LED_PIN, LOW);
      ledStatus = OFF;
      ledTimer = millis();
  }
  else if(!ledStatus && (millis() - ledTimer) > LED_OFF){//reset buzzer
      ledTimer = 0;
  }

}

void save(){
  menu.writeEeprom();
}
