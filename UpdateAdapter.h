#ifndef _UPDATEADAPTER_H_INCLUDED
#define _UPDATEADAPTER_H_INCLUDED

#include <Arduino.h>

#include "SensorChainedAdapterBase.h"

class UpdateAdapter : public SensorChainedAdapterBase
{
	public:
		UpdateAdapter(const char* topic) : SensorChainedAdapterBase("UPD", NULL, topic)  {}
		bool process(char* topic, char* payload) override;

};

#endif

