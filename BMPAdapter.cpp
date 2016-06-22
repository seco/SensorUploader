#include "BMPAdapter.h"


bool BMPAdapter::begin() {
    return mSensor.begin();
}


bool BMPAdapter::save(JsonObject* json) {
    float p = mSensor.readPressure();
    float t = mSensor.readTemperature();

    if (isnan(p) || p > 1500 || p <= 0 || isnan(t))
        return false;

    (*json)[String("pressure") + mSuffix] = p;
    (*json)[String("temperature") + mSuffix] = t;

    return true;
}
