
#include "display.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


boolean show_in_display(float time_on_display, float temperature_rtc,float pressure_rtc, float altitude_rtc,float humidity_rtc){

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
  display.display();
  Serial.println("Entrou...");
  Serial.println("Time:"+ String(time_on_display));
  //cut_off_display = true;
  hibernation_sleep(time_on_display/60000);
  return true;
  }
}


void turn_off_display(){
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
      Serial.println("SSD1306 allocation failed");
    }else{
      display.ssd1306_command(SSD1306_DISPLAYOFF);
    } 
}

