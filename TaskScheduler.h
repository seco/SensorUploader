#ifndef TASKSCHEDULER_H
#define TASKSCHEDULER_H

#ifdef ESP8266
#include <functional>
#define SCHEDULER_CALLBACK_SIGNATURE std::function<void(void)> callback
#else
#define SCHEDULER_CALLBACK_SIGNATURE void (*callback)(void)
#endif


class TaskScheduler
{
  private:
    int timesRun;
    unsigned long lastRun;
    unsigned long lastTime;
    int timesToRun;
    int delay;
    int interval;

    SCHEDULER_CALLBACK_SIGNATURE;
  public:
    // public methods
    TaskScheduler();
    TaskScheduler& setCallback(SCHEDULER_CALLBACK_SIGNATURE);
    TaskScheduler& setDelay(int delay);
    TaskScheduler& setInterval(int interval);
    TaskScheduler& setTimesToRun(int timesToRun);

    void start();
    void stop();
    bool loop();
    static void wait(int delayms);
};

#endif //TASKSCHEDULER_H
