
#ifndef __PAYLOAD_H__
#define __PAYLOAD_H__

#include "HashMap.h"

/**
 * Payload
 *
 * This structure implements main object which contains module info
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
  	};
};
#endif // __PAYLOAD_H__
