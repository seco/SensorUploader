#ifndef _RELAYADAPTER_H
#define _RELAYADAPTER_H

#include <Arduino.h>
#include <ArduinoJson.h>

#include "SensorChainedAdapterBase.h"


class RelayAdapter : public SensorChainedAdapterBase {
public:
    RelayAdapter(const char* topic, int16_t pin,  const char* msgon,  const char* msgoff) : mPin(pin), pMsgOn(msgon), pMsgOff(msgoff),SensorChainedAdapterBase("RLY", NULL, topic)  {
		pinMode(pin, OUTPUT);
	}
	bool process(char* topic, char* payload) override;
private:
    int16_t mPin;
    const String pMsgOn;
    const String pMsgOff;
};


#endif
