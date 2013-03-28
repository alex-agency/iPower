// Import libraries
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "Mesh.h"
#include "printf.h"
#include "dht11.h"
#include "HashMap.h"
#include "Payload.h"

// Set up nRF24L01 radio on SPI bus pin CE and CS
RF24 radio(9,10);
// Set up Mesh network
Mesh mesh(radio);
// Declare radio channel 1-128
const uint8_t channel = 100;
// Declare unique node id
const uint16_t node_id = 00102;
// Declare base id
const uint16_t base_id = 00001;

// Declare DHT11 sensor digital pin
#define DHT11PIN  3

// Declare LED digital pins
#define LED_RED  6
#define LED_GREEN  5
// Declare LED state
#define LED_OFF  0

// Declare relays digital pins 
const uint8_t relay_pins[] = {7, 8};
// Relays state (on, off)
uint8_t relay_states[sizeof(relay_pins)];

// Declare pushbutton digital pin
#define BUTTON  4

// Declare ACS712 sensor analog pin
#define ACS712PIN  A0
// Shifting value for calibrate sensor
float ACS712_shift = 0;
// How many reading cycle?
int ACS712_sensitivity = 700;

// Declare payload
Payload payload;
// Define payload keys
#define HUMIDITY  "humidity"
#define TEMPERATURE  "temperature"
#define AMPERAGE  "amperage"
#define RELAY1  "relay1"
#define RELAY2  "relay2"

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
    mesh.read(&payload);
    if(DEBUG) printf("MESH: Info: Got payload: %s", payload.toString());

    if( payload.controls[RELAY1] )
      relay_on(1);
    else
      relay_off(1);
    
    if( payload.controls[RELAY2] )
      relay_on(2);
    else
      relay_off(2);
  }
  
  // connection ready
  if( mesh.ready() ) {
    led(LED_GREEN);
    // create message
    create_payload();
    // send message to base
    mesh.send(&payload, base_id);
  } else {
    led(LED_RED);
  }

  // check button
  handle_button();
}

/****************************************************************************/

void create_payload() {
  int humidity = 0;
  int temperature = 0;
  float amperage = 0;
  
  if( read_DHT11(humidity, temperature) ) {
    payload.sensors[HUMIDITY] = humidity;
    payload.sensors[TEMPERATURE] = temperature;
  }

  read_ACS712(amperage);
  payload.sensors[AMPERAGE] = amperage;

  payload.controls[RELAY1] = relay_states[1];
  payload.controls[RELAY2] = relay_states[2];

  if(DEBUG) printf("PAYLOAD: Info: Created payload: %s", payload.toString());
}

/****************************************************************************/

void led(int pin) {
  // initialize led pins
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  switch (pin) {
    case LED_RED:
      digitalWrite(LED_RED, HIGH);
      digitalWrite(LED_GREEN, LOW);
      if(DEBUG) printf("LED: Info: RED led is enabled.\n\r");
      return;
    case LED_GREEN:
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_RED, LOW);
      if(DEBUG) printf("LED: Info: GREEN led is enabled.\n\r");
      return;
    default: 
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_RED, LOW);
      if(DEBUG) printf("LED: Info: Leds turned off.\n\r");
      return;
  }
}

/****************************************************************************/

void handle_button() {
  
  switch ( read_button() ) {
    case 1:
      led(LED_RED);
      // turning ON relay #1
      relay_on(1);
      return;
    case 2:
      led(LED_RED);
      // turning ON relay #2
      relay_on(2);
      return;
    case 3:
      // turning OFF relay #1 and #2
      relay_off(1);
      relay_off(2);
      led(LED_OFF);
      return;
    default:
      return;
  }
}

/****************************************************************************/

int read_button() {
  // initialize pushbutton pin
  pinMode(BUTTON, INPUT);

  // is button released?
  if(digitalRead(BUTTON) == LOW) {
    return 0;
  }
  delay(1000);
  if(digitalRead(BUTTON) == LOW) {
    // skip if it pushed by mistake
    printf("BUTTON: Error: Sticky button!\n\r");
    return 0;
  }

  if(DEBUG) printf("BUTTON: Info: Button is pushed. And waiting for commands.\n\r");
  // turned off leds
  led(LED_OFF);

  int count_pushed = 0;
  while( button_pushed() ) {
    count_pushed++;
    if(DEBUG) printf("BUTTON: Info: Button is pushed: %d times.\n\r", count_pushed);
  }
  return count_pushed;
}

/****************************************************************************/

bool button_pushed() {
  // wait for release a button
  delay(500);
  if(digitalRead(BUTTON) == HIGH) {
    return false;
  }
  led(LED_GREEN);
  // wait for push a button
  delay(500);
  if(digitalRead(BUTTON) == HIGH) {
    led(LED_OFF);
    return true;
  }
  return false;
}

/****************************************************************************/

void relay_on(int index) {
  const int ON = 0;
  if(relay_states[index] == false) {
    // initialize relay pin
    pinMode(relay_pins[index], OUTPUT);
    // turning on
    digitalWrite(relay_pins[index], ON);
    // save state
    relay_states[index] = true;
    if(DEBUG) printf("RELAY: Info: Relay #%d is enabled.\n\r", index);
  }
}

/****************************************************************************/

void relay_off(int index) {
  const int OFF = 1;
  if(relay_states[index]) {
    // turning off
    digitalWrite(relay_pins[index], OFF);
    // save state
    relay_states[index] = false;
    if(DEBUG) printf("RELAY: Info: Relay #%d is disabled.\n\r", index);
  }
}

/****************************************************************************/

bool read_DHT11(int& humidity, int& temperature) {
  dht11 DHT11;
  int state = DHT11.read(DHT11PIN);
  switch (state) {
    case DHTLIB_OK:
      humidity = DHT11.humidity;
      temperature = DHT11.temperature;
      if(DEBUG) printf("DH11: Info: Sensor values: humidity: %d, temperature: %d.\n\r", 
                          humidity, temperature);
      return true;
    case DHTLIB_ERROR_CHECKSUM:  
      printf("DH11: Error: Checksum test failed!: The data may be incorrect!\n\r");
      return false;
    case DHTLIB_ERROR_TIMEOUT: 
      printf("DH11: Error: Timeout occured!: Communication failed!\n\r");
      return false;
    default: 
      printf("DH11: Error: Unknown error!\n\r");
      return false;
  }
}

/****************************************************************************/

void read_ACS712(float& amperage) {
  // check relays state
  if( relay_states[1] == false 
      && relay_states[2] == false ) {
    // calibarate zero value
    calibrate_ACS712();
    amperage = 0;
    return;
  }
  // turn on pullup resistors
  pinMode(ACS712PIN, INPUT);
  digitalWrite(ACS712PIN, HIGH);

  float sum = 0;
  for(int i = 0; i < ACS712_sensitivity; i++) {
    float value = analogRead(ACS712PIN);

    if(value-ACS712_shift < -2.5) {
      value = ACS712_shift + ((value-ACS712_shift) * (-1));
    }
    sum = sum + value;
    delay(10);
  }
  // calculate
  float sensor = sum / ACS712_sensitivity;
  float delta = sensor - ACS712_shift;
  float voltage = delta * 0.00488 * 1000; // 5V/1024
  amperage = voltage / 100; // 100mv = 1A
  
  // Debug info
  if(DEBUG) printf("ACS712: Info: sensor: %d, delta: %d, voltage: $d, amperage: $d \n\r", 
    sensor, delta, voltage, amperage);
}

/****************************************************************************/

void calibrate_ACS712() {
  // turn on pullup resistors
  pinMode(ACS712PIN, INPUT);
  digitalWrite(ACS712PIN, HIGH);       

  float sum = 0;
  for(int i = 0; i < ACS712_sensitivity; i++) {
    sum = sum + analogRead(ACS712PIN);
    delay(10);
  }
  // update zero value
  ACS712_shift = sum / ACS712_sensitivity;
}

/****************************************************************************/
