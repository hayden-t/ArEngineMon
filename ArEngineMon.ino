
/*
Copyright (C) 2013 Hayden Thring www.httech.com.au

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.

*/

// must edit MENWIZ.h to make:  #define MAX_OPTXMENU   	7

#define DISABLE_BUZZER

#define OIL_PRESSURE_SENDER //disable your simple on/off oil pressure switch

const int Version = 106;//change to force load default settings and save them to eeprom

char splash[] = " ARDUINO ENGINE\nMONITOR STARTING\n";

#include <LCD.h>
#include <LiquidCrystal.h>
//#include <buttons.h>
#include <MENWIZ.h>
#include <RunningAverage.h>
#include <EEPROM.h>
#include <Button.h>

const int UP_BOTTON_PIN      = 3;
const int DOWN_BOTTON_PIN    = 4;
const int CONFIRM_BOTTON_PIN = 2;
const int ESCAPE_BOTTON_PIN  = 5;

int MENU_TIMEOUT  = 4;
int SPLASH_TIMEOUT  = 5;
int LCD_TIMEOUT  = 10; //timeout for lcd backlight

const int LED_PIN = 13;
int LED_ON = 200;
int LED_OFF = 100;

const int LCD_PIN = A4;//backlight

boolean SLEEP = false;
unsigned long LAST_BUTTON_TIME;

const int BUZZER_PIN = 6;
int BUZZER_ON = 100;//millis on for
int BUZZER_OFF= 1000;//millis off for

int STARTUP_DELAY = 5000; //delay before record max or check for alert

//coolant temp
    const int SENSOR1_PIN = A0;
    int SENSOR1;//raw reading
    float SENSOR1_LOW = 2.5;
    float SENSOR1_HIGH = 0;
    int SENSOR1_ALARM = 80;
    int SENSOR1_RECORD = 0;
    float SENSOR1_VOLTAGE;
    int SENSOR1_PERCENT;
    boolean SENSOR1_ALERT = false;
    RunningAverage SENSOR1_AVERAGE(20);
//coolant temp
 
//oil pressure
    const int SENSOR2_PIN = A5;
    int SENSOR2;//raw reading
    float SENSOR2_VOLTAGE;
    boolean SENSOR2_ALERT = false;
    #ifdef OIL_PRESSURE_SENDER 
    int SENSOR2_RECORD = 0;
    float SENSOR2_LOW = 0.5;
    float SENSOR2_HIGH = 3;
    int  SENSOR2_PERCENT;
    int SENSOR2_ALARM = 30;
    RunningAverage SENSOR2_AVERAGE(10);
    #else
    double SENSOR2_ALARM = 1.00;
    #endif
//oil pressure    
 
//coolant float    
    const int SENSOR3_PIN = A6;
    int SENSOR3;//raw reading
    int SENSOR3_ALARM = 1.00;
    float SENSOR3_VOLTAGE;
    boolean SENSOR3_ALERT = false;
//coolant float    

const int SENSOR_INTERVAL = 500;//delay between sensor reads

int DEBUG_MODE = 0;//show voltages

const int lcdNumCols = 16; //why does this need to be a double ?
const int lcdNumRows = 2;

menwiz menu;
// create lcd obj using LiquidCrystal lib
LiquidCrystal lcd ( 12, 11, 10, 9, 8, 7 );

Button confirm = Button(CONFIRM_BOTTON_PIN, BUTTON_PULLUP_INTERNAL, true, 100);
Button escape = Button(ESCAPE_BOTTON_PIN, BUTTON_PULLUP_INTERNAL, true, 100);
Button up = Button(UP_BOTTON_PIN, BUTTON_PULLUP_INTERNAL, true, 100);
Button down = Button(DOWN_BOTTON_PIN, BUTTON_PULLUP_INTERNAL, true, 100);

char line[lcdNumCols];

int button_nav(){
  
if(confirm.uniquePress()){
  LAST_BUTTON_TIME = millis();
  if(SLEEP){
    SLEEP = false;
    return MW_BTNULL;
  }
  else return MW_BTC;
}
else if(escape.uniquePress()){
  LAST_BUTTON_TIME = millis();
  if(SLEEP){
    SLEEP = false;
    return MW_BTNULL;
  }
  else return MW_BTE;
}
else if(up.uniquePress()){
  LAST_BUTTON_TIME = millis();
  if(SLEEP){
    SLEEP = false;
    return MW_BTNULL;
  }
  else return MW_BTU;
}
else if(down.uniquePress()){
  LAST_BUTTON_TIME = millis();
  if(SLEEP){
    SLEEP = false;
    return MW_BTNULL;
  }
  else return MW_BTD;
}
else{
    if(millis() - LAST_BUTTON_TIME > ((LCD_TIMEOUT+MENU_TIMEOUT)*1000)){
      SLEEP = true;      
    }    
    return MW_BTNULL;
}
  
}

void setup(){

      Serial.begin(9600);
      Serial.println("Engine Monitor Starting...");
      
      _menu *r,*s1,*s2;
    
      int  mem;
    
      pinMode(LED_PIN, OUTPUT);
      pinMode(LCD_PIN, OUTPUT);
      digitalWrite(LCD_PIN, HIGH);
      
      pinMode(BUZZER_PIN, OUTPUT);
      tone(BUZZER_PIN, 4000, 100);
      delay(150);
      tone(BUZZER_PIN, 4000, 100);    
      
      menu.begin(&lcd,lcdNumCols,lcdNumRows); //declare lcd object and screen size to menwiz lib
    
      r=menu.addMenu(MW_ROOT,NULL,F("MENU"));
        
      s1=menu.addMenu(MW_SUBMENU,r, F("TEMPERATURE"));    
          s2=menu.addMenu(MW_VAR,s1,F("ALARM PERCENT"));
              s2->addVar(MW_AUTO_INT,&SENSOR1_ALARM,0,99,1);
          s2=menu.addMenu(MW_VAR,s1,F("LOW VOLTAGE"));
              s2->addVar(MW_AUTO_FLOAT,&SENSOR1_LOW,0,5,0.1);
          s2=menu.addMenu(MW_VAR,s1,F("HIGH VOLTAGE"));
              s2->addVar(MW_AUTO_FLOAT,&SENSOR1_HIGH,0,5,0.1);
       
     #ifdef OIL_PRESSURE_SENDER
      s1=menu.addMenu(MW_SUBMENU,r, F("OIL PRESSURE"));    
          s2=menu.addMenu(MW_VAR,s1,F("ALARM PERCENT"));
              s2->addVar(MW_AUTO_INT,&SENSOR2_ALARM,0,99,1);
          s2=menu.addMenu(MW_VAR,s1,F("LOW VOLTAGE"));
              s2->addVar(MW_AUTO_FLOAT,&SENSOR2_LOW,0,5,0.1);
          s2=menu.addMenu(MW_VAR,s1,F("HIGH VOLTAGE"));
              s2->addVar(MW_AUTO_FLOAT,&SENSOR2_HIGH,0,5,0.1);     
     #endif
     
     s1=menu.addMenu(MW_VAR,r,F("VOLTAGES"));
            s1->addVar(MW_LIST,&DEBUG_MODE); 
            s1->addItem(MW_LIST, F("OFF"));
            s1->addItem(MW_LIST, F("COOLANT"));
            s1->addItem(MW_LIST, F("OIL"));
              
      s1=menu.addMenu(MW_VAR,r, F("BUZZER DUTY"));    
          s1->addVar(MW_AUTO_INT,&BUZZER_ON,20,1000,20);
      
      s1=menu.addMenu(MW_VAR,r, F("LCD TIMEOUT"));    
          s1->addVar(MW_AUTO_INT,&LCD_TIMEOUT,0,60,1);
          
      s1=menu.addMenu(MW_VAR,r, F("MENU TIMEOUT"));    
          s1->addVar(MW_AUTO_INT,&MENU_TIMEOUT,0,60,1);   
          
       s1=menu.addMenu(MW_VAR,r,F("SAVE SETTINGS"));
          s1->addVar(MW_ACTION,save);
    
      menu.addUsrNav(button_nav, 4);
      menu.addSplash(splash, SPLASH_TIMEOUT*1000);
      menu.addUsrScreen(show_stats, (MENU_TIMEOUT*1000));
    
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
      
      SENSOR1_PERCENT = map(SENSOR1_VOLTAGE*10000, SENSOR1_LOW*10000, SENSOR1_HIGH*10000, 0, 99);
    
      SENSOR1_PERCENT = constrain(SENSOR1_PERCENT, 0, 99);//limit graph to  0% - 99%
      
      if(SENSOR1_PERCENT > SENSOR1_RECORD && millis() > STARTUP_DELAY)SENSOR1_RECORD = SENSOR1_PERCENT;//store highest reading, after 5 sec startup delay
//SENSOR1


//SENSOR2      
      // read the input on analog pin
  #ifndef OIL_PRESSURE_SENDER     
      SENSOR2 = analogRead(SENSOR2_PIN);
  #else   
      SENSOR2_AVERAGE.addValue(analogRead(SENSOR2_PIN));
      SENSOR2 = SENSOR2_AVERAGE.getAverage();
  #endif   
      
      // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
      SENSOR2_VOLTAGE = SENSOR2 * (5.0 / 1023.0);
      
    #ifdef OIL_PRESSURE_SENDER  
     if(SENSOR2_HIGH > SENSOR2_LOW){
       SENSOR2_PERCENT = map(SENSOR2_VOLTAGE*10000, SENSOR2_LOW*10000, SENSOR2_HIGH*10000, 0, 99);
     }else SENSOR2_PERCENT = 0;
       
       SENSOR2_PERCENT = constrain(SENSOR2_PERCENT, 0, 99);//limit graph to  0% - 99%
       if(SENSOR2_PERCENT > SENSOR2_RECORD && millis() > STARTUP_DELAY)SENSOR2_RECORD = SENSOR2_PERCENT;
    #endif
//SENSOR2


//SENSOR3
      // read the input on analog pin
      SENSOR3 = analogRead(SENSOR3_PIN);
      
      // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
      SENSOR3_VOLTAGE = SENSOR3 * (5.0 / 1023.0);
//SENSOR3
 
 
  lastRead = millis();

}


void check_stats(){
    
    if(millis() < STARTUP_DELAY)return;
    
    if(SENSOR1_PERCENT >= SENSOR1_ALARM)SENSOR1_ALERT = true;
    else  SENSOR1_ALERT = false;
    
  #ifndef OIL_PRESSURE_SENDER 
    if(SENSOR2_VOLTAGE < SENSOR2_ALARM)SENSOR2_ALERT = true;
    else  SENSOR2_ALERT = false;
  #else
    if(SENSOR2_PERCENT <= SENSOR2_ALARM || SENSOR2_PERCENT == 99)SENSOR2_ALERT = true;//99 = disconnected
    else  SENSOR2_ALERT = false;
  #endif
    
    if(SENSOR3_VOLTAGE < SENSOR3_ALARM)SENSOR3_ALERT = true;
    else  SENSOR3_ALERT = false;
  
    if(SENSOR1_ALERT || SENSOR2_ALERT || SENSOR3_ALERT){
      led(true);
      buzzer(true);
      SLEEP = false;
      LAST_BUTTON_TIME = millis();
    }
    else{
      led(false);
      buzzer(false);
    }  
    
    if(SLEEP)digitalWrite(LCD_PIN, LOW);
    else digitalWrite(LCD_PIN, HIGH);  
  
  }
  
  
  // user defined default screen
  
  boolean cleared = false;
  
  
 void show_stats(){
  
    
    if(!cleared){
        lcd.clear();
        cleared = true;     
    }  
  
    lcd.setCursor(0, 0);
  
    lcd.print("T: ");
    sprintf(line, "%02d/%02d/%02d%% ", SENSOR1_PERCENT, SENSOR1_RECORD, SENSOR1_ALARM);
    lcd.print(line);
  
    if(SENSOR3_ALERT){
          lcd.print("LOW");
    }else{
           lcd.print("OK ");
    }
  
    lcd.setCursor(0,1);
  
  if(DEBUG_MODE == 0){
    
      lcd.print("O: ");   
      
  #ifdef OIL_PRESSURE_SENDER  
    sprintf(line, "%02d/%02d/%02d%% ", SENSOR2_PERCENT, SENSOR2_RECORD, SENSOR2_ALARM);
    lcd.print(line);
  #endif
  
       if(SENSOR2_ALERT){
          lcd.print("LOW ");
       }else{
           lcd.print("OK ");
       }       
    
  }else{
    
    if(DEBUG_MODE == 1){ 
  
       lcd.print("1:"); 
       lcd.print(SENSOR1_VOLTAGE, 2); 
       lcd.print("v ");  
       
       lcd.print("3:");
       lcd.print(SENSOR3_VOLTAGE, 2); 
       lcd.print("v ");
    
    }else if(DEBUG_MODE == 2){
      
        lcd.print("2:"); 
        lcd.print(SENSOR2_VOLTAGE, 2); 
        lcd.print("v ");  
        
        lcd.print("     ");
        
    }    
  }    
}
  
  boolean buzzerStatus = false;
  unsigned long buzzerTimer;
  
 void buzzer(boolean on){
    
    #ifdef DISABLE_BUZZER
    return;  
    #endif
    
    if(!on){
        noTone(BUZZER_PIN);
        buzzerStatus = false;
        buzzerTimer = 0;
    }
    else if(on && !buzzerStatus && buzzerTimer == 0){//turn buzzer on
        tone(BUZZER_PIN, 4000);
        buzzerTimer = millis();
        buzzerStatus = true;
    }
    else if(buzzerStatus && (millis() - buzzerTimer) > BUZZER_ON){//turn buzzer off and pause
        noTone(BUZZER_PIN);
        buzzerStatus = false;
        buzzerTimer = millis();
    }
    else if(!buzzerStatus && (millis() - buzzerTimer) > BUZZER_OFF){//reset buzzer
        buzzerTimer = 0;
    }

}

boolean ledStatus = false;
unsigned long ledTimer;

void led(boolean on){
  
  if(!on){
      digitalWrite(LED_PIN, LOW);
      ledStatus = false;
      ledTimer = 0;
  }
  else if(on && !ledStatus && ledTimer == 0){//turn buzzer on
      digitalWrite(LED_PIN, HIGH);
      ledTimer = millis();
      ledStatus = true;
  }
  else if(ledStatus && (millis() - ledTimer) > LED_ON){//turn buzzer off and pause
      digitalWrite(LED_PIN, LOW);
      ledStatus = false;
      ledTimer = millis();
  }
  else if(!ledStatus && (millis() - ledTimer) > LED_OFF){//reset buzzer
      ledTimer = 0;
  }

}
void save(){
  menu.writeEeprom();
}
