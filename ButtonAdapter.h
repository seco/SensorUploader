#ifndef _BUTTONADAPTER_H_INCLUDED
#define _BUTTONADAPTER_H_INCLUDED

#include <Arduino.h>
#include <ArduinoJson.h>

#include "SensorChainedAdapterBase.h"

#define MAX_BUTTONS 17

class ButtonAdapter : public SensorChainedAdapterBase {
public:
    ButtonAdapter(int16_t pin,  const char* msgon,  const char* msgoff, const char* suffix) : mPin(pin), pMsgOn(msgon), pMsgOff(msgoff),SensorChainedAdapterBase("BTN", suffix)  
    {
        if (ButtonAdapter::sFirst) 
        {
            memset(ButtonAdapter::pButtonAdapters, 0, sizeof(ButtonAdapter::pButtonAdapters));
            ButtonAdapter::sFirst = false;
        }
    }

    bool begin() override;

    void end() override;

    bool check() override;

    bool save(JsonObject* json) override;

private:
    static void interrupt();
    static bool sFirst;
    static ButtonAdapter* pButtonAdapters[MAX_BUTTONS];
    int16_t mPin;
    bool mChanged;
    bool mValue;
    const String pMsgOn;
    const String pMsgOff;
};


#endif
