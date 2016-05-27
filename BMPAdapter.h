#ifndef _BMPADAPTER_H_INCLUDED
#define _BMPADAPTER_H_INCLUDED

#include <ArduinoJson.h>
#include "BMP085.h"

#include "SensorChainedAdapterBase.h"

class BMPAdapter : public SensorChainedAdapterBase {
public:
    BMPAdapter(int16_t sda, int16_t scl, const char* suffix) : mSensor(sda, scl), SensorChainedAdapterBase("BMP", suffix)  {}

    bool begin() override;

    bool save(JsonObject* json) override;

private:
    BMP085 mSensor;
};


#endif
