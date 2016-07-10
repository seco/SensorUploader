#include "UpdateAdapter.h"
#include <ESP8266httpUpdate.h>
#include "Utils.h"


bool UpdateAdapter::process(char* topic, char* payload)
{
	ESPhttpUpdate.rebootOnUpdate(false);
        
    t_httpUpdate_return ret = ESPhttpUpdate.update(payload);

    switch(ret) {
        case HTTP_UPDATE_FAILED:
            Serial.printf("Update failed Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
            break;

        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("Update no updates");
            break;

        case HTTP_UPDATE_OK:
            Serial.println("Update ok");
            break;
    }

    Utils::systemRestart();
}