#ifndef _DHTADAPTER_H_
#define _DHTADAPTER_H_

#include <ArduinoJson.h>
#include <DHT.h>

#include "SensorChainedAdapterBase.h"

class DHTAdapter : public SensorChainedAdapterBase {
public:
    DHTAdapter(uint8_t pin, uint8_t type, const char* suffix) : mSensor(pin, type), SensorChainedAdapterBase("DHT", suffix)  {}

    bool begin() override;

    bool save(JsonObject* json) override;

private:
    DHT mSensor;
};


#endif
