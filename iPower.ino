// Import libraries
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "Mesh.h"
#include "printf.h"
#include "dht11.h"
#include "HashMap.h"
#include "Payload.h"
#include "acs712.h"

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
      payload.print();
      printf("\n\r");
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
    led(LED_GREEN);
    // fill payload message
    charge_payload();
    // send message to base
    mesh.send(&payload, base_id);
    delay(4000);
  }

  // check button
  handle_button();
}

/****************************************************************************/

void charge_payload() {
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
    printf("PAYLOAD: Info: New payload: ");
    payload.print();
    printf("\n\r");
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
  acs12 ACS12;
  int state;
  // check power
  if(states[POWER]) {
    state = ACS12.read(ACS712PIN);
  } else {
    state = ACS12.calibrate(ACS712PIN);
  }

  switch (state) {
    case ACS12LIB_OK:
      states[AMPERAGE] = ACS12.amperage;
      return true;
    case ACS12LIB_CALIBRATED:
      return true;
    default: 
      printf("ACS12: Error: Unknown error!\n\r");
      return false;
  }
}