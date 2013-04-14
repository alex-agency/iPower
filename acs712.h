#ifndef __ACS712_H__
#define __ACS712_H__

#define ACSLIB_OK  0
#define ACSLIB_DEBUG  true

const int sensitivity = 100;
uint16_t shift = 514;

class acs712
{
public:
  int amperage;
  
  int read(int pin) {
    uint16_t sensor = 0;
    uint16_t value = 0;
    // initialize sensor pin
    pinMode(pin, INPUT);
    // get maximum value
    for(int i = 0; i < sensitivity; i++) {
      sensor = analogRead(pin);
      if(sensor > value) {
        value = sensor;
      }
      delay(50);
    }
    // shifting zero
    if(value>=shift-2 && value<=shift) {
      value = shift;
    }
    // calculate amperage
    // 512 = 0A, 512*(5V/1024) = 2.5V
    // 1A = 100mV, 10A = 1V = (1V/5V)*1024 = 204.8
    // 1 = 10000mA/204.8 = 48,83 = 48mA
    int delta = value - shift;
    amperage = delta * 48;
    if(ACSLIB_DEBUG) printf("ACS712: Info: Sensor: %u, Delta: %d, Amperage: %d, Power: %d.\n\r", 
                        value, delta, amperage, (amperage*216)/1000);
    return ACSLIB_OK;
  };
};

#endif // __ACS712_H__
