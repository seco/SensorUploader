#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <FS.h>

#include "Config.h"
#include "SensorChainedAdapterBase.h"
#include "DHTAdapter.h"
#include "BMPAdapter.h"
#include "TimeSyncronizer.h"

#define RSTPIN 13     // what pin we're connected to

#define SWITCH 5     // what pin we're connected to
#define DHTPIN 4     // what pin we're connected to
#define SDAPIN 12     // what pin we're connected to
#define SCLPIN 14     // what pin we're connected to

// Uncomment whatever type you're using!
#define DHTTYPE DHT22   // DHT 22  (AM2302)


SensorChainedAdapterBase* adapterChain;

unsigned long volatile time1;

Client* pwclient;
PubSubClient client;


Config config;
String prefix;



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
            
            StaticJsonBuffer<2048> jsonBuffer;
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

            if (root.containsKey("mignfingerprint"))
                config.mignfingerprint = root["mignfingerprint"].as<bool>();

            if (root.containsKey("mfingerprint"))              
                strncpy(&config.mfingerprint[0], (const char *)root["mfingerprint"], 129);

            if (root.containsKey("mswitchtopic"))
                strncpy(&config.mswitchtopic[0], (const char *)root["mswitchtopic"], 65);

            if (root.containsKey("mswitchmsg"))
                strncpy(&config.mswitchmsg[0], (const char *)root["mswitchmsg"], 65);

            if (root.containsKey("mstatustopic"))
                strncpy(&config.mstatustopic[0], (const char *)root["mstatustopic"], 65);

            if (root.containsKey("mstatusmsg"))
                strncpy(&config.mstatusmsg[0], (const char *)root["mstatusmsg"], 65);

            Serial.print("ssid: ");Serial.println(config.ssid);
            Serial.print("password: ");Serial.println(config.password);
            Serial.print("localhost: ");Serial.println(config.localhost);
            
            Serial.print("mserver: ");Serial.println(config.mserver);
            Serial.print("muser: ");Serial.println(config.muser);
            Serial.print("mpassword: ");Serial.println(config.mpassword);
            Serial.print("mport: ");Serial.println(config.mport);
            
            Serial.print("mprefix: ");Serial.println(config.mprefix);
            
            Serial.print("mnjson: ");Serial.println(config.mnjson);
            Serial.print("minterval: ");Serial.println(config.minterval);
            
            Serial.print("mfingerprint: ");Serial.println(config.mfingerprint);

            Serial.print("mignfingerprint: ");Serial.println(config.mignfingerprint);
            
            Serial.print("mswitchtopic: ");Serial.println(config.mswitchtopic);
            Serial.print("mswitchmsg: ");Serial.println(config.mswitchmsg);

            Serial.print("mstatustopic: ");Serial.println(config.mstatustopic);
            Serial.print("mstatusmsg: ");Serial.println(config.mstatusmsg);


            SPIFFS.remove("/config.bin");
            
            optimistic_yield(10);

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
    
            yield();
            
            if (bufc.length() >= 1024)
                return false;
        }
    }

    return false;
}


void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived ");
    /*Serial.print(topic);
    Serial.print("[");
    Serial.print(length);
    Serial.print("] = ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
*/
    boolean on = (length == strlen(config.mswitchmsg) && !memcmp(&config.mswitchmsg[0], payload, length));

    Serial.println(on);

    digitalWrite(SWITCH, on); 
}


void systemRestart() {
    optimistic_yield(50);
    pinMode(RSTPIN, OUTPUT); 
    digitalWrite(RSTPIN, LOW); 
}

void setup() {
    Serial.begin(115200);

    SPIFFS.begin();

    pinMode(SWITCH, OUTPUT); 

    optimistic_yield(100);

    File configFile = SPIFFS.open("/config.bin", "r");
    
    if (!configFile) {
        Serial.println("Waiting for configuration (no file)");
        while (true) {
            if (readConfigurationFromSerial())
            {
                Serial.println("Resetting to update configuration");
                systemRestart();
            }
            optimistic_yield(100);
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
            
         optimistic_yield(1000);
    }
    
    WiFi.begin(config.ssid, config.password);
    WiFi.hostname(config.localhost);

    Serial.print("Connecting to ");
    Serial.print(config.ssid);


    prefix = String(config.mprefix[0] == 0 ? config.localhost : config.mprefix);
    
    while (WiFi.status() != WL_CONNECTED)
    {
        if (readConfigurationFromSerial())
        {
            Serial.println("Resetting to update configuration");
            systemRestart();
        }
        
        Serial.print(".");
        optimistic_yield(1000);
    }

    Serial.println();

    if (!MDNS.begin(config.localhost))
        Serial.println("Error setting up MDNS responder!");

  
    if (config.mserver[0]){
        if (config.mport == 0)
            config.mport = 1883;

        if (config.minterval == 0)
            config.minterval = 10000;

        if (!config.mswitchtopic[0]){
            strncpy(&config.mswitchtopic[0],(prefix + "/switch").c_str(), 65);
        }
    
        if (!config.mswitchmsg[0]){
            strncpy(&config.mswitchmsg[0], "on", 65);
        }


        if (!config.mstatustopic[0]){
            strncpy(&config.mstatustopic[0],(prefix + "/status").c_str(), 65);
        }
    
        if (!config.mstatusmsg[0]){
            strncpy(&config.mstatusmsg[0], "all", 65);
        }
    
        
        client.setServer(config.mserver, config.mport);

        pwclient = config.mfingerprint[0] ? new WiFiClientSecure() : new WiFiClient();
        client.setClient(*pwclient);
        client.setCallback(callback);
    } 

    time1 = millis();
}


void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("disconnected!");
        optimistic_yield(3000);
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
        
        bool connected = false;
           
        if (config.mserver[0]) {
            connected = client.connected();
            
            if (!connected) {
                if (client.connect(config.localhost, config.muser, config.mpassword)) {
                     connected = !config.mfingerprint[0] || ((WiFiClientSecure *)pwclient)->verify( config.mfingerprint, config.mserver);

                     if (!connected) {
                         Serial.print("fingerprint doesn't match: ");
                         Serial.print(config.mfingerprint);
                         Serial.print(", host: ");
                         Serial.println(config.mserver);
    
                         if (config.mignfingerprint) {
                            connected = true;
                         } else {
                            client.disconnect();
                         }
                     } else {
                         Serial.println("reconnected");
                     }
    
                     if (connected && config.mswitchtopic[0]) {
                         if (!client.subscribe(config.mswitchtopic)) {
                            Serial.println("subscription failed");
                         } else {
                            Serial.println("subscription succeded");
                            client.loop();
                            client.publish(config.mstatustopic, config.mstatusmsg);
                         }
                     }
                }
            }
        }

        
        StaticJsonBuffer<200> jsonBuffer;
        JsonObject& json = jsonBuffer.createObject();
        adapterChain->saveAll(&json);


        if (connected) {
            if (config.mnjson) {
                for (JsonObject::iterator it=json.begin(); it!=json.end(); ++it)  {
                    client.publish((prefix + "/" + it->key).c_str(), it->value.as<String>().c_str());
                    Serial.println(prefix + "/" + it->key);
                }
            } else {
                char buffer[json.measureLength() + 1];
                json.printTo(buffer, sizeof(buffer));
                client.publish((prefix + "/data").c_str(), buffer);
                Serial.println(prefix + "/data");
            }
        } else {
            json.printTo(Serial);
            Serial.println();
        }


    }

   
    client.loop();
    
    if (readConfigurationFromSerial())
    {
        Serial.println("Resetting to update configuration");
        systemRestart();
    }
}
