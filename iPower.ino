// Import libraries
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "Mesh.h"
#include "printf.h"
#include "dht11.h"

// Set up nRF24L01 radio on SPI bus pin CE and CS
RF24 radio(9,10);
// Set up Mesh network
Mesh mesh(radio);
// Declare radio channel 1-128
const uint8_t channel = 100;
// Declare unique node id
const uint16_t node_id = 00102;

// Declare DHT11 sensor digital pin
#define DHT11PIN  3

// Declare LED digital pins
#define LED_RED  6
#define LED_GREEN  5
// Declare LED state
#define LED_OFF  0

// Declare relays digital pins 
#define RELAY_1  8
#define RELAY_2  7

// Declare pushbutton digital pin
#define BUTTON  4

// Declare ACS712 sensor analog pin
#define ACS712PIN  A0

// Debug info.
const bool DEBUG = true;

/**
 * Payload
 */
struct Payload
{
  int humidity;
  int temperature;
  bool relay_1;
  bool relay_2;
  float power; 
  char* DHT11_state;
  char* ACS712_state;
  //Payload(void): DHT11_reading() {}
  char* toString(void) {  };
};

//
// Setup
//
void setup(void)
{
  // Configure console
  Serial.begin(57600);
  printf_begin();
  
  // initialize radio
  radio.begin();
  // initialize network
  mesh.begin(channel, node_id);
  
  // initialize relays, turned off
  //digitalWrite(RELAY_1, RELAY_OFF);
  //digitalWrite(RELAY_2, RELAY_OFF);
  // initialize relays pins
  //pinMode(RELAY_1, OUTPUT);  
  //pinMode(RELAY_2, OUTPUT);
}

//
// Loop
//
void loop(void)
{
  // update network
  mesh.update();
  
  while( mesh.available() ) 
  {
    Payload payload;
    mesh.read(&payload);
    
    payload.toString();
  }
  
  
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

      return;
    case 2:

      return;
    case 3:

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

void relay_on(int pin) {
  const int ON = 0;
  // initialize relay pin
  pinMode(pin, OUTPUT);

  digitalWrite(pin, ON);

  relay
}

void relay_off(int pin) {
  const int OFF = 1;

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

bool read_ACS712(float& amperage) {

  //if( relays_off ) {
    //calibrated_zero

  //}

////  // turn on pullup resistors
////  pinMode(sensorPin, INPUT);
////  digitalWrite(sensorPin, HIGH);
////  // sensitivity
////  int cycleCount = 700;
////  float sum = 0;
////  for(int i = 0; i < cycleCount; i++) {
////    float sensor = analogRead(sensorPin);
////    
////    if(sensor-zeroValue < -2.5) {
////      sensor = zeroValue + ((sensor-zeroValue) * (-1));
////    }
////    sum = sum + sensor;
////    delay(10);
////  }
////  // calculate
////  float sensorValue = sum / cycleCount;
////  float delta = sensorValue - zeroValue;
////  float voltage = delta * 0.00488 * 1000; // 5V/1024
////  float current = voltage / 100; // 100mv = 1A
////  
////    // Debug info
////  printf("\n\r sensorValue: %d, delta: %d, voltage: $d", sensorValue, delta, voltage);
////  
////  return current;
}

/****************************************************************************/
