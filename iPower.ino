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

// Declare relays digital pin 
#define RELAY_1  8
#define RELAY_2  7
// Declare relays state
#define RELAY_ON  0
#define RELAY_OFF  1

// Declare ACS712 sensor analog pin
#define ACS712PIN  A0

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
  digitalWrite(RELAY_1, RELAY_OFF);
  digitalWrite(RELAY_2, RELAY_OFF);
  // initialize relays pins
  pinMode(RELAY_1, OUTPUT);  
  pinMode(RELAY_2, OUTPUT);
  
  // initialize led, turned off
  led(LED_OFF);
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
  switch (pin) {
    case LED_RED:
      digitalWrite(LED_RED, HIGH);
      digitalWrite(LED_GREEN, LOW);
      return;
    case LED_GREEN:
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_RED, LOW);
      return;
    default: 
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_RED, LOW);
      // initialize led pins
      pinMode(LED_RED, OUTPUT);
      pinMode(LED_GREEN, OUTPUT);
      return;
  }
}

/****************************************************************************/

bool read_DHT11(int humidity&, int temperature&) {
  dht11 DHT11;
  int state = DHT11.read(DHT11PIN);
  switch (state) {
    case DHTLIB_OK:
      humidity = DHT11.humidity;
      temperature = DHT11.temperature;
      return true;
    case DHTLIB_ERROR_CHECKSUM:  
      
      return false;
    case DHTLIB_ERROR_TIMEOUT: 
      
      return false;
    default: 
      
      return false;
  }
}

/****************************************************************************/

