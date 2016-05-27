#include "TimeSyncronizer.h"

TimeSyncronizer Time;


uint64_t TimeSyncronizer::acquire(bool force) {
    uint32_t cval = millis();
    if (mMillis > cval  || mMillis == 0 || force) {
        mMillis = cval;

        Serial.println("Reading Time data");
        
        if ( http.GET() == 200) {
            String data = http.getString();

            uint64_t tmp = 0;
            int d = data.indexOf("<timestamp time=\"");

            if (d >= 0) {
                for (size_t i = 17 + d; i < data.length() ; i++) {
                    if (!isDigit(data[i]))
                        break;

                    tmp = tmp * 10ULL + (uint64_t)(data[i] - '0');
                }


                mCurrent = tmp;

                return mCurrent;

            } else {
                Serial.println("No timestamp");
                return -1;
            }
        } else {
            Serial.println("No 200");
            return -1;
        }

    }

    return mCurrent + (uint64_t)(millis() - mMillis) * 1000ULL;
}
