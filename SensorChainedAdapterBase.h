#ifndef _SENSORCHAINEDADAPTERBASE_H_INCLUDED
#define _SENSORCHAINEDADAPTERBASE_H_INCLUDED

#include <ArduinoJson.h>

#ifdef ESP8266
#include <functional>
#define SENSOR_CALLBACK_SIGNATURE std::function<void(const SensorChainedAdapterBase*)> callback
#else
#define SENSOR_CALLBACK_SIGNATURE void (*callback)(const SensorChainedAdapterBase*)
#endif

class SensorChainedAdapterBase {
public:
    SensorChainedAdapterBase(const char* name, const char* suffix, const char* topic = NULL) : mName(name), mSuffix(suffix), mTopic(topic), pNext(NULL)  {};

	virtual const char* name() const { return mName.c_str(); };

	virtual const char* topic() const { return mTopic.c_str(); };

    virtual bool check() { return false; };
    virtual bool begin() { return true; };
    virtual void end() {};
	virtual bool process(char* topic, char* payload) { return false; };
    virtual bool save(JsonObject*) { return true; };

	virtual void applyAll(SENSOR_CALLBACK_SIGNATURE);

    virtual bool beginAll();
    virtual void endAll();
    virtual bool saveAll(JsonObject*);
    virtual bool checkAll();
	virtual bool processAll(char* topic, char* payload);

    virtual ~SensorChainedAdapterBase() { end(); }

    SensorChainedAdapterBase* next() { return pNext; }
    SensorChainedAdapterBase* next(SensorChainedAdapterBase* pVal) { pNext = pVal; return next(); };
protected:
    SensorChainedAdapterBase* pNext;
    const String mName;
	const String mTopic;
    const String mSuffix;
};


#endif
