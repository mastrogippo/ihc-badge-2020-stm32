#include <Arduino.h>
#include "pins.h"

#include <APA102.h>


#define ESP_OFF 1
#define ESP_ON 5
#define ESP_ON_BOOT 10



#ifdef SER_TEST
  #define SerialESP Serial2
#else
  #define SerialESP Serial3
#endif
#define SerialPC Serial

//main
extern void ESP32_Bootloader(bool activate);


//keypad
extern void KeypadInit();
extern void KeypadTest();
extern void DoKeypad();

//leds
extern void DoRainbow();