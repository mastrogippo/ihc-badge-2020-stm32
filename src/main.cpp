/*
    SoundBadge
    Copyright (C) 2020 Mastro Gippo
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/ 

//This is a port of the Arduino Sketck from the 3310 badge.
//I should make classes for the components and shit

#include "main.h"


#define BOARDV3 1
#define TEST 1
//#define SER_TEST 1
//#define ExternalProgrammer 1 //Uncomment to put ESP_RST and ESP_BOOT as highZ


#define CMD_SN 'S' //S0
#define CMD_BATT 'B' //B0
#define CMD_LED 'L' //L0 off, L1 on, LKn on con timer n*1sec che riparte se si preme un tasto, LDn dove n > 2 con timer *100ms. LK0 disabilita funzione tastiera

#define BlinkPeriod 300
#define REPORT_INTERVAL 10000


static volatile int Status = ESP_OFF;


unsigned long TimeVIB = 0;
unsigned long TimeLED = 0;
unsigned long TimeLEDrst = 0;
unsigned long TimeReport = 0;
unsigned long TimeKEY = 0;

unsigned long TimeLEDBlink = 0;

void handler_key(void);


void DoBootloader() {
  byte in;
  if (SerialPC.available()) 
  {
    in = SerialPC.read();
    SerialESP.write(in);
  }
  if (SerialESP.available()) 
  {
    in = SerialESP.read();
    SerialPC.write(in);
  }
}

void DoCommPC() {
#ifndef SER_TEST
  
  static byte state = 0;
  
//  if (SerialPC.available()) SerialESP.write(SerialPC.read());
  while (SerialPC.available()) 
  {
    byte in = SerialPC.read();
    SerialESP.write(in);
    /*if (Status == ESP_OFF) //ESP is OFF, can go into bootloader
    switch (state)
    {
      case 0:
        if (in == '&')
          state = 2;
        break;
      case 2:
        if (in == 'B')
        {
          ESP32_Bootloader(true);
        }
        state = 0;
        break;
    }*/
  }
#endif
}

void DoCommESP() {
  static byte state = 0;
  static byte cmd = 0;
  static byte toneF = 0;

  while (SerialESP.available()) 
  {
    byte in = SerialESP.read();
    SerialPC.write(in);
    
    #ifndef SER_TEST
      if (Status == ESP_ON) //ESP is ON, normal boot. Avoid bootloader stuff
    #endif
    {
      switch (state)
      {
        case 0:
          if (in == '&')
            state = 2;
          break;
        case 2:
          if (in == '&')
            state = 5;
          break;
        case 5:
          cmd = in;
          state = 10;
          break;
        case 10:
          switch (cmd)
          {
            case CMD_SN: //&&S0
              if (in == '0')
              {
                SerialPC.println("SN req");
                uint8_t *idBase0 =  (uint8_t *) (0x1FFFF7E8);
                SerialESP.print("$SN,");
                for (int i = 0; i < 12; i++)
                {
                  if (*(idBase0 + i) < 0x10) SerialESP.print("0");
                  SerialESP.print(*(idBase0 + i), HEX);
                }
                SerialESP.print("++");
              }
              cmd = 0;
              break;

            case CMD_BATT:
              if (in == '0')
              {
                //AGGIUNGERE SE STA CARICANDO + NOTIFICA DI BATT SCARICA?
/*                int raw = analogRead(vBat_Pin);
                SerialPC.print("BATT req: ");
                SerialPC.print(raw * 1.61);
                SerialPC.println(" mV");

                SerialESP.print("$BV,");
                SerialESP.print(raw);
                SerialESP.print("++");*/
              }
              cmd = 0;
              break;
              
            case CMD_LED:
              switch(in)
              {
                case '0': digitalWrite(led_Pin, LOW); cmd = 0; break;
                case '1': digitalWrite(led_Pin, HIGH); cmd = 0; break;
                case 'K': cmd = in; state = 20; return;
                case 'D': cmd = in; state = 20; return;
              }
              break;
          }
          state = 0;
          break;

        case 20:
          if(cmd == 'D') TimeLED = millis() + ((unsigned long)(in)*100);
          {
            digitalWrite(led_Pin, HIGH);
            TimeLEDrst = ((unsigned long)(in)*1000);
            TimeLED = millis() + TimeLEDrst;
          }
          cmd = 0;
          state = 0; 
          break;

        case 25:
          cmd = 0;
          state = 0; 
          break;         
      }
    }
  }
}

void DoTimers()
{
  if(TimeLED != 0) 
    if(millis() >= TimeLED) 
    {
      digitalWrite(led_Pin, LOW);
      TimeLED = 0;
    }

  if(millis() >= TimeKEY)
  {
    handler_key();
    TimeKEY = millis() + 20;
  }    
  
/*  #ifndef SER_TEST
    if (Status == ESP_ON) //ESP is ON, normal boot. Avoid bootloader stuff
  #endif
  if(millis() >= TimeReport) 
  {
    //$R,2554,1,0++ vbat,usb_present,charging
    SerialESP.print("$R,");
    SerialESP.print(analogRead(vBat_Pin));

    #ifdef BOARDV3
      if(digitalRead(usbp_Pin))
        SerialESP.print(",1,");
      else
        SerialESP.print(",0,");
      if(!digitalRead(chg_Pin))
        SerialESP.print("1");
      else
        SerialESP.print("0");    
    #else
      SerialESP.print(",0,0"); 
    #endif
    SerialESP.print("++");
    TimeReport = millis() + REPORT_INTERVAL;
  }*/

  if(TimeLEDBlink != 0)
    if(millis() >= TimeLEDBlink)
    {
      digitalWrite(led_Pin, !digitalRead(led_Pin));
      TimeLEDBlink = millis() + BlinkPeriod;
    }
}


void ESP32_Bootloader(bool activate)
{
  digitalWrite(esp_rst_Pin, LOW);
  
  //Reverse logic on on V0!!
  if(activate)
    digitalWrite(esp_b0_Pin, HIGH);
  else
    digitalWrite(esp_b0_Pin, LOW);
  delay(200);
  digitalWrite(esp_rst_Pin, HIGH);
  //TimeLEDBlink = millis() + BlinkPeriod;
  if(activate)
    Status = ESP_ON_BOOT;
  else
    Status = ESP_ON;
}

void PowerCheck() {
  /*static unsigned long keyTime = 0;
  static byte stat = 0;
  switch (stat) {
    case 0:
      if (!digitalRead(kp_Pin))
      {
        stat = 5;
        keyTime = millis();
      }
      break;
    case 5:
      if ((keyTime + 1000) <= millis())
      {
        if (digitalRead(kp_Pin))
        {
          stat = 0;
          break;
        }
        if (Status == ESP_OFF)
        {
          SerialPC.print("Power ON [");
          SerialPC.print(boot_key);
          //Accendo ESP
          if (boot_key)
          { //Bootloader se accendo con * premuto!
            SerialPC.println("] boot");
            ESP32_Bootloader();
          }
          else
          {
            tone(buz_Pin, 1000, 8);
            SerialPC.println("] normal");
            digitalWrite(esp_rst_Pin, LOW);
            digitalWrite(esp_b0_Pin, HIGH);
            digitalWrite(led_Pin, HIGH);
            TimeLED_K = millis() + TimeLED_Krst;
            delay(100);
            digitalWrite(esp_rst_Pin, HIGH);
            Status = ESP_ON;
          }
        }
        else
        {
          SerialESP.print("$P0++");
          delay(200);
          digitalWrite(esp_rst_Pin, LOW);
          digitalWrite(led_Pin, LOW);
          Status = ESP_OFF;
          tone(buz_Pin, 100, 50);
          TimeLEDBlink = 0;
        }
        stat = 10;
      }

      break;

    default:
      stat = 0;
  }*/
}

void GetSN() {
  uint16_t *flashSize = (uint16_t *) (0x1FFFF7E0);
  uint8_t *idBase0 =  (uint8_t *) (0x1FFFF7E8);

  SerialPC.print("Flash size is ");
  SerialPC.print(*flashSize );
  SerialPC.println("k");

  SerialPC.print("Unique ID is ");
  for (int i = 0; i < 12; i++)
  {
    if (*(idBase0 + i) < 0x10) SerialPC.print("0"); //Arduino merda
    SerialPC.print(*(idBase0 + i), HEX);
    SerialPC.print(" ");
  }

  SerialPC.println();
}




void setup() {

 #ifdef ExternalProgrammer
  pinMode(esp_b0_Pin, INPUT);
  pinMode(esp_rst_Pin, INPUT);
 #else
  pinMode(esp_b0_Pin, OUTPUT);
  digitalWrite(esp_b0_Pin, LOW);
  
  pinMode(esp_rst_Pin, OUTPUT);
  digitalWrite(esp_rst_Pin, LOW);
 #endif
 
  digitalWrite(led_Pin, LOW);
  pinMode(led_Pin, OUTPUT);
  digitalWrite(led_Pin, HIGH);
  
  pinMode(LEDS_ON, OUTPUT);
  digitalWrite(LEDS_ON, LOW);
  
  //digitalWrite(LEDS_ON, HIGH); //Spegnere?

  
  pinMode(vBat_Pin, INPUT);
  pinMode(Pot1, INPUT);
  pinMode(Pot2, INPUT);

  SerialPC.begin(115200);
  SerialESP.begin(115200); //ESP32

#ifdef TEST
  delay(100);
  SerialPC.flush();
  
  //battery
  while(1)
  {
        int raw = analogRead(vBat_Pin);
    SerialPC.print("BATT req: ");
    SerialPC.print(raw * 1.41);
    SerialPC.println(" mV");
digitalWrite(led_Pin, HIGH);
delay(1000);
digitalWrite(led_Pin, LOW);
delay(1000);

    //DoRainbow();
    /*
    uint8_t time = millis() >> 4;

    for(uint16_t i = 0; i < ledCount; i++)
    {
      uint8_t p = time - i * 8;
      colors[i] = hsvToRgb((uint32_t)p * 359 / 256, 255, 255);
    }
  
    ledStrip.write(colors, ledCount, brightness);
  
    delay(10);*/
    
  }
  
  while (!SerialPC.available()) //Wait for some input
  {
    delay(100);
    digitalWrite(led_Pin, HIGH);
    delay(100);
    digitalWrite(led_Pin, LOW); 
  }
  SerialPC.println("STM32 Booting");
  digitalWrite(led_Pin, HIGH);
  delay(1000);
  digitalWrite(led_Pin, LOW);
  
  GetSN();
  
  KeypadTest();


  //Accendo ESP Bootloader
  SerialPC.println("ESP32 boot");
  digitalWrite(esp_rst_Pin, LOW);
  digitalWrite(esp_b0_Pin, LOW);
  digitalWrite(led_Pin, HIGH);
  delay(200);
  digitalWrite(esp_rst_Pin, HIGH);
  digitalWrite(led_Pin, LOW);
#endif

  KeypadInit();
  //keypad.addEventListener(keypadEvent);
  TimeReport = millis() + REPORT_INTERVAL;
}



void loop() {
  while (1)
  {
    if(Status == ESP_ON_BOOT)
    {
      DoBootloader();
    }
    else
    {
      DoCommPC();
      DoCommESP();
  
      DoKeypad();
  
      //PowerCheck();
      DoTimers();
    }
  }
}
