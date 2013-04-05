
#ifndef _LED_H__
#define _LED_H__

void led(char* led) {
  // clear all states
  states[LED_RED] = false;
  states[LED_GREEN] = false;

  if(led == LED_RED) {
    // enable red led
    digitalWrite(LEDREDPIN, HIGH);
    digitalWrite(LEDGREENPIN, LOW);
    // save state
    states[LED_RED] = true;

  } else if(led == LED_GREEN) {
    // enable green led
    digitalWrite(LEDGREENPIN, HIGH);
    digitalWrite(LEDREDPIN, LOW);
    // save state
    states[LED_GREEN] = true;
    
  } else {
    // disable leds
    digitalWrite(LEDREDPIN, LOW);
    digitalWrite(LEDGREENPIN, LOW);
  }
}

#endif // _LED_H__
