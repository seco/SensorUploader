#include "DHTAdapter.h"


bool DHTAdapter::begin() {
    mSensor.begin();

    delay(300);
    
    return !isnan(mSensor.readTemperature());
}


bool DHTAdapter::save(JsonObject* json) {
    float h = mSensor.readHumidity();
    float t = mSensor.readTemperature();

    if (isnan(h) || isnan(t))
        return false;
    
    (*json)[String("humidity") + mSuffix] = h;
    (*json)[String("temperature") + mSuffix] = t;

    return true;
}
