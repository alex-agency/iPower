
#ifndef _BUTTON_H__
#define _BUTTON_H__

int read_button() {
  // 1: exit if button released
  if(digitalRead(BUTTONPIN) == HIGH) {
    return 0;
  }
  // 2: exit if first push shorter than 1 sec
  led(LED_GREEN);
  delay(1000);
  if(digitalRead(BUTTONPIN) == HIGH) {
    // skip if it pushed by mistake
    led(LED_OFF);
    printf("BUTTON: Error: Incorrect push! It's too short.\n\r");
    return 0;
  }
  // 3: exit if first push longer than 1,5 sec
  led(LED_OFF);
  delay(500);
  if(digitalRead(BUTTONPIN) != HIGH) {
    // skip if it pushed by mistake
    led(LED_OFF);
    printf("BUTTON: Error: Incorrect push! It's too long.\n\r");
    return 0;
  }
  // 4: counting push
  int count_pushed = 0;
  byte last_button_state;
  if(DEBUG) printf("BUTTON: Info: Button is waiting for commands.\n\r");
  int wait = 3000;
  long last_pushed = millis();
  while(millis() < wait+last_pushed) {
    // read button
    byte button = digitalRead(BUTTONPIN);
    // check for changing state from release to push
    if(button != last_button_state && button != HIGH) {
      led(LED_RED);
      count_pushed++;
      if(DEBUG) printf("BUTTON: Info: %d push after %d msec.\n\r",
                  count_pushed, millis()-last_pushed);
      delay(250);
    }
    led(LED_OFF);
    last_button_state = button;
  }
  if(DEBUG) printf("BUTTON: Info: Button is pushed: %d times.\n\r", 
              count_pushed);
  return count_pushed;
}

#endif // _BUTTON_H__
