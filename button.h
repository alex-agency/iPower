
#ifndef _BUTTON_H__
#define _BUTTON_H__

#define BUTTONLIB_OK  1
#define BUTTONLIB_RELEASE  0
#define BUTTONLIB_ERROR_SHORT_START  -1
#define BUTTONLIB_ERROR_LONG_START  -2

#define BUTTONLIB_DEBUG  false

class button 
{
public:
  int command;

  int read(int button_pin, int led1_pin, int led2_pin) {
    // initialize button pin with internal pullup resistor
    pinMode(button_pin, INPUT_PULLUP);
    // initialize led pins
    pinMode(led1_pin, OUTPUT);
    pinMode(led2_pin, OUTPUT);
    // reset last command
    command = 0;
    
    // 1: exit if button released
    if(digitalRead(button_pin) == HIGH) {
      return BUTTONLIB_RELEASE;
    }
    // 2: exit if first push shorter than 1 sec
    digitalWrite(led1_pin, HIGH);
    digitalWrite(led2_pin, LOW);
    delay(1000);
    if(digitalRead(button_pin) == HIGH) {
      // skip if it pushed by mistake
      digitalWrite(led1_pin, LOW);
      return BUTTONLIB_ERROR_SHORT_START;
    }
    // 3: exit if first push longer than 1,5 sec
    digitalWrite(led1_pin, LOW);
    delay(500);
    if(digitalRead(button_pin) != HIGH) {
      // skip if it pushed by mistake
      return BUTTONLIB_ERROR_LONG_START;
    }
    // 4: counting push
    byte last_button_state;
    int wait = 3000;
    long last_pushed = millis();
    while(millis() < wait+last_pushed) {
      // read button
      byte state = digitalRead(button_pin);
      // check for changing state from release to push
      if(state != last_button_state && state != HIGH) {
        digitalWrite(led2_pin, HIGH);
        command++;
        if(BUTTONLIB_DEBUG) printf("BUTTON: Info: %d push after %d msec.\n\r",
                    command, millis()-last_pushed);
        delay(250);
      }
      digitalWrite(led2_pin, LOW);
      last_button_state = state;
    }
    if(BUTTONLIB_DEBUG) printf("BUTTON: Info: Button is pushed: %d times.\n\r", 
                command);
    return BUTTONLIB_OK;
  };
};

#endif // _BUTTON_H__
