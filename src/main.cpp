#include <Wire.h>
#include <WiFi.h>
#include "main.h"
#include <RTClib.h>

// declaration for Wifi connection
#define WIFI_SSID "Devices"
#define WIFI_PASSWORD ""

RTC_DATA_ATTR float deep_sleep_time; // Variable RTC for the sleep time
int time_beetwen_samples;            // time beetwen mensurments
RTC_DATA_ATTR float time_on_display; // Time on display
int number_samples_mean;             // Numbre of samples by cycles

// Variables RTC of sensor
RTC_DATA_ATTR float temperature_rtc;
RTC_DATA_ATTR float pressure_rtc;
RTC_DATA_ATTR float altitude_rtc;
RTC_DATA_ATTR float humidity_rtc;
// Variable RTC for verification if the display turn off after de time of display
RTC_DATA_ATTR boolean cut_off_display;
// RTC_DATA_ATTR uint32_t time_init_deep_sleep;
// RTC_DATA_ATTR DateTime now;

void setup()
{
  setCpuFrequencyMhz(80); // Frequency of CPU
  Serial.begin(9600);//Inicializa o serial
  // Serial.begin(9600, SERIAL_8N1, 13, 12);
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TOUCHPAD)
  {//Se o touchpad foi acionado
    cut_off_display = true;//Variavel que verifica se o displey deve ser desligado
    show_in_display(time_on_display, temperature_rtc, pressure_rtc, altitude_rtc, humidity_rtc);//Mostra as informações no display
  }

  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER && cut_off_display == 1)
  {//Se o tempo de exibição da tela acabou 
    Serial.println("Entrou" + String(cut_off_display));
    turn_off_display();//Desliga o display
    cut_off_display = false;
    // Serial.println("tempo antigo: "+String(time_init_deep_sleep));
    // Serial.println("tempo novo: "+String(now.unixtime()));
    // Serial.println("Diferença: "+String(now.unixtime() - time_init_deep_sleep));
    // hibernation_sleep((time_init_deep_sleep - now.unixtime())/60);
    hibernation_sleep(deep_sleep_time,true);//ativa a hibernação profunda 
  }
  // verify sensor connection
  BME280_status(0x76);
  BME280_status_2(0x77);

  // connect to the WiFi
  WiFi.mode(WIFI_STA); // WiFi station mode
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  delay(1000);
  if (WiFi.status() != WL_CONNECTED)//Se o wifi não esta conectado
  {
    Serial.printf("NO Connect to the WiFi");
  }
  else//Se esta conectado
  {
    Serial.println("Connect to the WiFi");
    Serial.printf("IP Address: ");
    Serial.println(WiFi.localIP());
  }
  //Conecting to redis server
  redisStart(&deep_sleep_time, &time_beetwen_samples, &time_on_display, &number_samples_mean);

  delay(1000);
  Serial.println("Loopback program started");
  // Note the format for setting a serial port is as follows: Serial2.begin(baud-rate, protocol, RX pin, TX pin);

  // Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial2.begin(9600, SERIAL_8O2, 12, 13);
  Serial.println("Serial Txd is on pin: " + String(12));
  Serial.println("Serial Rxd is on pin: " + String(13));
}
/// @brief Controla o ciclo de medições do BME e salva no Redis 
void submit_for_redis(void)
{
  Serial.println("");
  Serial.println("#######################################################");
  BME280_read_mean(number_samples_mean, &temperature_rtc, &pressure_rtc, &altitude_rtc, &humidity_rtc);
  redis_update_BME280_data(temperature_rtc, pressure_rtc, altitude_rtc, humidity_rtc);
  Serial.println("#######################################################");
  Serial.println("");
  delay(1000);
  Serial.println("");
  Serial.println("#######################################################");
  BME280_read_mean_SPI(number_samples_mean, &temperature_rtc, &pressure_rtc, &altitude_rtc, &humidity_rtc);
  redis_update_BME280_data(temperature_rtc, pressure_rtc, altitude_rtc, humidity_rtc);
  Serial.println("#######################################################");
  Serial.println("");
  BME280_Sleep(0x76);
  BME280_Sleep(0x77);
}

void loop()
{

  print_wakeup_reason();
  print_wakeup_touchpad();
  // now = DateTime(2023, 02, 8, 14, 05, 01);
  submit_for_redis(); // Submit the informations for the Redis
  // time_init_deep_sleep =  now.unixtime();
  hibernation_sleep(deep_sleep_time, true);

  //  if(Serial.available()){
  //    Serial2.print(".");
  //    delay(1000);
  //    Serial.print(Serial2.read());
  //    delay(1000);
  //  }

  //}
}
