#ifndef _SENSORCHAINEDADAPTERBASE_H_INCLUDED
#define _SENSORCHAINEDADAPTERBASE_H_INCLUDED

#include <ArduinoJson.h>

class SensorChainedAdapterBase {
public:
    SensorChainedAdapterBase(const char* name, const char* suffix) : mName(name), mSuffix(suffix), pNext(NULL)  {};

    virtual const char* name() const { return mName; };

    virtual bool begin() { return false; };
    virtual void end() {};
    virtual bool save(JsonObject*) { return true; };


    virtual bool beginAll();
    virtual void endAll();
    virtual bool saveAll(JsonObject*);

    virtual ~SensorChainedAdapterBase() { end(); }

    SensorChainedAdapterBase* next() { return pNext; }
    SensorChainedAdapterBase* next(SensorChainedAdapterBase* pVal) { pNext = pVal; return next(); };
protected:
    SensorChainedAdapterBase* pNext;
    const char* mName;
    const char* mSuffix;
};


#endif
