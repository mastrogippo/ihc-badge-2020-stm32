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

#include "main.h"

//all keypad implementatins are CRAP. let's make a better one.
//constraint: it's unlikely that people will press more than 2 buttons at the same time. I'm not afraid of ghosts

static volatile char keyp = 0;

#define boot_key KeyStatus[12] == key_pressed // *
#define C_key KeyStatus[3] == key_pressed // C

#define debounce_ms 30

#define key_idle 0
#define key_pressed_debounce 5
#define key_pressed 10
#define key_released_debounce 15
#define key_released 20

#define NUM_KEYS 23
//                  0    1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18   19   20   21   22
char KeyMap[23] = {'3', '2', '1', '0', '7', '6', '5', '4', 'B', 'A', '9', '8', 'F', 'E', 'D', 'C', 'i', 'h', 'c', 'd', 'g', 'a', 'e'};
unsigned long KeyDebounce[23];
unsigned char KeyStatus[23];

byte rowPins[4] = {ky1_Pin, ky2_Pin, ky3_Pin, ky4_Pin}; //connect to the row pinouts of the keypad
byte colPins[4] = {kx1_Pin, kx2_Pin, kx3_Pin, kx4_Pin}; //connect to the column pinouts of the keypad
byte btnPins[7] = {btn1, btn2, btn3, btn4, btn5, btn6, btn7};


void KeypadInit()
{
  for(int i = 0; i < 4; i++)
  {
    pinMode(rowPins[i], INPUT_PULLUP);
    pinMode(colPins[i], INPUT);
  }
  for(int i = 0; i < NUM_KEYS; i++)
  {
    KeyStatus[i] = key_idle;
    KeyDebounce[i] = 0;
  }
  
  for(int i = 0; i < 7; i++)
    pinMode(btnPins[i], INPUT_PULLUP);
  //attachInterrupt(digitalPinToInterrupt(kp_Pin), kp_int, FALLING);

}

//send keypad status
void handler_key(void) {
    uint8_t tmp;
    uint8_t cnt = 0;
    SerialESP.write(0xFC);
    //SerialPC.print(0xFC, HEX);
    //SerialPC.write(' ');
    tmp = 0;
    for(int i = 0; i < 8; i++)
    {
      tmp <<= 1;
      if(KeyStatus[cnt++] == key_pressed)
        tmp +=1;
    }
    //SerialPC.print(tmp, HEX);
    //SerialPC.write(' ');
    SerialESP.write(tmp);
    
    tmp = 0;
    for(int i = 0; i < 8; i++)
    {
      tmp <<= 1;
      if(KeyStatus[cnt++] == key_pressed)
        tmp +=1;
    }
    //SerialPC.print(tmp, HEX);
    //SerialPC.write(' ');
    SerialESP.write(tmp);
    
    tmp = 0;
    for(int i = 0; i < 7; i++)
    {
      tmp <<= 1;
      if(KeyStatus[cnt++] == key_pressed)
        tmp +=1;
    }
    //SerialPC.println(tmp, HEX);
    SerialESP.write(tmp);
    SerialESP.write(0xFE);
} 

void DoKey(int KeyID, bool stat)
{  
  switch(KeyStatus[KeyID])
  {
    case key_idle:
      if(stat)
      {
        KeyDebounce[KeyID] = millis() + debounce_ms;
        KeyStatus[KeyID] = key_pressed_debounce;
      }
    break;

    case key_pressed_debounce:
      if(millis() > KeyDebounce[KeyID])
      {
        KeyStatus[KeyID] = key_pressed;
        /*SerialESP.write(0xFD);
        SerialESP.write('D');
        SerialESP.write(KeyMap[KeyID]);
        SerialESP.write(0xFE);*/
      }
    break;

    case key_pressed:
      if(!stat)
      {
        KeyDebounce[KeyID] = millis() + debounce_ms;
        KeyStatus[KeyID] = key_released_debounce;
      }
    break;
    
    case key_released_debounce:
      if(millis() > KeyDebounce[KeyID])
      {
        KeyStatus[KeyID] = key_idle;
        /*SerialESP.write(0xFD);
        SerialESP.write('U');
        SerialESP.write(KeyMap[KeyID]);
        SerialESP.write(0xFE);*/

        //16 + 17
        //if((KeyStatus[16] == key_pressed) && (KeyStatus[17] == key_pressed)) //I + H
        if((KeyStatus[16] == key_pressed)) //I
        {
          if(KeyID == 17) //H
          {
            SerialPC.println("RESET BOOTLOADER");
            ESP32_Bootloader(true);
          }
          if(KeyID == 18) //C
          {
            SerialPC.println("RESET APP");
            ESP32_Bootloader(false);
          }
        }
      }
    break;
  }
}


void DoKeypad()
{
  int ki = 0xFF; //key index
  for(int i = 0; i < 4; i++)
  {
    pinMode(colPins[i], OUTPUT);
    digitalWrite(colPins[i], LOW);
    for(int j = 0; j < 4; j++)
    {//scan rows
      ki = (i*4)+j;
      DoKey(ki, !digitalRead(rowPins[j]));
    }
    pinMode(colPins[i], INPUT);
  }
  for(int i = 0; i < 7; i++)
  {
    DoKey(i+16, !digitalRead(btnPins[i]));
  }
  /*
  if (key) {
      
      SerialPC.print("K");
      SerialPC.println(key);
      
      if ( rtttl::isPlaying() && (key == 'C') )
        rtttl::stop();
        
      if(key == '*') KKK=true;
      else KKK=false;
      
      if ((Status == ESP_ON) && EnableKeyLight) 
      {
        digitalWrite(led_Pin, HIGH);
        TimeLED_K = millis() + TimeLED_Krst;
      }
    }*/
}

void KeypadTest()
{
      //Test keyboard
  for (int i = 0; i < 4; i++)
  {
    pinMode(colPins[i], OUTPUT);
    digitalWrite(colPins[i], LOW);
  }

  for (int i = 0; i < 4; i++)
  {
    pinMode(rowPins[i], INPUT_PULLUP);

    if (!digitalRead(rowPins[i]))
    {
      SerialPC.print("Error: LOW on row ");
      SerialPC.println(i);
    }
  }
}