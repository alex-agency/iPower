
#ifndef _RELAY_H__
#define _RELAY_H__

void relay(char* relay, int state) {
  // turn on/off
  if(relay == RELAY_1) {
    digitalWrite(RELAY1PIN, state);
    if(DEBUG) printf("RELAY: Info: %s is enabled.\n\r", relay);

  } else if(relay == RELAY_2) {
    digitalWrite(RELAY2PIN, state);
    if(DEBUG) printf("RELAY: Info: %s is disabled.\n\r", relay);
  }
  // save states
  if(state == RELAY_ON) {
    states[relay] = true;
    states[POWER] = true;

  } else if(state == RELAY_OFF) {
    states[relay] = false;

    if(states[RELAY_1] == states[RELAY_2])
      states[POWER] = false;
  }
}

#endif // _BUTTON_H__
