#include "RelayAdapter.h"

bool RelayAdapter::process(char* topic, char* payload)
{
	bool on = (!strcmp(pMsgOn.c_str(), payload));
	bool off = (!strcmp(pMsgOff.c_str(), payload));

	if (on || off)
	{
		digitalWrite(mPin, on);
	}
}