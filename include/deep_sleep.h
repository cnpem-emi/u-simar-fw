#ifndef _DEEP_SLEEP_H_

#define _DEEP_SLEEP_H_

#include "driver/adc.h"//controle dos drivers do esp
#include <esp_wifi.h>//Biblioteca para controle do wifi no esp
#include <esp_bt.h>//Biblioteca para controle do bluetooth do esp
#include <Wire.h>
#include <WiFi.h>

#define Threshold 40 /* Greater the value, more the sensitivity */

void print_wakeup_reason();
void print_wakeup_touchpad();
void hibernation_sleep(float time_deep_sleep, boolean active_touch);
void callback();

#endif //_DEEP_SLEEP_H_