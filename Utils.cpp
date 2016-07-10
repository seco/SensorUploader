#include "Utils.h"
#include "TaskScheduler.h"

void Utils::systemRestart(uint8_t pin)
{
	TaskScheduler::wait(50);
	pinMode(RSTPIN, OUTPUT);
	digitalWrite(RSTPIN, LOW);
}
