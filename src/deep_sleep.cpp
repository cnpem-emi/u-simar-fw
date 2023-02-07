#include "deep_sleep.h"
#include "driver/rtc_io.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/rtc.h"


touch_pad_t touchPin;

//hibernation function.
void hibernation_sleep(float time_deep_sleep){
    Serial.println("Going to sleep...");
    Serial.println("Tempo"+String(time_deep_sleep));

    //disable resources of esp
    //esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH,   ESP_PD_OPTION_OFF);
    //esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
    //esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
    //esp_sleep_pd_config(ESP_PD_DOMAIN_CPU,   ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL,         ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC8M, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_VDDSDIO, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_MAX,         ESP_PD_OPTION_OFF);
    
    //diseble the sources of wake up
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    // esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_EXT1);
    // esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TOUCHPAD);
    // esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ULP);
    // esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_GPIO);
    // esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_UART);
    // esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_WIFI);
    // esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_COCPU);
    // esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_COCPU_TRAP_TRIG);
    // esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_BT);
    // esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_UNDEFINED);
    // esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
    //esp_sleep_enable_ext0_wakeup(GPIO_NUM_32,1);
    
    //Setup interrupt on Touch Pad 3 (GPIO15)
    touchAttachInterrupt(T0, callback, Threshold);

    //Configure Touchpad as wakeup source
    esp_sleep_enable_touchpad_wakeup();
    
    //Disconnect Wifi
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    btStop();
    //power off adc 
    adc_power_off();
    //power off wifi - In defaut configuration of deep sleep this is configurate, but esp have bug historical 
    esp_wifi_stop();
    //power off bluetooth
    esp_bt_controller_disable();
    
    // Configure the timer to wake us up!
    esp_sleep_enable_timer_wakeup(time_deep_sleep * 60L * 1000000L);
    //set CPU frequency 
    setCpuFrequencyMhz(80);
    
    rtc_gpio_isolate(GPIO_NUM_2);

    // Go to sleep! Zzzz
    esp_deep_sleep_start();

    
}


void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

void print_wakeup_touchpad(){
  touchPin = esp_sleep_get_touchpad_wakeup_status();

  switch(touchPin)
  {
    case 0  : Serial.println("Touch detected on GPIO 4"); break;
    case 1  : Serial.println("Touch detected on GPIO 0"); break;
    case 2  : Serial.println("Touch detected on GPIO 2"); break;
    case 3  : Serial.println("Touch detected on GPIO 15"); break;
    case 4  : Serial.println("Touch detected on GPIO 13"); break;
    case 5  : Serial.println("Touch detected on GPIO 12"); break;
    case 6  : Serial.println("Touch detected on GPIO 14"); break;
    case 7  : Serial.println("Touch detected on GPIO 27"); break;
    case 8  : Serial.println("Touch detected on GPIO 33"); break;
    case 9  : Serial.println("Touch detected on GPIO 32"); break;
    default : Serial.println("Wakeup not by touchpad"); break;
  }
}

void callback(){
  delay(2000);
}