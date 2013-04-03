
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
  const void toString() const
  {
    printf_P(PSTR("Sensors: "));
    sensors.toString();
    printf_P(PSTR(", Controls: "));
    controls.toString();
  };
};

#endif // __PAYLOAD_H__
