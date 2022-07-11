#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BME280.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino resetpin)
#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Adafruit_BME280 bme;

unsigned long delayTime;

void setup()
{
  Serial.begin(9600);
  bool status;

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); 
  }

   //Sensor read address 0x76
  status = bme.begin(0x76);
  if(!status){
    Serial.println("Sensor BMx280 not found");
    while(1);
  }
  
  display.clearDisplay();
}

  void read_variables(){

    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.printf("Temperat. %.2f%cC\n",bme.readTemperature(),248);
    display.printf("Pressure %.2fPa\n",bme.readPressure()/100.0F);
    display.printf("Altitude %.2fm\n",bme.readAltitude(SEALEVELPRESSURE_HPA));
    display.printf("Humidity %.2fg.Kg-1",bme.readHumidity());
    display.display();
    delay(4000);
    display.clearDisplay();

  }

  void loop() {
    read_variables();
  }