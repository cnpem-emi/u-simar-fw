#ifndef _BME280_H_

#define _BME280_H_

#include "main.h"


#include <Adafruit_BME280.h>//Sernsor BME280
#include <Adafruit_GFX.h>
//#include <Adafruit_BMP280.h>

//Variable of Sensor (Temperature, pressure and Humidity)

#define SEALEVELPRESSURE_HPA (1013.25)

void BME280_Sleep(int device_address);
void BME280_Sleep_SPI();
void BME280_read_mean(int number_samples_mean, float *temperature_rtc, float *pressure_rtc, float *altitude_rtc, float *humidity_rtc);
void BME280_read_mean_SPI(int number_samples_mean, float *temperature_rtc, float *pressure_rtc, float *altitude_rtc, float *humidity_rtc);
void BME280_status(int device_address);
void BME280_status_2(int device_address);


#endif //_BME280_H_