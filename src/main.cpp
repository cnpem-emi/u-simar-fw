#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BME280.h>
#include <WiFi.h>
#include <Redis.h>
#include "driver/adc.h"
#include <esp_wifi.h>
#include <esp_bt.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino resetpin)
#define SEALEVELPRESSURE_HPA (1013.25)
//declaration for Wifi connection
#define WIFI_SSID "Devices"
#define WIFI_PASSWORD ""
//Declaration for Redis Server
#define REDIS_ADDR "10.0.38.46"
#define REDIS_PORT 6379
#define REDIS_PASSWORD ""
//Time of sleep defaut
#define DEEP_SLEEP_TIME_DEFAUT 0.5//Unit in minutes
//Time between samples  
#define SAMPLE_TIME 1000//Unit in miliseconds
#define TIME_ON_DISPLAY_DEFAUT  5000//Unit in miliseconds
#define NUMBER_SAMPLE_MEAN_DAFAUT 10//Number of samples for mean

#define Threshold 40 /* Greater the value, more the sensitivity */
touch_pad_t touchPin;

RTC_DATA_ATTR float deep_sleep_time;
int time_beetwen_samples;
RTC_DATA_ATTR float time_on_display;
int number_samples_mean;

//Variable of Display 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
//Variable of Sensor (Temperature, pressure and Humidity)
Adafruit_BME280 bme;
unsigned long delayTime;
WiFiClient redisConn;
//Variable Redis
Redis redis(redisConn);

//Mensure variables 
char *temperature;
char *pressure;
char *altitude;
char *humidity; 

RTC_DATA_ATTR float temperature_rtc;
RTC_DATA_ATTR float pressure_rtc;
RTC_DATA_ATTR float altitude_rtc;
RTC_DATA_ATTR float humidity_rtc; 

RTC_DATA_ATTR boolean cut_off_display;


void show_in_display(void);
void hibernation_sleep(float time_deep_sleep);

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
  //placeholder callback function
}

  
void setup()
{
  setCpuFrequencyMhz(80);//Frequency of CPU
  Serial.begin(9600);
  if(esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TOUCHPAD){
    show_in_display();
  }
  if(esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER && cut_off_display == 1){
    Serial.println("Entrou"+String(cut_off_display));
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
      Serial.println("SSD1306 allocation failed");
    }else{
      display.ssd1306_command(SSD1306_DISPLAYOFF);
    } 
    cut_off_display = false; 
    hibernation_sleep(deep_sleep_time);
    
  }
  bool status;

  temperature = (char*)malloc(16);
  pressure = (char*)malloc(16);
  altitude = (char*)malloc(16);
  humidity = (char*)malloc(16);

  
  //verify sensor connection
  status = bme.begin(0x76);
  if(!status){
    Serial.println("Sensor BMx280 not found");
    while(1);
  }
  

  // connect to the WiFi
  WiFi.mode(WIFI_STA); // WiFi station mode
  WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
  delay(1000);
    if (WiFi.status() != WL_CONNECTED){
      Serial.printf("NO Connect to the WiFi");
    }else{
      Serial.printf("Connect to the WiFi");
    }

  // while (WiFi.status() != WL_CONNECTED)
  //   {
  //     delay(250);
  //     Serial.printf(".");
  //   }
  Serial.println();
  Serial.printf("IP Address: ");
  Serial.println(WiFi.localIP());
  //delay(1000);

 //conecting to redis server

deep_sleep_time = DEEP_SLEEP_TIME_DEFAUT;
time_beetwen_samples = SAMPLE_TIME;
time_on_display = TIME_ON_DISPLAY_DEFAUT;
number_samples_mean = NUMBER_SAMPLE_MEAN_DAFAUT;
 
    if (!redisConn.connect(REDIS_ADDR, REDIS_PORT))
    {   
      if (!redisConn.connect("10.0.38.42", REDIS_PORT))
      {
        if (!redisConn.connect("10.0.38.59", REDIS_PORT))
        {
          Serial.println("Failed to connect to the Redis server!");
          return;
        }
      }    
    }

    auto connRet = redis.authenticate(REDIS_PASSWORD);
    if (connRet == RedisSuccess)
    {
        Serial.println("Connected to the Redis server!");

        String time = redis.get("uSIMAR:Testes:Sleep-SP");
        Serial.println("Time:"+time);
        deep_sleep_time = time.toFloat();

        // String sample = redis.get("uSIMAR:Testes:Sleep-SP");
        // Serial.println("Sample Beetwen samples:"+sample);
        // time_beetwen_samples = sample.toInt();

        String tdisplay = redis.get("uSIMAR:Testes:Time_Display-SP");
        Serial.println("Time of display:"+tdisplay);
        time_on_display = tdisplay.toInt();

        String nmean = redis.get("uSIMAR:Testes:Number_Sample-SP");
        Serial.println("Number of samples for mean:"+nmean);
        number_samples_mean = nmean.toInt();

    }
    else
    {
        Serial.printf("Failed to authenticate to the Redis server! Errno: %d\n", (int)connRet);
        return;
    }
    
    

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

//hibernation function.
void hibernation_sleep(float time_deep_sleep){
    Serial.println("Going to sleep...");
    Serial.println("Tempo"+String(time_deep_sleep));
    //Sensor Sleeping
    //display.clearDisplay();
    //display.ssd1306_command(SSD1306_DISPLAYOFF);

    //disable resources of esp
    //esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH,   ESP_PD_OPTION_OFF);
    //esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
    //esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL,         ESP_PD_OPTION_OFF);
    //esp_sleep_pd_config(ESP_PD_DOMAIN_CPU,   ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC8M, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_VDDSDIO, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_MAX,         ESP_PD_OPTION_OFF);
    
    //diseble the sources of wake up
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    /* esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_EXT1);
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TOUCHPAD);
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ULP);
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_GPIO);
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_UART);
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_WIFI);
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_COCPU);
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_COCPU_TRAP_TRIG);
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_BT);
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_UNDEFINED);
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER); */
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
    // Go to sleep! Zzzz
    esp_deep_sleep_start();

}


void show_in_display(void){
  //esp_light_sleep_start();
  // Verify display connection
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
  }else{
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.printf("Temperat. %.2f%cC\n",temperature_rtc,248);
  display.printf("Pressure %.2fPa\n",pressure_rtc);
  display.printf("Altitude %.2fm\n",altitude_rtc);
  display.printf("Humidity %.2f%%",humidity_rtc);
  // display.printf("Temperat. %.2f%cC\n",bme.readTemperature(),248);
  // display.printf("Pressure %.2fPa\n",bme.readPressure()/100.0F);
  // display.printf("Altitude %.2fm\n",bme.readAltitude(SEALEVELPRESSURE_HPA));
  // display.printf("Humidity %.2f%%",bme.readHumidity());
  // BME280_Sleep(0x76);
  display.display();
  Serial.println("Entrou...");
  Serial.println("Time:"+ String(time_on_display));
  //delay(time_on_display-1);
  //Serial.println("Saiu...");
  //display.ssd1306_command(SSD1306_DISPLAYOFF);
  cut_off_display = true;
  hibernation_sleep(time_on_display/60000);
  

  }
}

void submit_for_redis(void){
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
  
  temperature_rtc = sum_temperature/number_samples_mean;
  snprintf(temperature,16,"%.2f",sum_temperature/number_samples_mean);
  redis.set("uSIMAR:Testes:Temp-Mon", temperature);
  Serial.println("uSIMAR:Testes:Temp-Mon:"+String(temperature));

  pressure_rtc = sum_pressure/number_samples_mean;
  snprintf(pressure,16,"%.2f",sum_pressure/number_samples_mean);
  redis.set("uSIMAR:Testes:Pressure-Mon", pressure);
  Serial.println("uSIMAR:Testes:Pressure-Mon:"+String(pressure));

  altitude_rtc = sum_altitude/number_samples_mean;
  snprintf(altitude,16,"%.2f",sum_altitude/number_samples_mean);
  redis.set("uSIMAR:Testes:Altitude-Mon", altitude);
  Serial.println("uSIMAR:Testes:Altitude-Mon:"+String(altitude));

  humidity_rtc = sum_humidity/number_samples_mean;
  snprintf(humidity,16,"%.2f",sum_humidity/number_samples_mean);
  redis.set("uSIMAR:Testes:Humidity-Mon", humidity);
  Serial.println("uSIMAR:Testes:Humidity-Mon:"+String(humidity));

  BME280_Sleep(0x76);
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


void loop() {
  
  print_wakeup_reason();
  print_wakeup_touchpad();


  
  
  if (redisConn.connect(REDIS_ADDR, REDIS_PORT))
  {
    submit_for_redis();//Submit the informations for the Redis
    if(deep_sleep_time > 0){

      hibernation_sleep(deep_sleep_time);

    }else if(deep_sleep_time == 0.0){

      delay(SAMPLE_TIME);

    }
  }else{

      hibernation_sleep(deep_sleep_time);

  }
    
}

