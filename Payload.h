
#ifndef __PAYLOAD_H__
#define __PAYLOAD_H__

#include "HashMap.h"

/**
 * Payload
 */
struct Payload
{
  CreateHashMap(sensors, char*, int, 10);
  CreateHashMap(controls, char*, int, 10);
  const char* toString() const
  {
    static char buffer[45];
    snprintf_P(buffer,sizeof(buffer),PSTR("Sensors: %s, Controls: %s"),
      sensors.toString(), controls.toString());
    return buffer;
  };
};

#endif // __PAYLOAD_H__
