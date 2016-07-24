#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <FS.h>

#include <ESP8266httpUpdate.h>

#include "UpdateAdapter.h"
#include "Config.h"
#include "Utils.h"
#include "TaskScheduler.h"
#include "SensorChainedAdapterBase.h"
#include "DHTAdapter.h"
#include "BMPAdapter.h"
#include "ButtonAdapter.h"
#include "RelayAdapter.h"
#include "TimeSyncronizer.h"



#define SWITCH 4     // what pin we're connected to
#define BTNPIN 5     // what pin we're connected to
#define SDAPIN 12     // what pin we're connected to
#define SCLPIN 14     // what pin we're connected to
#define DHTPIN 13     // what pin we're connected to

// Uncomment whatever type you're using!
#define DHTTYPE DHT22   // DHT 22  (AM2302)



SensorChainedAdapterBase* adapterChain;

TaskScheduler task;

Client* pwclient;
PubSubClient client;


Config config;
String prefix;


char serialbuf[2048 + 1];

bool readConfigurationFromSerial()
{
	uint16_t serialpos = strlen(serialbuf);

	while(Serial.available() > 0) {
		char t = Serial.read();

		if (t == '\r' || t == '\n')
		{
			if (!serialpos)
				return false;

			StaticJsonBuffer<1024> jsonInBuffer;

			bool res = false;

			JsonObject& rootIn = jsonInBuffer.parseObject(serialbuf);

			if (!rootIn.success()) {
				Serial.println("Parse Error");
			} else {

				StaticJsonBuffer<1024> jsonOutBuffer;

				JsonObject* rootOut = NULL;
				char* buffer = NULL;

				File configInFile = SPIFFS.open("/config.json", "r");

				if (configInFile)
				{
					if (configInFile.size() <= 2048)
					{
						buffer = (char *)(calloc(configInFile.size() + 1, 1));

						if (buffer) 
						{
							configInFile.readBytes(buffer, configInFile.size());
							rootOut = &jsonOutBuffer.parseObject(buffer);
						}
					}

					configInFile.close();
				}

				if (!rootOut || !rootOut->success())
				{
					rootOut = &jsonOutBuffer.createObject();
				}


				for (JsonObject::iterator it=rootIn.begin(); it!=rootIn.end(); ++it)
				{
					if ((const char *)it->value || it->value.is<JsonArray&>()) 
					{
						rootOut->set(it->key, it->value);
					}
					else
					{
						rootOut->remove(it->key);
					}
				}

				SPIFFS.remove("/config.bin");

				SPIFFS.remove("/config.json");

				TaskScheduler::wait(10);

				File configFile = SPIFFS.open("/config.json", "w");

				res = configFile;

				if (!configFile) {
					Serial.println("Failed to open config file for writing");
				} else {

					rootOut->printTo(configFile);

					rootOut->printTo(Serial);
					Serial.println();

					configFile.close();
				}


				free(buffer);
			}

			memset(&serialbuf, 0, sizeof(serialbuf));

			return res;
		}
		else
		{
			serialbuf[serialpos++] = t;

			yield();

			if (serialpos >= sizeof(serialbuf) - 1)
			{
				memset(&serialbuf, 0, sizeof(serialbuf));

				return false;
			}
		}
	}

	return false;
}

void callback(char* topic, byte* payload, unsigned int length) {

	char payloads[1 + length];
	memset(payloads, 0, 1 + length);
	memcpy(payloads, payload, length);
	Serial.printf("message [%s, %s] arrived\r\n", topic, payloads);

	adapterChain->processAll(topic, payloads);
}


SensorChainedAdapterBase* createSensorAdapter(String prefix, String suffix, JsonObject& root)
{
	SensorChainedAdapterBase* res = NULL;

	if (root.containsKey("type")) 
	{
		int type = root["type"].as<int>();

		if (type > 0)
		{
			int pin1, pin2;


			const char* key1, key2, val1, val2;

			const char* suffixc = root["suffix"].asString();

			if (!suffixc && suffix)
			{
				suffixc = suffix.c_str();
			}

			switch (type)
			{
			case 1: 
				{
					pin1 = root["pin1"].as<int>();
					pin2 = root["pin2"].as<int>();

					if ((pin1 > 0 && pin2 > 0) || (pin1 == 0 && pin2 == 0))
					{
						if (pin1 == 0 && pin2 == 0)
						{
							pin1 = SDAPIN;
							pin2 = SCLPIN;
						}

						res = new BMPAdapter(pin1, pin2, suffixc);
					}

					break;
				}
			case 2: 
				{
					pin1 = root["pin1"].as<int>();

					if (pin1 == 0)
					{
						pin1 = DHTPIN;
					}
					res = new DHTAdapter(pin1, DHT22, suffixc);

					break;
				}
			case 3: 
				{
					pin1 = root["pin1"].as<int>();
					val1 = root["val1"].asString();
					val2 = root["val2"].asString();

					if (pin1 == 0)
					{
						pin1 = BTNPIN;
					}

					if (val1 == NULL)
					{
						val1 = "on";
					}

					if (val2 == NULL)
					{
						val2 = "off";
					}

					res = new ButtonAdapter(pin1, val1, val2, suffixc);

					break;
				}
			case 4: 
				{
					pin1 = root["pin1"].as<int>();

					key1 = root["key1"].asString();

					val1 = root["val1"].asString();
					val2 = root["val2"].asString();


					if (pin1 == 0)
					{
						pin1 = SWITCH;
					}

					if (key1 == NULL)
					{
						val1 = "switch";
					}

					if (val1 == NULL)
					{
						val1 = "on";
					}

					if (val2 == NULL)
					{
						val2 = "off";
					}

					res = new RelayAdapter(((prefix + "/") + key1).c_str(), pin1, val1, val2);

					break;
				}
			}
		}
	}
	return res;
}

void subscribeOne(const SensorChainedAdapterBase* adapter)
{
	if (adapter) 
	{
		const char* topic = adapter->topic();

		if (topic && topic[0])
		{
			if (!client.subscribe(topic)) 
			{
				Serial.printf("subscription [%s] failed\r\n", topic);
			} 
			else 
			{
				Serial.printf("subscription [%s] succeded\r\n", topic);
				client.loop();
			}
		}
	}
}



void readConfigurationFromSerialAndReset(int ms)
{
	if (readConfigurationFromSerial())
	{
		Serial.println("Resetting to update configuration");
		Utils::systemRestart();
	}

	if (ms > 500)
	{
		Serial.print(".");
	}

	TaskScheduler::wait(ms);
}

void readConfigurationFromFile(File configFile)
{
	if (configFile.size() <= 2048)
	{
		char* buffer = (char *)(calloc(configFile.size() + 1, 1));

		if (buffer)
		{
			configFile.readBytes(buffer, configFile.size());

			StaticJsonBuffer<1024> jsonBuffer;
			JsonObject& root = jsonBuffer.parseObject(buffer);

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


			if (root.containsKey("mfingerprint"))
				strncpy(&config.mfingerprint[0], (const char *)root["mfingerprint"], 129);

			if (root.containsKey("mport")) {
				int port = root["mport"].as<int>();

				if (port <= 0 || port >= 65535)
					port = (config.mfingerprint[0]) ? 8883 : 1883;

				config.mport = port;
			} else {
				config.mport = (config.mfingerprint[0]) ? 8883 : 1883;
			}

			if (root.containsKey("minterval")) {
				int interval = root["minterval"].as<int>();

				if (interval <= 0 || interval >= 3600 * 1000)
					interval = 10000;

				config.minterval = interval;
			}

			if (root.containsKey("mignfingerprint"))
				config.mignfingerprint = root["mignfingerprint"].as<bool>();

			if (root.containsKey("debug"))
				config.debug = root["debug"].as<bool>();

			prefix = String(config.mprefix[0] == 0 ? config.localhost : config.mprefix);

			if (prefix.length())
			{
				adapterChain =  new UpdateAdapter((prefix + "/update").c_str());

				if (root.containsKey("sensors"))
				{
					JsonArray& arr = root["sensors"].asArray();

					if (arr.size()) 
					{
						SensorChainedAdapterBase* padapter = adapterChain;
						char suffix[16];

						for (int i = 0; i < arr.size(); i++)
						{
							SensorChainedAdapterBase* adapter = createSensorAdapter(prefix, itoa(i + 1, &suffix[0], sizeof(suffix)), arr[i]);

							if (adapter)
							{
								padapter = padapter->next(adapter);
							}
						}
					}
				}
			}

			free(buffer);
		}
	}
}

void setup() {
	Serial.begin(115200);

	SPIFFS.begin();

	WiFi.disconnect(1);

	TaskScheduler::wait(1000);

	SPIFFS.remove("/config.bin");

	File configFile = SPIFFS.open("/config.json", "r");

	if (!configFile || configFile.size() > 2048) 
	{
		if (configFile)
			configFile.close();

		Serial.println("Waiting for configuration (no file or too large)");
		while (true) 
		{
			readConfigurationFromSerialAndReset(100);
		}
	} 
	else 
	{

		readConfigurationFromFile(configFile);

		configFile.close();

		if (adapterChain) 
		{
			Serial.println("Verifying sensors");

			int cnt = 0;
			while (!adapterChain->beginAll() && cnt++ < 5) 
			{
				readConfigurationFromSerialAndReset(1000);
			}
		}
	}

	Serial.println();

	if (config.localhost[0])
	{
		WiFi.hostname(config.localhost);
	}

	if (config.ssid[0])
	{
		WiFi.begin(config.ssid, config.password);

		Serial.printf("Connecting to %s\r\n", config.ssid);

		TaskScheduler::wait(1000);

		while (!WiFi.isConnected())
		{
			readConfigurationFromSerialAndReset(1000);
		}
	} 
	else 
	{
		Serial.println("Waiting for configuration (no wifi configuration)");
		while (true) 
		{
			readConfigurationFromSerialAndReset(1000);
		}
	}

	Serial.println();

	if (config.localhost[0] && !MDNS.begin(config.localhost))
	{
		Serial.println("Error setting up MDNS responder!");
	}

	if (config.minterval <= 0 || config.minterval >= 3600 * 1000)
	{
		config.minterval = 10000;
	}

	pwclient = config.mfingerprint[0] || config.mignfingerprint ? new WiFiClientSecure() : new WiFiClient();

	client.setClient(*pwclient);
	client.setCallback(callback);

	task.setInterval(config.minterval).start();
}




void loop() 
{
	if (!WiFi.isConnected()) 
	{
		Serial.println("WiFi disconnected!");
		TaskScheduler::wait(3000);
		Utils::systemRestart();
	}

	if (adapterChain && (task.loop() || adapterChain->checkAll())) 
	{
		bool connected = false;

		if (config.mserver[0]) 
		{
			connected = client.connected();

			if (!connected) 
			{
				if (client.connect(config.localhost, config.muser, config.mpassword)) 
				{
					connected = !config.mfingerprint[0] || ((WiFiClientSecure *)pwclient)->verify( config.mfingerprint, config.mserver);

					if (!connected) 
					{
						Serial.printf("Fingerprint doesn't match: %s, host: %s\r\n", config.mfingerprint, config.mserver);

						if (config.mignfingerprint) 
						{
							connected = true;
						}
						else 
						{
							client.disconnect();
						}
					}
					else 
					{
						Serial.println("reconnected");
					}

					if (connected)
					{ 
						adapterChain->applyAll(&subscribeOne);
					}
				}
			}
		}


		StaticJsonBuffer<200> jsonBuffer;
		JsonObject& json = jsonBuffer.createObject();
		adapterChain->saveAll(&json);

		if (connected) 
		{
			if (config.mnjson) 
			{
				for (JsonObject::iterator it=json.begin(); it!=json.end(); ++it)  
				{
					client.publish((prefix + "/" + it->key).c_str(), it->value.as<String>().c_str());
					Serial.println(prefix + "/" + it->key);
				}
			} 
			else 
			{
				char buffer[json.measureLength() + 1];
				json.printTo(buffer, sizeof(buffer));
				client.publish((prefix + "/data").c_str(), buffer);
				Serial.println(prefix + "/data");
			}
		}

		if (!connected || config.debug)
		{
			json.printTo(Serial);
			Serial.println();
		}

	}


	client.loop();
	readConfigurationFromSerialAndReset(100);
}
