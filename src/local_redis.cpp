#include "local_redis.h"
#include <Redis.h>
#include <HTTPClient.h> //Biblioteca para realizar requisições por HTTP
#include <ESP32httpUpdate.h> //Biblioteca que possibilita OTA por checagem HTTP


WiFiClient redisConn;
//Variable Redis
Redis redis(redisConn);

int version_in_Redis;
//Colocar a versão atual do firmware
int versionCurrent = 2;


void redisStart(float *deep_sleep_time,int *time_beetwen_samples, float *time_on_display,int *number_samples_mean){

    //conecting to redis server
 
    if (!redisConn.connect(REDIS_ADDR, REDIS_PORT))
    {   
      if (!redisConn.connect("10.0.38.42", REDIS_PORT))
      {
        if (!redisConn.connect("10.0.38.59", REDIS_PORT))
        {
            Serial.println("Failed to connect to the Redis server!");
            *deep_sleep_time = DEEP_SLEEP_TIME_DEFAUT;
            *time_beetwen_samples = SAMPLE_TIME;
            *time_on_display = TIME_ON_DISPLAY_DEFAUT;
            *number_samples_mean = NUMBER_SAMPLE_MEAN_DAFAUT;
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
        *deep_sleep_time = time.toFloat();

        // String sample = redis.get("uSIMAR:Testes:Sleep-SP");
        // Serial.println("Sample Beetwen samples:"+sample);
        // *time_beetwen_samples = sample.toInt();

        String tdisplay = redis.get("uSIMAR:Testes:Time_Display-SP");
        Serial.println("Time of display:"+tdisplay);
        *time_on_display = tdisplay.toInt();

        String nmean = redis.get("uSIMAR:Testes:Number_Sample-SP");
        Serial.println("Number of samples for mean:"+nmean);
        *number_samples_mean = nmean.toInt();

        String v = redis.get("uSIMAR:Testes:Version-SP");
        Serial.println("Version in Redis:"+v);
        version_in_Redis = v.toInt();
        checkUpdate();
        Serial.println("!!!!!Nova Versão V1 !!!!!");

    }
    else
    {
        Serial.printf("Failed to authenticate to the Redis server! Errno: %d\n", (int)connRet);
        return;
    }
    

}


void redis_update_BME280_data(float temperature_rtc,float pressure_rtc, float altitude_rtc,float humidity_rtc){
    //Mensure variables 
    char *temperature;
    char *pressure;
    char *altitude;
    char *humidity; 

    temperature = (char*)malloc(16);
    pressure = (char*)malloc(16);
    altitude = (char*)malloc(16);
    humidity = (char*)malloc(16);

    snprintf(temperature,16,"%.2f",temperature_rtc);
    redis.set("uSIMAR:Testes:Temp-Mon", temperature);
    Serial.println("uSIMAR:Testes:Temp-Mon:"+String(temperature));


    snprintf(pressure,16,"%.2f",pressure_rtc);
    redis.set("uSIMAR:Testes:Pressure-Mon", pressure);
    Serial.println("uSIMAR:Testes:Pressure-Mon:"+String(pressure));


    snprintf(altitude,16,"%.2f",altitude_rtc);
    redis.set("uSIMAR:Testes:Altitude-Mon", altitude);
    Serial.println("uSIMAR:Testes:Altitude-Mon:"+String(altitude));


    snprintf(humidity,16,"%.2f",humidity_rtc);
    redis.set("uSIMAR:Testes:Humidity-Mon", humidity);
    Serial.println("uSIMAR:Testes:Humidity-Mon:"+String(humidity));

}

void checkUpdate()
{
    //Define timeout
    int timeout = 4000;

    if (versionCurrent != version_in_Redis)//verifica se as verões são diferentes 
    {
    
    //Inicializa conexão por HTTP para acessar o firmware no servidor
    HTTPClient http;
    http.begin("https://github.com/cnpem-sei/u-simar-fw/tree/organizado/.pio/build/esp32doit-devkit-v1/firmware.bin");
    http.setConnectTimeout(timeout);
    http.setTimeout(timeout);
    int resCode = http.GET();
    Serial.println("Version: " + String(versionCurrent));
    if (resCode > 0)
    {         
            //Se a versão for diferente da atual neste código, inicializa a atualização
            updateOTA();
    }
    }
}

void updateOTA() //Função de atualização via OTA
{

    //Se houver conexão WiFi
    if ((WiFi.status() == WL_CONNECTED))
    {

        //Realiza o download do firmware (.bin) e realiza a atualização
        
        t_httpUpdate_return ret = ESPhttpUpdate.update("https://raw.githubusercontent.com/cnpem-sei/u-simar-fw/organizado/.pio/build/esp32doit-devkit-v1/firmware.bin");
        
        //Switch para tratamento da resposta
        switch (ret)
        {
        case HTTP_UPDATE_FAILED: //Falha
            Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
            break;

        case HTTP_UPDATE_NO_UPDATES: //Ausência do arquivo de firmware
            Serial.println("HTTP_UPDATE_NO_UPDATES");
            break;

        case HTTP_UPDATE_OK: //Sucesso
            Serial.println("HTTP_UPDATE_OK");
            versionCurrent = version_in_Redis;
            break;
        }
    }
}