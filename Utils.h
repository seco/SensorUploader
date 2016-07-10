// Utils.h

#ifndef _UTILS_H_INCLUDED
#define _UTILS_H_INCLUDED

#include <Arduino.h>


#define RSTPIN 16     // what pin we're connected to

class Utils
{
	public:
		static void systemRestart(uint8_t pin = RSTPIN);
};

#endif

