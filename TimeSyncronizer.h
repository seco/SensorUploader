#ifndef _TimeSyncronizer_H_INCLUDED
#define _TimeSyncronizer_H_INCLUDED

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>


class TimeSyncronizer {
public:
    TimeSyncronizer()  {
        http.begin("www.time.gov", 80, "/actualtime.cgi");
    }

    uint64_t acquire(bool force = false);

private:
    HTTPClient http;
    uint32_t mMillis;
    uint64_t mCurrent;
};

extern TimeSyncronizer Time;

#endif
