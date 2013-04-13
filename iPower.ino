// Import libraries
#include <SPI.h>
#include "printf.h"
#include "nRF24L01.h"
#include "RF24.h"
#include "Mesh.h"
#include "dht11.h"
#include "SimpleMap.h"
#include "acs712.h"
#include "button.h"
#include "timer.h"
//#include "sleep.h"

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
#define LED_RED  "red_led"
#define LED_GREEN  "green_led"
// Declare state value
#define LED_OFF  0

// Declare relays digital pins
#define RELAY1PIN  7
#define RELAY2PIN  8
// Declare state map keys
#define RELAY_1  "relay_1"
#define RELAY_2  "relay_2"
// Declare state map values
#define RELAY_OFF  1
#define RELAY_ON  0

// Declare pushbutton digital pin
#define BUTTONPIN  4

// Declare ACS712 sensor analog pin
#define ACS712PIN  A0
// Declare state map key
#define AMPERAGE  "amperage"

// Declare state map
struct comparator {
  bool operator()(const char* &str1, const char* &str2) {
    return strcmp(str1, str2) == 0;
  }
};
SimpleMap<const char*, int, 8, comparator> states;

// Declare delay manager in ms
timer_t send_timer(60000);

// Sleep constants.  In this example, the watchdog timer wakes up
// every 4s, and every single wakeup we power up the radio and send
// a reading.  In real use, these numbers which be much higher.
// Try wdt_8s and 7 cycles for one reading per minute.> 1
//const wdt_prescalar_e wdt_prescalar = wdt_4s;
//const int sleep_cycles_per_transmission = 1;

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

  // Configure sleep
  //Sleep.begin(wdt_prescalar,sleep_cycles_per_transmission);
}

//
// Loop
//
void loop()
{
  // check button
  handle_button();
  
  // enable warning led if power on 
  if(states[RELAY_1] || states[RELAY_2])
     led_blink(LED_RED, false);
  else
     led_blink(LED_OFF, false);
  
  // sleeping
  //if (Sleep) {
  //  if(DEBUG) printf("SLEEP: Info: Go to Sleep.\n\r");
    // Power down the radio.  Note that the radio will get powered back up
    // on the next write() call.
  //  radio.powerDown();
    // Be sure to flush the serial first before sleeping, so everything
    // gets printed properly
  //  Serial.flush();
    // Sleep the MCU.  The watchdog timer will awaken in a short while, and
    // continue execution here.
  //  Sleep.go();
  //  if(DEBUG) printf("SLEEP: Info: WakeUp\n\r");
  //}
  
  if( mesh.ready() && send_timer ) {
    led_blink(LED_GREEN, false);
        
    // send DHT11 sensor values
    read_DHT11();
    Payload payload1(HUMIDITY, states[HUMIDITY]);
    mesh.send(payload1, base_id);
    Payload payload2(TEMPERATURE, states[TEMPERATURE]);
    mesh.send(payload2, base_id);
    
    // send ACS712 sensor value
    read_ACS712();
    Payload payload3(AMPERAGE, states[AMPERAGE]);
    mesh.send(payload3, base_id);

    // send relays state
    Payload payload4(RELAY_1, states[RELAY_1]);
    mesh.send(payload4, base_id);
    Payload payload5(RELAY_2, states[RELAY_2]);
    mesh.send(payload5, base_id);

    led_blink(LED_OFF, false);
  }
  
  // update network
  mesh.update();
  
  // is new payload message available?
  while( mesh.available() ) {
    Payload payload;
    mesh.read(payload);
    
    if(payload.value)
      relay(payload.key, RELAY_ON);
    else if(payload.value == false)
      relay(payload.key, RELAY_OFF);
  }
}

/****************************************************************************/

void led_blink(const char* led, bool blink) {
  // blinking
  if(blink && states[led]) {
    led = LED_OFF;
  }
  // clear all states
  states[LED_RED] = false;
  states[LED_GREEN] = false;

  if(strcmp(led, LED_RED) == 0) {
    // initialize led pins
    pinMode(LEDREDPIN, OUTPUT);
    // enable red led
    digitalWrite(LEDREDPIN, HIGH);
    digitalWrite(LEDGREENPIN, LOW);
    // save state
    states[LED_RED] = true;
  } 
  else if(strcmp(led, LED_GREEN) == 0) {
    // initialize led pins
    pinMode(LEDGREENPIN, OUTPUT);
    // enable green led
    digitalWrite(LEDGREENPIN, HIGH);
    digitalWrite(LEDREDPIN, LOW);
    // save state
    states[LED_GREEN] = true;
  } 
  else {
    // disable leds
    digitalWrite(LEDREDPIN, LOW);
    digitalWrite(LEDGREENPIN, LOW);
  }
}

/****************************************************************************/

void relay(const char* relay, int state) {
  // initialize relays pin
  pinMode(RELAY1PIN, OUTPUT);
  pinMode(RELAY2PIN, OUTPUT);
  // turn on/off
  if(strcmp(relay, RELAY_1) == 0) {
    digitalWrite(RELAY1PIN, state);
  } 
  else if(strcmp(relay, RELAY_2) == 0) {
    digitalWrite(RELAY2PIN, state);
  } 
  else {
    printf("RELAY: Error: '%s' is unknown!\n\r", relay);
    return;
  }
  // save states
  if(state == RELAY_ON) {
    if(DEBUG) printf("RELAY: Info: %s is enabled.\n\r", relay);
    states[relay] = true;
  } 
  else if(state == RELAY_OFF) {
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
  } 
  else if(state != BUTTONLIB_OK) {
    printf("BUTTON: Error: Incorrect push! It's too short or long.\n\r");
    return false;
  }
  // handle command
  switch (BUTTON.command) {
    case 1:
      // turning ON relay #1
      relay(RELAY_1, RELAY_ON);
      return true;
    case 2:
      // turning ON relay #2
      relay(RELAY_2, RELAY_ON);
      return true;
    case 3:
      // turning OFF relay #1 and #2
      relay(RELAY_1, RELAY_OFF);
      relay(RELAY_2, RELAY_OFF);
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
