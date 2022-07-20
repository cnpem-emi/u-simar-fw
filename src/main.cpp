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
#define REDIS_ADDR "10.0.6.106"
#define REDIS_PORT 6379
#define REDIS_PASSWORD ""

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_BME280 bme;
unsigned long delayTime;
WiFiClient redisConn;
Redis redis(redisConn);

char *temperature;
char *pressure;
char *altitude;
char *humidity;

  
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

  //verify sensor connection
  status = bme.begin(0x76);
  if(!status){
    Serial.println("Sensor BMx280 not found");
    while(1);
  }

  // connect to the WiFi
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

 //conecting to redis server
 
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

    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.setTextSize(1);

    display.printf("Temperat. %.2f%cC\n",bme.readTemperature(),248);
    snprintf(temperature,16,"%.2f",bme.readTemperature());
    redis.set("Temperature", temperature);

    display.printf("Pressure %.2fPa\n",bme.readPressure()/100.0F);
    snprintf(pressure,16,"%.2f",bme.readPressure()/100.0F);
    redis.set("Pressure", pressure);

    display.printf("Altitude %.2fm\n",bme.readAltitude(SEALEVELPRESSURE_HPA));
    snprintf(altitude,16,"%.2f",bme.readAltitude(SEALEVELPRESSURE_HPA));
    redis.set("Altitude", altitude);

    display.printf("Humidity %.2fg.Kg-1",bme.readHumidity());
    snprintf(humidity,16,"%.2f",bme.readHumidity());
    redis.set("Humidity", humidity);

    display.display();
    delay(4000);
    display.clearDisplay();
}