// Import libraries
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "Mesh.h"
#include "printf.h"
#include "dht11.h"
#include "HashMap.h"
//#include "Payload.h"

// Set up nRF24L01 radio on SPI bus pin CE and CS
RF24 radio(9,10);
// Set up Mesh network
Mesh mesh(radio);
// Declare radio channel 1-128
const uint8_t channel = 90;
// Declare unique node id
const uint16_t node_id = 1001;
// Declare base id, it should be 00
const uint16_t base_id = 00;

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
int ACS712_shift = 0;
// Sensor reading cycle count
int ACS712_sensitivity = 700;

// Declare payload

struct Payload
{
  CreateHashMap(sensors, char*, int, 10);
  CreateHashMap(controls, char*, int, 10);
  char* toString()
  	{
    	char* buffer;
  		sprintf(buffer, "Sensors: %s, Controls: %s", 
  			sensors.toString(), controls.toString());
  		return buffer;
  	};
};
Payload payload;

// Declare payload keys
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

  // initialize button pin with internal pullup resistor
  pinMode(BUTTON, INPUT_PULLUP);

  // initialize led pins
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

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

    //if( payload.controls[RELAY1] )
    //  relay_on(0);
    //else
    //  relay_off(0);
    
    //if( payload.controls[RELAY2] )
    //  relay_on(1);
    //else
    //  relay_off(1);
  }
  
  // connection ready
  if( mesh.ready() ) {
    led(LED_GREEN);
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

  printf("PAYLOAD: %s ", payload.sensors.toString());


  // get ACS712 sensor value
  int amperage;
  read_ACS712(amperage);
  //payload.sensors[AMPERAGE] = amperage;

  // get relays state
  //payload.controls[RELAY1] = relay_states[0];
  //payload.controls[RELAY2] = relay_states[1];
  
  //if(DEBUG) printf("PAYLOAD: Info: Created payload: %s", payload.toString());
}

/****************************************************************************/

void led(int pin) {
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
  if(digitalRead(BUTTON) == HIGH) {
    return 0;
  }
  // 2: exit if first push shorter than 1 sec
  led(LED_GREEN);
  delay(1000);
  if(digitalRead(BUTTON) == HIGH) {
    // skip if it pushed by mistake
    led(LED_OFF);
    printf("BUTTON: Error: Incorrect push! It's too short.\n\r");
    return 0;
  }
  // 3: exit if first push longer than 1,5 sec
  led(LED_OFF);
  delay(500);
  if(digitalRead(BUTTON) != HIGH) {
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
    byte button = digitalRead(BUTTON);
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

void read_ACS712(int& amperage) {
  // check relays state
  if( relay_states[0] == false 
      && relay_states[1] == false ) {
    // calibarate zero value
    calibrate_ACS712();
    amperage = 0;
    return;
  }

  uint64_t sum = 0;
  for(int i = 0; i < ACS712_sensitivity; i++) {
    int value = analogRead(ACS712PIN);
    // ????
    //if(value-ACS712_shift < -2.5) {
    //  value = ACS712_shift + ((value-ACS712_shift) * (-1));
    //}
    sum = sum + value;
    delay(10);
  }
  // calculate
  int sensor = sum / ACS712_sensitivity;
  int delta = sensor - ACS712_shift;
  int voltage = delta * 4.88; // 5V/1024*1000
  amperage = voltage / 100; // 100mv = 1A

  if(DEBUG) printf("ACS712: Info: sensor: %d, delta: %d, voltage: %d, amperage: %d \n\r", 
              sensor, delta, voltage, amperage);
}

/****************************************************************************/

void calibrate_ACS712() {
  uint64_t sum = 0;
  for(int i = 0; i < ACS712_sensitivity; i++) {
    sum = sum + analogRead(ACS712PIN);
    delay(10);
  }
  // update zero value
  ACS712_shift = sum / ACS712_sensitivity;
  if(DEBUG) printf("ACS712: Info: Calibrated: shift is %d \n\r", ACS712_shift);
}

/****************************************************************************/
