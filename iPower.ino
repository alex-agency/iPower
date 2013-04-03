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
// Declare state value
#define LED_OFF  0

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
uint16_t ACS712_shift = 512;
// Declare state map key
#define AMPERAGE  "amperage"

// Declare payload
Payload payload;

// Declare state map
CreateHashMap(states, char*, int, 8);

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
    if(DEBUG) {
      printf("PAYLOAD: Info: Got payload: ");
      payload.toString();
    }

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
    led(LED_RED);
    
    // create message
    create_payload();
    // send message to base
    mesh.send(&payload, base_id);
  }

  // check button
  handle_button();
}

/****************************************************************************/

void create_payload() {
  // get DHT11 sensor values
  read_DHT11();
  payload.sensors[HUMIDITY] = states[HUMIDITY];
  payload.sensors[TEMPERATURE] = states[TEMPERATURE];
  
  // get ACS712 sensor value
  read_ACS712();
  payload.sensors[AMPERAGE] = states[AMPERAGE];
  
  // get relays state
  payload.controls[RELAY_1] = states[RELAY_1];
  payload.controls[RELAY_2] = states[RELAY_2];
  
  if(DEBUG) {
    printf("PAYLOAD: Info: Created payload: ");
    payload.toString();
    printf("\n\r");
  }
}

/****************************************************************************/

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

/****************************************************************************/

void handle_button() {
  switch ( read_button() ) {
    case 1:
      led(LED_RED);
      // turning ON relay #1
      relay(RELAY_1, RELAY_ON);
      return;
    case 2:
      led(LED_RED);
      // turning ON relay #2
      relay(RELAY_2, RELAY_ON);
      return;
    case 3:
      // turning OFF relay #1 and #2
      relay(RELAY_1, RELAY_OFF);
      relay(RELAY_2, RELAY_OFF);
      led(LED_OFF);
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

void relay(char* relay, int state) {
  // turn on/off
  if(relay == RELAY_1) {
    digitalWrite(RELAY1PIN, state);
  } else if(relay == RELAY_2) {
    digitalWrite(RELAY2PIN, state);
  }
  
  if(state == RELAY_ON) {
    // save states
    states[relay] = true;
    states[POWER] = true;
    if(DEBUG) printf("RELAY: Info: %s is enabled.\n\r", relay);
  } else if(state == RELAY_OFF) {
    // save states
    states[relay] = false;
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
      if(DEBUG) printf("DHT11: Info: Sensor values: humidity: %d, temperature: %d.\n\r", 
                          states[HUMIDITY], states[TEMPERATURE]);
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
    if(DEBUG) printf("ACS712: Info: Calibrated: shift is %u.\n\r", ACS712_shift);
    return false;
  }
  // read sensor
  uint16_t sensor = analogReadAccuracy(ACS712PIN, sensitivity);
  // calculate
  // 512 = 0A = 2500mV, shift = 512
  // 100mV/(2500mV/512) = 20.48mA = 513
  int delta = sensor - ACS712_shift;
  states[AMPERAGE] = delta * 20.48;

  if(DEBUG) printf("ACS712: Info: sensor: %u, delta: %d, amperage: %d.\n\r", 
              sensor, delta, states[AMPERAGE]);
  return true;
}

/****************************************************************************/

uint16_t analogReadAccuracy(int pin, int sensitivity) {
  uint64_t sum = 0;
  for(int i = 0; i < sensitivity; i++) {
    sum = sum + analogRead(pin);
    delay(5);
  }
  return sum / sensitivity;
}

/****************************************************************************/
