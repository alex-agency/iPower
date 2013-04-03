// Import libraries
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "Mesh.h"
#include "printf.h"
#include "dht11.h"
#include "HashMap.h"
#include "Payload.h"

// Declare SPI bus pins
#define CE_PIN  9
#define CS_PIN  10
// Set up nRF24L01 radio
RF24 radio(CE_PIN, CS_PIN);
// Set up network
Mesh mesh(radio);
// Declare radio channel 1-128
const uint8_t channel = 90;
// Declare unique node id
const uint16_t node_id = 1001;
// Declare base id, base always has 00 id
const uint16_t base_id = 00;

// Declare DHT11 sensor digital pin
#define DHT11PIN  3
// Declare state map keys
#define HUMIDITY  "humidity"
#define TEMPERATURE  "temperature"

// Declare LED digital pins
#define LEDREDPIN  6
#define LEDGREENPIN  5
// Declare state map keys
#define LED_RED  "red led"
#define LED_GREEN  "green led"

// Declare relays digital pins
#define RELAY1PIN  7
#define RELAY2PIN  8
// Declare state map keys
#define RELAY_1  "relay 1"
#define RELAY_2  "relay 2"
#define POWER  "power"
// Declare state map values
#define RELAY_OFF  1
#define RELAY_ON  0

// Declare pushbutton digital pin
#define BUTTONPIN  4
// Declare state map key
#define PUSHBUTTON  "pushbutton"

// Declare ACS712 sensor analog pin
#define ACS712PIN  A0
// Shifting value for calibrate sensor
int ACS712_shift = 512;
// Declare state map key
#define AMPERAGE  "amperage"

// Declare payload
Payload payload;

// Declare state map
CreateHashMap(states, char*, int, 10);

// Debug info.
const bool DEBUG = true;

//
// Setup
//
void setup()
{
  // Configure console
  Serial.begin(57600);
  printf_begin();

  // initialize radio
  radio.begin();
  // initialize network
  mesh.begin(channel, node_id);

  // initialize button pin with internal pullup resistor
  pinMode(BUTTONPIN, INPUT_PULLUP);

  // initialize led pins
  pinMode(LEDREDPIN, OUTPUT);
  pinMode(LEDGREENPIN, OUTPUT);

  // initialize relays pin
  pinMode(RELAY1PIN, OUTPUT);
  pinMode(RELAY2PIN, OUTPUT);

  // initialize sensor pin with internal pullup resistor
  pinMode(ACS712PIN, INPUT_PULLUP);
}

//
// Loop
//
void loop()
{
  // update network
  mesh.update();

  // new message available
  while( mesh.available() ) {
    mesh.read(&payload);
    if(DEBUG) printf("PAYLOAD: Info: Got payload: %s", payload.toString());

    if( payload.controls[RELAY_1] )
      relay(RELAY_1, RELAY_ON);
    else
      relay(RELAY_1, RELAY_OFF);
    
    if( payload.controls[RELAY_2] )
      relay(RELAY_2, RELAY_ON);
    else
      relay(RELAY_2, RELAY_OFF);
  }
  
  // connection ready
  if( mesh.ready() ) {
    //led(LED_GREEN);
    // create message
    create_payload();
    // send message to base
    //mesh.send(&payload, base_id);
  }

  // check button
  handle_button();
}

/****************************************************************************/

void create_payload() {
  // get DHT11 sensor values
  int humidity, temperature;
  read_DHT11(humidity, temperature);
  payload.sensors[HUMIDITY] = humidity;
  payload.sensors[TEMPERATURE] = temperature;

  /////////////////////
  printf("PAYLOAD: %s ", payload.sensors.toString());
  /////////////////////

  // get ACS712 sensor value
  int amperage;
  read_ACS712(amperage);
  payload.sensors[AMPERAGE] = amperage;

  // get relays state
  payload.controls[RELAY1] = relay_states[0];
  payload.controls[RELAY2] = relay_states[1];
  
  if(DEBUG) printf("PAYLOAD: Info: Created payload: %s", payload.toString());
}

/****************************************************************************/

void led(char* led, bool blink) {
  int delay = 200;
  if(blink && millis() > delay+last_led_blink) {
    // turn off
    pin = 0;
  }

  if()


  switch (pin) {
    // enable red led
    case LED_RED:
      digitalWrite(LED_RED, HIGH);
      digitalWrite(LED_GREEN, LOW);
      return;
    // enable green led
    case LED_GREEN:
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_RED, LOW);
      return;
    // turn off leds
    default: 
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_RED, LOW);
      return;
  }
}

/****************************************************************************/

void led_blink(int pin, int count) {
  led(LED_OFF);
  for(int i=0; i<count; i++) {
    delay(300);
    led(pin);
    delay(150);
    led(LED_OFF);
  }
}

/****************************************************************************/

void handle_button() {
  switch ( read_button() ) {
    case 1:
      led(LED_RED);
      // turning ON relay #1
      relay_on(0);
      return;
    case 2:
      led(LED_RED);
      // turning ON relay #2
      relay_on(1);
      return;
    case 3:
      // turning OFF relay #1 and #2
      relay_off(0);
      relay_off(1);
      return;
    default:
      return;
  }
}

/****************************************************************************/

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

/****************************************************************************/

relay(char* relay, int state) {
  // turn on/off
  if(states[relay] == RELAY1) {
    digitalWrite(RELAY1PIN, state);
  } else if(states[relay] == RELAY2) {
    digitalWrite(RELAY2PIN, state);
  }
  // save state
  states[relay] = state;

  if(state == RELAY_ON) {
    // power is on
    states[POWER] = true;
    if(DEBUG) printf("RELAY: Info: %s is enabled.\n\r", relay);
  } else if(state == RELAY_OFF) {
    // power is off
    if(states[RELAY_1] == states[RELAY_2])
      states[POWER] = false;
    if(DEBUG) printf("RELAY: Info: %s is disabled.\n\r", relay);
  }
}

/****************************************************************************/

bool read_DHT11() {
  dht11 DHT11;
  int state = DHT11.read(DHT11PIN);
  switch (state) {
    case DHTLIB_OK:
      states[HUMIDITY] = DHT11.humidity;
      states[TEMPERATURE] = DHT11.temperature;
      if(DEBUG) printf("DHT11: Info: Sensor values: humidity: %d, temperature: %d\n\r", 
                          humidity, temperature);
      return true;
    case DHTLIB_ERROR_CHECKSUM:  
      printf("DHT11: Error: Checksum test failed!: The data may be incorrect!\n\r");
      return false;
    case DHTLIB_ERROR_TIMEOUT: 
      printf("DHT11: Error: Timeout occured!: Communication failed!\n\r");
      return false;
    default: 
      printf("DHT11: Error: Unknown error!\n\r");
      return false;
  }
}

/****************************************************************************/

bool read_ACS712() {
  // reading sensitivity
  int sensitivity = 500;
  // check relays state
  if(states[POWER] == false) {
    states[AMPERAGE] = 0;
    // calibarate zero value
    ACS712_shift = analogReadAccuracy(ACS712PIN, sensitivity);
    if(DEBUG) printf("ACS712: Info: Calibrated: shift is %d \n\r", ACS712_shift);
    return false;
  }
  // read sensor
  int sensor = analogReadAccuracy(ACS712PIN, sensitivity);
  // calculate
  // 512 = 0A = 2500mV, shift = 512
  // 100mV/(2500mV/512) = 20.48mA = 513
  int delta = sensor - ACS712_shift;
  states[AMPERAGE] = delta * 20.48;

  if(DEBUG) printf("ACS712: Info: sensor: %d, delta: %d, amperage: %d \n\r", 
              sensor, delta, amperage);
  return true;
}

/****************************************************************************/

int analogReadAccuracy(int pin, int sensitivity) {
  uint64_t sum = 0;
  for(int i = 0; i < sensitivity; i++) {
    sum = sum + analogRead(pin);
    delay(5);
  }
}

/****************************************************************************/
