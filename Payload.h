
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
	HashMap sensors;
  	HashMap controls;
  	char* print()
  	{
  		printf_P(PSTR("Sensors: %s, Controls: %s", 
  			sensors.toString(), controls.toString()));
  	};
};
#endif // __PAYLOAD_H__