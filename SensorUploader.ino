#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>
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

unsigned long volatile time1;

WiFiClient wclient;
PubSubClient client(wclient);


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
                strncpy(&config.ssid[0],(const char *)root["ssid"], 33);
            
            if (root.containsKey("password"))
                strncpy(&config.password[0], (const char *)root["password"], 65);
            
            if (root.containsKey("localhost"))
                strncpy(&config.localhost[0], (const char *)root["localhost"], 129);
            
            if (root.containsKey("mserver"))              
                strncpy(&config.mserver[0], (const char *)root["mserver"], 129);

            if (root.containsKey("mpassword"))             
                strncpy(&config.mpassword[0], (const char *)root["mpassword"], 65);

            if (root.containsKey("muser"))
                strncpy(&config.muser[0], (const char *)root["muser"], 65);

            if (root.containsKey("mprefix"))              
                strncpy(&config.mprefix[0], (const char *)root["mprefix"], 129);

            if (root.containsKey("mnjson"))
                config.mnjson = root["mnjson"].as<bool>();

            if (root.containsKey("mport")) {
                int port = root["mport"].as<int>();

                if (port <= 0 || port >= 65535)
                    port = 1883;

                 config.mport = port;
            }

             if (root.containsKey("minterval")) {
                int interval = root["minterval"].as<int>();

                if (interval <= 0 || interval >= 3600 * 1000)
                    interval = 10000;

                 config.minterval = interval;
            }

            SPIFFS.remove("/config.bin");
            
            delay(10);

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


void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
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
        size_t size = std::min(configFile.size(), sizeof(Config));
        configFile.readBytes((char *)&config, size);            
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
    //Serial.print(":");
    //Serial.print(config.password);
    //Serial.print(" ");

    
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


    if (strlen(config.mserver)){
        if (config.mport == 0)
            config.mport = 1883;

        if (config.minterval == 0)
            config.minterval = 10000;

        client.setServer(config.mserver, config.mport);

        client.setCallback(callback);
    } 

    time1 = millis();
}


void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Disconnected!");
        delay(30000);
        systemRestart();
    }

    unsigned long time2 = millis();

    if (time2 < time1)
    {
        time1 = time2;
        return;
    }

    if (time2 - time1 > config.minterval) {
        
        time1  = time2;
        
        StaticJsonBuffer<200> jsonBuffer;
        JsonObject& json = jsonBuffer.createObject();
        adapterChain->saveAll(&json);
    
      
        
        if (strlen(config.mserver) && (client.connected() || client.connect(config.localhost, config.mpassword, config.mpassword))) {
            String prefix = String(config.mprefix[0] == 0 ? config.localhost : config.mprefix);
            
            if (config.mnjson) {
                for (JsonObject::iterator it=json.begin(); it!=json.end(); ++it)  {
                    client.publish((prefix + "/" + it->key).c_str(), it->value.asString());
                }
            } else {
                char buffer[json.measureLength() + 1];
                json.printTo(buffer, sizeof(buffer));
                client.publish((prefix + "/data").c_str(), buffer);
            }
        
        } else {
            json.printTo(Serial);
            Serial.println();
        }
    }
        

    if (client.connected())
      client.loop();
    
    delay(100);

    if (readConfigurationFromSerial())
    {
        Serial.println("Resetting to update configuration");
        systemRestart();
    }
}
