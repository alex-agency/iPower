#ifndef __ACS712_H__
#define __ACS712_H__

#define ACSLIB_OK  0
#define ACSLIB_DEBUG  true

const int sensitivity = 10;

class acs712
{
public:
  int amperage;
  
  int read(int pin) {
    int sensor = 0;
    int value = 0;
    int delta = 0;
    double sum = 0;
    // initialize sensor pin
    pinMode(pin, INPUT);
    // get sum values
    for(uint16_t i = 0; i < sensitivity; i++) {
      sensor = analogRead(pin);
      delta = sensor - 511;
      if(delta < 0) {
        delta = delta * (-1);
      }
      sum = sum + delta;
      delay(9);
    }
    // calculate avarage value
    value = sum / sensitivity;
    // calculate amperage
    // 512 = 0A, 512*(5V/1024) = 2.5V
    // 1A = 100mV, 10A = 1V = (1V/5V)*1024 = 204.8
    // 1 = 10000mA/204.8 = 48,83 = 49mA
    amperage = value * 49;
    if(ACSLIB_DEBUG) printf("ACS712: Info: Delta: %d, Amperage: %d.\n\r", 
                        value, amperage);
    Serial.println((amperage * 216.0) / 1000.0);
    return ACSLIB_OK;
  };
};

#endif // __ACS712_H__
