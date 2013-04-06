
#ifndef __PAYLOAD_H__
#define __PAYLOAD_H__

#include "HashMap.h"

/**
 * Payload
 */
struct Payload
{
  //CreateHashMap(sensors, char*, int, 3);
  HashMap<char*,int,3> sensors;
  //CreateHashMap(controls, char*, int, 2);
  HashMap<char*,int,2> controls;
  //////////
  int test;
  //////////
  const void print() const
  {
    printf_P(PSTR("Sensors: "));
    sensors.print();
    printf_P(PSTR(", Controls: "));
    controls.print();
  };
};

#endif // __PAYLOAD_H__
