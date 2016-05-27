#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <FS.h>

#include "Config.h"
#include "SensorChainedAdapterBase.h"
#include "DHTAdapter.h"
#include "BMPAdapter.h"
#include "TimeSyncronizer.h"




#define DHTPIN 2     // what pin we're connected to
#define SDAPIN 12     // what pin we're connected to
#define SCLPIN 14     // what pin we're connected to

// Uncomment whatever type you're using!
#define DHTTYPE DHT22   // DHT 22  (AM2302)

SensorChainedAdapterBase* adapterChain;


WiFiClientSecure client;
Config config;

bool readConfigurationFromSerial()
{
    String bufc;
    
    while(Serial.available() > 0) {
        char t = Serial.read();
        
        if (t == '\r' || t == '\n')
        {

            if (!bufc.length())
                return false;

            //Serial.println(bufc);
            
            StaticJsonBuffer<1024> jsonBuffer;
            JsonObject& root = jsonBuffer.parseObject(bufc);            

            if (!root.success()) {
                Serial.println("Parse Error");
                return false;
            }

            //root.printTo(Serial);

            if (root.containsKey("ssid"))
                strncpy((char *)(const char *)root["ssid"], &config.ssid[0], 33);
            Serial.println(1);
            
            if (root.containsKey("password"))
                strncpy((char *)(const char *)root["password"], &config.password[0], 65);
            Serial.println(2);
            
            if (root.containsKey("localhost"))
                strncpy((char *)(const char *)root["localhost"], &config.localhost[0], 129);
            Serial.println(3);

            
            if (root.containsKey("mserver"))              
                strncpy((char *)(const char *)root["mserver"], &config.mserver[0], 129);

         Serial.println(4);
            if (root.containsKey("mpassword"))             
                strncpy((char *)(const char *)root["mpassword"], &config.mpassword[0], 65);
                     Serial.println(5);
            if (root.containsKey("muser"))
                strncpy((char *)(const char *)root["muser"], &config.muser[0], 65);
         Serial.println(5);
            
            if (root.containsKey("mport")) {
                int port = root["mport"].as<int>();

                if (port <= 0 || port >= 65535)
                    port = 1883;

                 config.mport = port;
            }
                 Serial.println(6);
            SPIFFS.remove("/config.bin");
            
            delay(10);
                     Serial.println(7);
            File configFile = SPIFFS.open("/config.bin", "w");

            
            
            if (!configFile) {
                Serial.println("Failed to open config file for writing");
                return false;
            }

            
            configFile.write((const uint8_t*)&config, sizeof(Config));
            
            configFile.close();
            
            return true;
        }
        else 
        {
            bufc += t;
    
            delay(1);
            
            if (bufc.length() >= 1024)
                return false;
        }
    }

    return false;
}


void systemRestart() {
    delay(50);
    pinMode(13, OUTPUT); 
    digitalWrite(13, LOW); 
}

void setup() {
    Serial.begin(115200);

    SPIFFS.begin();

    delay(100);

    File configFile = SPIFFS.open("/config.bin", "r");
    
    if (!configFile) {
        Serial.println("Waiting for configuration (no file)");
        while (true) {
            if (readConfigurationFromSerial())
            {
                Serial.println("Resetting to update configuration");
                systemRestart();
            }
            delay(100);
        }
    } else {
        size_t size = configFile.size();
        configFile.readBytes((char *)&config, sizeof(Config));            
        configFile.close();
    }

    
    

    adapterChain = new BMPAdapter(SDAPIN, SCLPIN, "1");

    int cnt = 10;
    
    while (cnt-- > 0 && !adapterChain->beginAll()) {
        Serial.println("Could not find a valid sensor, check wiring!");

        if (cnt == 0)
            systemRestart();
            
         delay(1000);
    }
    
    WiFi.begin(config.ssid, config.password);
    WiFi.hostname(config.localhost);

    Serial.print("Connecting to ");
    Serial.print(config.ssid);
    Serial.print(":");
    Serial.print(config.password);
    Serial.print(" ");

    
    while (WiFi.status() != WL_CONNECTED)
    {
        if (readConfigurationFromSerial())
        {
            Serial.println("Resetting to update configuration");
            systemRestart();
        }
        
        Serial.print(".");
        delay(1000);
    }

    Serial.println();

    if (!MDNS.begin(config.localhost))
        Serial.println("Error setting up MDNS responder!");

}

void loop() {
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    short cnt = 10;
    while (cnt-- > 0 && !adapterChain->saveAll(&json)) {
        Serial.println("Could not find a valid sensor, check wiring!");

        if (cnt == 0)
            systemRestart();
            
         delay(1000);
    }

    
    json.printTo(Serial);
    delay(500);
}
