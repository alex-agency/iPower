#ifndef __ACS712_H__
#define __ACS712_H__

#define ACSLIB_OK  0
#define ACSLIB_DEBUG  false

const int sensitivity = 200;
uint16_t shift = 512;

class acs712
{
public:
  int amperage;

  int read(int pin) {
    // initialize sensor pin with internal pullup resistor
    pinMode(pin, INPUT_PULLUP);
    // read sensor
    uint16_t sensor = analogReadAccuracy(pin, sensitivity);
    // shifting zero for calibrate
    if(sensor > 500 && sensor < 524) {
      shift = sensor;
    }
    // calculate
    // 512 = 0A = 2500mV, shift = 512
    // 100mV/(2500mV/512) = 20.48mA = 513
    int delta = sensor - shift;
    amperage = delta * 20.48;

    if(ACSLIB_DEBUG) printf("ACS712: Info: Sensor: %u, Delta: %d, Amperage: %d.\n\r", 
                        sensor, delta, amperage);
    return ACSLIB_OK;
  };

private:
  uint16_t analogReadAccuracy(int pin, int sensitivity) {
    uint64_t sum = 0;
    for(int i = 0; i < sensitivity; i++) {
      sum = sum + analogRead(pin);
      delay(5);
    }
    return sum / sensitivity;
  };
};

#endif // __ACS712_H__