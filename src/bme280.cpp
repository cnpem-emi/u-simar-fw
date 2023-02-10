#include "bme280.h"

Adafruit_BME280 bme;//Variavel do sensor BME

Adafruit_BME280 bme2 = Adafruit_BME280(SS, MOSI, MISO, SCK);


//A função faz a media de uma numero de leituras pré determinadas
//A função delay define o intervlo entre cada amostra

/// @brief Função que faz as leituras das variaveis do BME(Temperatura, Pressão, Altitude e Umidade)
/// @param number_samples_mean Numero de amostras para a média
/// @param temperature_rtc Ponteiro que armazena o valor da temperatura
/// @param pressure_rtc Ponteiro que armazena o valor da pressão 
/// @param altitude_rtc Ponteiro que armazena o valor da altitude 
/// @param humidity_rtc Pornteiro que armazena o valor da umidade
void BME280_read_mean(int number_samples_mean, float *temperature_rtc, float *pressure_rtc, float *altitude_rtc, float *humidity_rtc) {
  //Variaveis que acumulam valores para a média
  float sum_temperature = 0;
  float sum_pressure = 0;
  float sum_altitude = 0;
  float sum_humidity = 0;
  Serial.println("Number of samples:"+String(number_samples_mean));
  for(int i =0; i<number_samples_mean;i++ ){//loop que realiza a somatória das amostras 
    sum_temperature += bme.readTemperature();
    sum_pressure += bme.readPressure();
    sum_altitude += bme.readAltitude(SEALEVELPRESSURE_HPA);
    sum_humidity += bme.readHumidity();
    delay(50);
  }
  //Funções que calculam a média e armazenam nos respectivos ponteiros
  *temperature_rtc = sum_temperature/number_samples_mean;
  *pressure_rtc = sum_pressure/number_samples_mean;
  *altitude_rtc = sum_altitude/number_samples_mean;
  *humidity_rtc = sum_humidity/number_samples_mean;
}

/// @brief Função que faz as leituras das variaveis do BME(Temperatura, Pressão, Altitude e Umidade) utilizando o protocolo SPI
/// @param number_samples_mean Numero de amostras para a média
/// @param temperature_rtc Ponteiro que armazena o valor da temperatura
/// @param pressure_rtc Ponteiro que armazena o valor da pressão 
/// @param altitude_rtc Ponteiro que armazena o valor da altitude 
/// @param humidity_rtc Pornteiro que armazena o valor da umidade
void BME280_read_mean_SPI(int number_samples_mean, float *temperature_rtc, float *pressure_rtc, float *altitude_rtc, float *humidity_rtc) {
  //Variaveis que acumulam valores para a média
  float sum_temperature = 0;
  float sum_pressure = 0;
  float sum_altitude = 0;
  float sum_humidity = 0;
  Serial.println("Number of samples:"+String(number_samples_mean));
  for(int i =0; i<number_samples_mean;i++ ){//loop que realiza a somatória das amostras 
    sum_temperature += bme2.readTemperature();
    sum_pressure += bme2.readPressure();
    sum_altitude += bme2.readAltitude(SEALEVELPRESSURE_HPA);
    sum_humidity += bme2.readHumidity();
    delay(50);
  }
  //Funções que calculam a média e armazenam nos respectivos ponteiros
  *temperature_rtc = sum_temperature/number_samples_mean;
  *pressure_rtc = sum_pressure/number_samples_mean;
  *altitude_rtc = sum_altitude/number_samples_mean;
  *humidity_rtc = sum_humidity/number_samples_mean;
}

/// @brief Função que coloca o BME280 em hibernação 
/// @param device_address Endereço do sensor
void BME280_Sleep(int device_address) {
  // BME280 Register 0xF4 (control measurement register) sets the device mode, specifically bits 1,0
  // The bit positions are called 'mode[1:0]'. See datasheet Table 25 and Paragraph 3.3 for more detail.
  // Mode[1:0]  Mode
  //    00      'Sleep'  mode
  //  01 / 10   'Forced' mode, use either '01' or '10'
  //    11      'Normal' mode
  bme2.setSampling(bme2.MODE_SLEEP);
  Serial.println("BME280 to Sleep mode...");
  Wire.beginTransmission(device_address);
  Wire.write((uint8_t)0xF4);       // Select Control Measurement Register
  Wire.write((uint8_t)0b00000000); // Send '00' for Sleep mode
  Wire.endTransmission();

}


/// @brief Função que verifica se o sensor esta conectado
/// @param device_address Endereço do sensor
void BME280_status(int device_address){
  while(!bme.begin(device_address)){
    Serial.println("Sensor BME280 not found");
    delay(10000);
  }
}
void BME280_status_2(int device_address){
  while(!bme2.begin(device_address)){
    Serial.println("Sensor BME280 2 not found");
    delay(10000);
  }
}




 