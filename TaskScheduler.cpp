#include "TaskScheduler.h"
#include <Arduino.h>

TaskScheduler::TaskScheduler()
{
    lastRun = 0;
    timesRun = 0;
    setCallback(NULL);
}

void TaskScheduler::wait(int interval)
{
    unsigned long time1 = millis();

    while (true)
    {
       unsigned long time2 = millis();
       
       if (time1 + interval < time2 || time2 < time1)
            break;
       
#ifdef ESP8266
        yield();
#endif
    }
}

bool TaskScheduler::loop()
{
    unsigned long time1 = millis();

    if (time1 < lastTime) //overflow happened
    {
        lastRun = time1 + 1;
    }

    lastTime = time1;
    
    if((lastRun > 0 && lastRun + interval < time1) && (timesRun < timesToRun || timesToRun == 0))
    {
        if (callback)
            callback();

        lastRun = time1;
        timesRun++;

        return true;
    }

    return false;
}


TaskScheduler& TaskScheduler::setCallback(SCHEDULER_CALLBACK_SIGNATURE) {
    this->callback = callback;
    return *this;
}

TaskScheduler& TaskScheduler::setDelay(int delay)
{
    this->delay = delay;
    return *this;
}

TaskScheduler& TaskScheduler::setInterval(int interval)
{
    this->interval = interval;
    return *this;
}

TaskScheduler& TaskScheduler::setTimesToRun(int timesToRun)
{
    this->timesToRun = timesToRun;
    return *this;
}


void TaskScheduler::start()
{
    lastRun = millis() + delay;
}

// Stops Task
void TaskScheduler::stop()
{
    lastRun = 0;
    timesRun = 0;
}
