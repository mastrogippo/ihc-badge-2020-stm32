#include "main.h"

// Create an object for writing to the LED strip.
APA102<ledDAT_Pin, ledSCK_Pin> ledStrip;
// Set the number of LEDs to control.
const uint16_t ledCount = 16;
// Create a buffer for holding the colors (3 bytes per color).
rgb_color colors[ledCount];
// Set the brightness to use (the maximum is 31).
const uint8_t brightness = 1;



/* Converts a color from HSV to RGB.
 * h is hue, as a number between 0 and 360.
 * s is the saturation, as a number between 0 and 255.
 * v is the value, as a number between 0 and 255. */
rgb_color hsvToRgb(uint16_t h, uint8_t s, uint8_t v)
{
    uint8_t f = (h % 60) * 255 / 60;
    uint8_t p = (255 - s) * (uint16_t)v / 255;
    uint8_t q = (255 - f * (uint16_t)s / 255) * (uint16_t)v / 255;
    uint8_t t = (255 - (255 - f) * (uint16_t)s / 255) * (uint16_t)v / 255;
    uint8_t r = 0, g = 0, b = 0;
    switch((h / 60) % 6){
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
    }
    return rgb_color(r, g, b);
}

void DoRainbow()
{
    uint8_t time = millis() >> 4;

    for(uint16_t i = 0; i < ledCount; i++)
    {
      uint8_t p = time - i * 8;
      colors[i] = hsvToRgb((uint32_t)p * 359 / 256, 255, 255);
    }
  
    ledStrip.write(colors, ledCount, brightness);
  
    delay(10);
}
