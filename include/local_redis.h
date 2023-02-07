#ifndef _REDIS_H_

#define _REDIS_H_

#include <Redis.h>// Banco de dados Redis
#include <WiFi.h>

//Time of sleep defaut
#define DEEP_SLEEP_TIME_DEFAUT 0.5//Unit in minutes
//Time between samples  
#define SAMPLE_TIME 1000//Unit in miliseconds
#define TIME_ON_DISPLAY_DEFAUT  5000//Unit in miliseconds
#define NUMBER_SAMPLE_MEAN_DAFAUT 10//Number of samples for mean

//Declaration for Redis Server
#define REDIS_ADDR "10.0.38.46"
#define REDIS_PORT 6379
#define REDIS_PASSWORD ""


void redisStart (float *deep_sleep_time,int *time_beetwen_samples, float *time_on_display,int *number_samples_mean);
void redis_update_BME280_data (float temperature_rtc,float pressure_rtc, float altitude_rtc,float humidity_rtc);
void checkUpdate();
void updateOTA();

#endif //_REDIS_H_