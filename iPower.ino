// Import libraries
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "Mesh.h"
#include "printf.h"
#include "dht11.h"
#include "HashMap.h"
#include "acs712.h"
#include "button.h"

// Declare SPI bus pins
#define CE_PIN  9
#define CS_PIN  10
// Set up nRF24L01 radio
RF24 radio(CE_PIN, CS_PIN);
// Set up network
Mesh mesh(radio);
// Declare radio channel 0-127
const uint8_t channel = 76;
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
// Declare state map values
#define RELAY_OFF  1
#define RELAY_ON  0

// Declare pushbutton digital pin
#define BUTTONPIN  4
// Declare state map key
#define PUSHBUTTON  "pushbutton"

// Declare ACS712 sensor analog pin
#define ACS712PIN  A0
// Declare state map key
#define AMPERAGE  "amperage"

// Declare payload (24byte maximum)
Payload payload;
// used for many payloads
uint8_t last_sent_payload = 0;

// Declare state map
HashMap<char*,uint8_t,8> states;

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
    mesh.read(payload);
    if(DEBUG) {
      printf("PAYLOAD: Info: Got payload: %s.\n\r", payload.toString());
    }

    //if(payload.get(RELAY_1))
      relay(RELAY_1, RELAY_ON);
    //else
      relay(RELAY_1, RELAY_OFF);
    
    //if(payload.get(RELAY_2))
      relay(RELAY_2, RELAY_ON);
    //else
      relay(RELAY_2, RELAY_OFF);
  }
  
  // connection ready
  if( mesh.ready() ) {
    led_blink(LED_GREEN, true);
    // fill payload message
    charge_payload();
    // send message to base
    mesh.send(payload, base_id);
  }

  // check button
  handle_button();
}

/****************************************************************************/

void charge_payload() {
  // clear payload
  payload.clear();
  // make first payload
  //if(last_sent_payload != 1) {
    // get DHT11 sensor values
    read_DHT11();
    payload.put(1, states[HUMIDITY]);
    payload.put(2, states[TEMPERATURE]);
    // get ACS712 sensor value
    read_ACS712();
    payload.put(3, states[AMPERAGE]);

    last_sent_payload = 1;
  // make second payload
  //} else if(last_sent_payload != 2) {
    // get relays state
    payload.put(4, states[RELAY_1]);
    payload.put(5, states[RELAY_2]);

    last_sent_payload = 2;
  //}
  if(DEBUG) {
    printf("PAYLOAD: Info: New payload: ");
    payload.print();
    printf("\n\r");
  }
}

/****************************************************************************/

void led_blink(char* led, bool blink) {
  // blinking
  if(blink && states[led]) {
    led = LED_OFF;
  }
  // clear all states
  states[LED_RED] = false;
  states[LED_GREEN] = false;

  if(led == LED_RED) {
    // initialize led pins
    pinMode(LEDREDPIN, OUTPUT);
    // enable red led
    digitalWrite(LEDREDPIN, HIGH);
    digitalWrite(LEDGREENPIN, LOW);
    // save state
    states[LED_RED] = true;

  } else if(led == LED_GREEN) {
    // initialize led pins
    pinMode(LEDGREENPIN, OUTPUT);
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

void relay(char* relay, int state) {
  // initialize relays pin
  pinMode(RELAY1PIN, OUTPUT);
  pinMode(RELAY2PIN, OUTPUT);
  // turn on/off
  if(relay == RELAY_1) {
    digitalWrite(RELAY1PIN, state);
  } else if(relay == RELAY_2) {
    digitalWrite(RELAY2PIN, state);
  }
  // save states
  if(state == RELAY_ON) {
    if(DEBUG) printf("RELAY: Info: %s is enabled.\n\r", relay);
    states[relay] = true;
  } else if(state == RELAY_OFF) {
    if(DEBUG) printf("RELAY: Info: %s is disabled.\n\r", relay);
    states[relay] = false;
  }
}

/****************************************************************************/

bool handle_button() {
  button BUTTON;
  //read button
  int state = BUTTON.read(BUTTONPIN, LEDGREENPIN, LEDREDPIN);
  if(state == BUTTONLIB_RELEASE) {
    return false;
  } else if(state != BUTTONLIB_OK) {
    printf("BUTTON: Error: Incorrect push! It's too short or long.\n\r");
    return false;
  }
  // handle command
  switch (BUTTON.command) {
    case 1:
      led_blink(LED_RED, false);
      // turning ON relay #1
      relay(RELAY_1, RELAY_ON);
      return true;
    case 2:
      led_blink(LED_RED, false);
      // turning ON relay #2
      relay(RELAY_2, RELAY_ON);
      return true;
    case 3:
      // turning OFF relay #1 and #2
      relay(RELAY_1, RELAY_OFF);
      relay(RELAY_2, RELAY_OFF);
      led_blink(LED_OFF, false);
      return true;
    default:
      return false;
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
  acs712 ACS712;
  int state = ACS712.read(ACS712PIN);
  switch (state) {
    case ACSLIB_OK:
      states[AMPERAGE] = ACS712.amperage;
      if(DEBUG) printf("ACS712: Info: Sensor value: amperage: %d.\n\r", 
                          states[AMPERAGE]);
      return true;
    default: 
      printf("ACS712: Error: Unknown error!\n\r");
      return false;
  }
}

/****************************************************************************/
