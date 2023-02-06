#include "bme280.h"

Adafruit_BME280 bme;

void BME280_read_mean(int number_samples_mean, float *temperature_rtc, float *pressure_rtc, float *altitude_rtc, float *humidity_rtc) {
  float sum_temperature = 0;
  float sum_pressure = 0;
  float sum_altitude = 0;
  float sum_humidity = 0;
  Serial.println("Number of samples:"+String(number_samples_mean));
  for(int i =0; i<number_samples_mean;i++ ){
    sum_temperature += bme.readTemperature();
    sum_pressure += bme.readPressure();
    sum_altitude += bme.readAltitude(SEALEVELPRESSURE_HPA);
    sum_humidity += bme.readHumidity();
    delay(50);
  }
  *temperature_rtc = sum_temperature/number_samples_mean;
  *pressure_rtc = sum_pressure/number_samples_mean;
  *altitude_rtc = sum_altitude/number_samples_mean;
  *humidity_rtc = sum_humidity/number_samples_mean;
}


void BME280_Sleep(int device_address) {
  // BME280 Register 0xF4 (control measurement register) sets the device mode, specifically bits 1,0
  // The bit positions are called 'mode[1:0]'. See datasheet Table 25 and Paragraph 3.3 for more detail.
  // Mode[1:0]  Mode
  //    00      'Sleep'  mode
  //  01 / 10   'Forced' mode, use either '01' or '10'
  //    11      'Normal' mode
  Serial.println("BME280 to Sleep mode...");
  Wire.beginTransmission(device_address);
  Wire.write((uint8_t)0xF4);       // Select Control Measurement Register
  Wire.write((uint8_t)0b00000000); // Send '00' for Sleep mode
  Wire.endTransmission();

}

void BME280_status(){
  while(!bme.begin(0x76)){
    Serial.println("Sensor BMx280 not found");
    delay(10000);
  }
}