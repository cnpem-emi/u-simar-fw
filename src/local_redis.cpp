#include "local_redis.h"     
#include <Redis.h>           //Biblioteca do Redis
#include <HTTPClient.h>      //Biblioteca para realizar requisições por HTTP
#include <ESP32httpUpdate.h> //Biblioteca que possibilita OTA por checagem HTTP
//Variavels de conexão com o Redis 
WiFiClient redisConn;
// Variavel Redis 
Redis redis(redisConn);

//Variavel que armazena a versão do firmware indicada no Redis
int version_in_Redis;
// Colocar a versão atual do firmware para a comparação
int versionCurrent = 2;

/// @brief Função que estabelece a conexão com o Redis
/// @param deep_sleep_time Ponteiro que recebe o tempo de hibernação do sistema  
/// @param time_beetwen_samples Ponteiro que recebe o empo entre as amostras para a média
/// @param time_on_display Ponteiro que recebe o tempo de exibição do display
/// @param number_samples_mean Ponteiro que recebe o numero de amostras para o calculo da média 
void redisStart(float *deep_sleep_time, int *time_beetwen_samples, float *time_on_display, int *number_samples_mean)
{

    // conecting to redis server

    if (!redisConn.connect(REDIS_ADDR, REDIS_PORT))//Testa se a conexão Redis com os parametros pré definidos 
    {
        if (!redisConn.connect("10.0.38.42", REDIS_PORT))//tenta a conexão com um novo IP
        {
            if (!redisConn.connect("10.0.38.59", REDIS_PORT))//tenta a conexão com um novo IP
            {//Caso não haja conexão inicia-se as variaveis com valores padrões 
                Serial.println("Failed to connect to the Redis server!");
                *deep_sleep_time = DEEP_SLEEP_TIME_DEFAUT;
                *time_beetwen_samples = SAMPLE_TIME;
                *time_on_display = TIME_ON_DISPLAY_DEFAUT;
                *number_samples_mean = NUMBER_SAMPLE_MEAN_DAFAUT;
                return;
            }
        }
    }

    auto connRet = redis.authenticate(REDIS_PASSWORD);//Autenticação no Redis 
    if (connRet == RedisSuccess)//Se a conexão for estabelecida com sucesso
    {
        Serial.println("Connected to the Redis server!");

        String time = redis.get("uSIMAR:Testes:Sleep-SP");//Le o tempo de hibernação do dispositivo 
        Serial.println("Time:" + time);
        *deep_sleep_time = time.toFloat();// Converte o tempo de String para float e salva no ponteiro

        // String sample = redis.get("uSIMAR:Testes:Sleep-SP");
        // Serial.println("Sample Beetwen samples:"+sample);
        // *time_beetwen_samples = sample.toInt();

        String tdisplay = redis.get("uSIMAR:Testes:Time_Display-SP");//Le o tempo de funcionamento do display
        Serial.println("Time of display:" + tdisplay);
        *time_on_display = tdisplay.toInt();//Converte o tempo do display de String para Int e salva no ponteiro

        String nmean = redis.get("uSIMAR:Testes:Number_Sample-SP");//Le o numero de amostra para a média
        Serial.println("Number of samples for mean:" + nmean);
        *number_samples_mean = nmean.toInt();//Converte o numero de amostra de String para Int e salva no ponteiro

        String v = redis.get("uSIMAR:Testes:Version-SP");//Le a Versão atual registrada no Redis
        Serial.println("Version in Redis:" + v);
        version_in_Redis = v.toInt();//Converte o versão de String para Int e salva no ponteiro
        checkUpdate();
        Serial.println("!!!!!Nova Versão V"+String(v)+"!!!!!");
    }
    else
    {
        Serial.printf("Failed to authenticate to the Redis server! Errno: %d\n", (int)connRet);
        return;
    }
}


/// @brief Função especifica para atualização dos dados coletados pelo BME280
/// @param temperature_rtc //Recebe o valor da temperatura coletado pelo BME280
/// @param pressure_rtc //Recebe o valor da pressão coletado pelo BME280
/// @param altitude_rtc //Recebe o valor da altitude coletado pelo BME280
/// @param humidity_rtc //Recebe o valor da humidade coletado pelo BME280
void redis_update_BME280_data(float temperature_rtc, float pressure_rtc, float altitude_rtc, float humidity_rtc)
{
    // Variaveis lidas
    char *temperature;
    char *pressure;
    char *altitude;
    char *humidity;

    temperature = (char *)malloc(16);
    pressure = (char *)malloc(16);
    altitude = (char *)malloc(16);
    humidity = (char *)malloc(16);

    snprintf(temperature, 16, "%.2f", temperature_rtc);//Salva o valor da temperatura em String
    redis.set("uSIMAR:Testes:Temp-Mon", temperature);//Salva o valor na variavel no Redis
    Serial.println("uSIMAR:Testes:Temp-Mon:" + String(temperature));//Exibe no serial

    snprintf(pressure, 16, "%.2f", pressure_rtc);//Salva o valor da pressão em String
    redis.set("uSIMAR:Testes:Pressure-Mon", pressure);//Salva o valor na variavel no Redi
    Serial.println("uSIMAR:Testes:Pressure-Mon:" + String(pressure));//Exibe no serial

    snprintf(altitude, 16, "%.2f", altitude_rtc);//Salva o valor da altitude em String
    redis.set("uSIMAR:Testes:Altitude-Mon", altitude);//Salva o valor na variavel no Redi
    Serial.println("uSIMAR:Testes:Altitude-Mon:" + String(altitude));//Exibe no serial

    snprintf(humidity, 16, "%.2f", humidity_rtc);//Salva o valor da humidade em String
    redis.set("uSIMAR:Testes:Humidity-Mon", humidity);//Salva o valor na variavel no Redi
    Serial.println("uSIMAR:Testes:Humidity-Mon:" + String(humidity));//Exibe no serial
}

void checkUpdate()
{
    // Define timeout
    int timeout = 4000;

    if (versionCurrent != version_in_Redis) // verifica se as verões são diferentes
    {

        // Inicializa conexão por HTTP para acessar o firmware no servidor
        HTTPClient http;
        http.begin("https://github.com/cnpem-sei/u-simar-fw/tree/organizado/.pio/build/esp32doit-devkit-v1/firmware.bin");
        http.setConnectTimeout(timeout);
        http.setTimeout(timeout);
        int resCode = http.GET();
        Serial.println("Version: " + String(versionCurrent));
        if (resCode > 0)
        {
            // Se a versão for diferente da atual neste código, inicializa a atualização
            updateOTA();
        }
    }
}

/// @brief Função de atualização do firmware via OTA
void updateOTA()
{

    // Se houver conexão WiFi
    if ((WiFi.status() == WL_CONNECTED))
    {

        // Realiza o download do firmware (.bin) e realiza a atualização
        t_httpUpdate_return ret = ESPhttpUpdate.update("https://raw.githubusercontent.com/cnpem-sei/u-simar-fw/organizado/.pio/build/esp32doit-devkit-v1/firmware.bin");

        // Switch para tratamento da resposta
        switch (ret)
        {
        case HTTP_UPDATE_FAILED: // Falha
            Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
            break;

        case HTTP_UPDATE_NO_UPDATES: // Ausência do arquivo de firmware
            Serial.println("HTTP_UPDATE_NO_UPDATES");
            break;

        case HTTP_UPDATE_OK: // Sucesso
            Serial.println("HTTP_UPDATE_OK");
            versionCurrent = version_in_Redis;
            break;
        }
    }
}