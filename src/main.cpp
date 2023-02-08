#include <Wire.h>
#include <WiFi.h>
#include "main.h"
#include <RTClib.h>


//declaration for Wifi connection
#define WIFI_SSID "Devices"
#define WIFI_PASSWORD ""

RTC_DATA_ATTR float deep_sleep_time;//Variable RTC for the sleep time
int time_beetwen_samples;//time beetwen mensurments
RTC_DATA_ATTR float time_on_display;//Time on display
int number_samples_mean;//Numbre of samples by cycles

unsigned long delayTime;

//Variables RTC of sensor 
RTC_DATA_ATTR float temperature_rtc;
RTC_DATA_ATTR float pressure_rtc;
RTC_DATA_ATTR float altitude_rtc;
RTC_DATA_ATTR float humidity_rtc; 
//Variable RTC for verification if the display turn off after de time of display
RTC_DATA_ATTR boolean cut_off_display;
RTC_DATA_ATTR uint32_t time_init_deep_sleep;  
  
void setup()
{
  setCpuFrequencyMhz(80);//Frequency of CPU
  Serial.begin(9600);
  if(esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TOUCHPAD){
  cut_off_display = true;
  show_in_display(time_on_display,temperature_rtc,pressure_rtc, altitude_rtc, humidity_rtc);
  
  }

  if(esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER && cut_off_display == 1){
    Serial.println("Entrou"+String(cut_off_display));
    turn_off_display();
    cut_off_display = false; 
    //RTC_DS3231 hour;
    //DateTime now =  hour.now();
    DateTime now;
    Serial.println("tempo antigo: "+String(time_init_deep_sleep));
    Serial.println("tempo novo: "+String(now.second()));
    Serial.println("Diferença: "+String(time_init_deep_sleep - now.second()));
    hibernation_sleep((time_init_deep_sleep - now.second())/60); 
  }
  //verify sensor connection
 BME280_status();

  // connect to the WiFi
  WiFi.mode(WIFI_STA); // WiFi station mode
  WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
  delay(1000);
    if (WiFi.status() != WL_CONNECTED){
      Serial.printf("NO Connect to the WiFi");
    }else{
      Serial.println("Connect to the WiFi");
      Serial.printf("IP Address: ");
      Serial.println(WiFi.localIP());
    }
 //conecting to redis server
  redisStart(&deep_sleep_time, &time_beetwen_samples, &time_on_display, &number_samples_mean);
}

void submit_for_redis(void){
  BME280_read_mean(number_samples_mean, &temperature_rtc, &pressure_rtc, &altitude_rtc, &humidity_rtc);
  
  redis_update_BME280_data(temperature_rtc ,pressure_rtc, altitude_rtc, humidity_rtc);

  BME280_Sleep(0x76);
}

void loop() {
  
  print_wakeup_reason();
  print_wakeup_touchpad();

  submit_for_redis();//Submit the informations for the Redis
  //RTC_DS3231 hour;
  //DateTime now =  hour.now();
  DateTime now;
  time_init_deep_sleep = now.unixtime();

  hibernation_sleep(deep_sleep_time);

}

