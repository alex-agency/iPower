#ifndef __ACS712_H__
#define __ACS712_H__

#define ACSLIB_OK  0
// debug console
//#define DEBUG

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
    if(value < 2)
      amperage = 0;
    else
      amperage = value * 49;
    #ifdef DEBUG
      printf_P(PSTR("ACS712: Info: Delta: %d, Amperage: %d.\n\r"), 
                        value, amperage);
    #endif
    //Serial.println((amperage * 216.0) / 1000.0);
    return ACSLIB_OK;
  };
};

#endif // __ACS712_H__
