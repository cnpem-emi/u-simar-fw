#ifndef _DISPLAY_H_

#define _DISPLAY_H_

#include "main.h"

//Bibliotecas do display
#include <Wire.h>
#include <Adafruit_SSD1306.h>//Display


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino resetpin)

//Variable of Display 
//Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//Prototype of functions
boolean show_in_display(float time_on_display, float temperature_rtc,float pressure_rtc, float altitude_rtc,float humidity_rtc);
void turn_off_display();

#endif //_DISPLAY_H_