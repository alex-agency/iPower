
#ifndef __PAYLOAD_H__
#define __PAYLOAD_H__

#include "HashMap.h"

/**
 * Payload
 */
struct Payload
{
	//CreateHashMap(sensors, char*, int, 10);
        char* sensors;
	//CreateHashMap(controls, char*, int, 10);
        char* controls;
  	char* toString()
  	{
    	char* buffer;
  		sprintf(buffer, "Sensors: %s, Controls: %s", 
  			//sensors.toString(), controls.toString());
                        sensors, controls);
  		return buffer;
  	};
};

#endif // __PAYLOAD_H__
