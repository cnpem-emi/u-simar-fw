#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BME280.h>
#include <WiFi.h>
#include <Redis.h>


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino resetpin)
#define SEALEVELPRESSURE_HPA (1013.25)
#define WIFI_SSID "Devices"
#define WIFI_PASSWORD ""
#define REDIS_ADDR "10.0.38.59"
#define REDIS_PORT 6379
#define REDIS_PASSWORD ""
#define uS_TO_S_FACTOR 1000000
#define TIME_TO_SLEEP 300

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_BME280 bme;
unsigned long delayTime;
WiFiClient redisConn;
Redis redis(redisConn);


char *temperature;
char *pressure;
char *altitude;
char *humidity;

RTC_DATA_ATTR int bootCount = 0;

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}
  
void setup()
{
  Serial.begin(9600);
  bool status;

  temperature = (char*)malloc(16);
  pressure = (char*)malloc(16);
  altitude = (char*)malloc(16);
  humidity = (char*)malloc(16);

  
  // Verify display connection
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for (;;); 
  }

  //Verify sensor connection 
  status = bme.begin(0x76);
  if(!status){
    Serial.println("Sensor BMx280 not found");
    while(1);
  }

  // Connect to the WiFi
  display.clearDisplay();
  WiFi.mode(WIFI_STA); // WiFi station mode
  WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
  Serial.println();
  Serial.printf("Connected to the WiFi");
  delay(1000);

  while (WiFi.status() != WL_CONNECTED)
    {
      delay(250);
      Serial.printf(".");
    }
  Serial.println();
  Serial.printf("IP Address: ");
  Serial.println(WiFi.localIP());
  delay(1000);
  display.clearDisplay();

 // Connecting to redis server
 
    if (!redisConn.connect(REDIS_ADDR, REDIS_PORT))
    {
        Serial.println("Failed to connect to the Redis server!");
        return;
    }

    auto connRet = redis.authenticate(REDIS_PASSWORD);
    if (connRet == RedisSuccess)
    {
        Serial.println("Connected to the Redis server!");
    }
    else
    {
        Serial.printf("Failed to authenticate to the Redis server! Errno: %d\n", (int)connRet);
        return;
    }

  }


void loop() {

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.setTextSize(1);

  display.printf("Temperat. %.2f%cC\n",bme.readTemperature(),248);
  snprintf(temperature,16,"%.2f",bme.readTemperature());
  redis.set("uSIMAR:Testes:Temp-Mon", temperature);

  display.printf("Pressure %.2fPa\n",bme.readPressure()/100.0F);
  snprintf(pressure,16,"%.2f",bme.readPressure()/100.0F);
  redis.set("uSIMAR:Testes:Pressure-Mon", pressure);

  display.printf("Altitude %.2fm\n",bme.readAltitude(SEALEVELPRESSURE_HPA));
  snprintf(altitude,16,"%.2f",bme.readAltitude(SEALEVELPRESSURE_HPA));
  redis.set("uSIMAR:Testes:Altitude-Mon", altitude);

  display.printf("Humidity %.2f%%",bme.readHumidity());
  snprintf(humidity,16,"%.2f",bme.readHumidity());
  redis.set("uSIMAR:Testes:Humidity-Mon", humidity);

  display.display();
  delay(2000);

  
  //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  //Print the wakeup reason for ESP32
  print_wakeup_reason();

  // Configure sleep time/hibernation 

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH,ESP_PD_OPTION_OFF);

  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
  Serial.println("Going to sleep now");
  
  esp_deep_sleep_start();
    
}