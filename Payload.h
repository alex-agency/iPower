
#ifndef __PAYLOAD_H__
#define __PAYLOAD_H__

#include "HashMap.h"

/**
 * Payload
 */
struct Payload
{
	HashMap<char*,float,10> sensors;
  	HashMap<char*,bool,10> controls;
  	char* toString()
  	{
    	char* buffer;
  		sprintf(buffer, "Sensors: %s, Controls: %s", 
  			sensors.toString(), controls.toString());
  		return buffer;
  	};
};

#endif // __PAYLOAD_H__