
#ifndef __ACS712_H__
#define __ACS712_H__


bool read_ACS712() {
  // reading sensitivity
  int sensitivity = 200;
  // check relays state
  if(states[POWER] == false) {
    states[AMPERAGE] = 0;
    // calibarate zero value
    ACS712_shift = analogReadAccuracy(ACS712PIN, sensitivity);
    if(DEBUG) printf("ACS712: Info: Calibrated: Shift is %u.\n\r", ACS712_shift);
    return false;
  }
  // read sensor
  uint16_t sensor = analogReadAccuracy(ACS712PIN, sensitivity);
  // calculate
  // 512 = 0A = 2500mV, shift = 512
  // 100mV/(2500mV/512) = 20.48mA = 513
  int delta = sensor - ACS712_shift;
  states[AMPERAGE] = delta * 20.48;

  if(DEBUG) printf("ACS712: Info: Sensor: %u, Delta: %d, Amperage: %d.\n\r", 
              sensor, delta, states[AMPERAGE]);
  return true;
}

uint16_t analogReadAccuracy(int pin, int sensitivity) {
  uint64_t sum = 0;
  for(int i = 0; i < sensitivity; i++) {
    sum = sum + analogRead(pin);
    delay(5);
  }
  return sum / sensitivity;
}

#endif // __ACS712_H__
