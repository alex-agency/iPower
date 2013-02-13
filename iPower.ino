// Import libraries
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "RF24Network.h"
#include "nodeconfig.h"
#include "sleep.h"
#include "dht11.h"
#include "printf.h"

// Declare DHT11 sensor digital pin
#define DHT11PIN  3

// Declare LEDs digital pin
#define RED  6
#define GREEN  5

// Declare relays digital pin 
#define RELAY_1  8
#define RELAY_2  7
// Declare relays status
#define RELAY_ON  0
#define RELAY_OFF  1

// Declare ACS712 sensor analog pin
#define ACS712PIN  A0

// Set up nRF24L01 radio on SPI bus pin CE and CS
RF24 radio(9,10);
// Set up Network uses that radio
RF24Network network(radio);
// Declare channel 1-128
const uint16_t channel = 100;
// current node address
eeprom_info_t this_node;

// The watchdog timer sleep constants (4 sec*1 cycle)
const wdt_prescalar_e wdt_prescalar = wdt_4s;
const int sleep_cycles_per_transmission = 1;

// Initialize timer for regulate sending interval
Timer send_timer(2000); // ms







// Delay manager to send message regularly
//const unsigned long interval = 2000; // ms
//unsigned long last_time_sent;




// Radio pipe addresses for the 2 nodes to communicate.
//const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

// Declare radio greetings
//const uint8_t grettings[] = { 0, 0 }

// Declare radio message
//const int message[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
//const uint8_t message_size = sizeof(message);
//const char* message_friendly_name[] = { "module_id", "module_state", 
//                                       "humidity", "temperature", 
//                                       "relay1", "relay2", "power", 
//                                       "DHT11_state", "ACS712_state" };
// Declare radio payload
//const uint8_t payload[] = { };
//const uint8_t payload_size = sizeof(payload);
//const char* payload_friendly_name[] = { "", "",   }

// module sleeping and wait greetings, checking every 15 sec. 
// base sent hello every 5 sec and wait greetings with id.
// module got hello, send hello with id and wait for connect.
// base got hello with id, send connect and wait for message.
// module got connect, send message and wait for payload.



//
// Setup
//
void setup(void) 
{
  // Configure console
  Serial.begin(57600);
  printf_begin();
  
  // read node from EEPROM
  this_node = nodeconfig_read();
  
  if ( this_node.relay == false ) {
    Sleep.begin(wdt_prescalar,sleep_cycles_per_transmission);
  }
  
  // initialize timer
  send_timer.begin();
  
  // initialize radio
  radio.begin();
  // initialize network
  network.begin(channel, this_node.address);
  
  
  
  
  
  
//  // initialize relays, turned off
//  digitalWrite(Relay1, RELAY_OFF);
//  digitalWrite(Relay2, RELAY_OFF);
//  // initialize relays pins
//  pinMode(Relay1, OUTPUT);  
//  pinMode(Relay2, OUTPUT);
//  
//  // initialize radio
//  radio.begin();
//  // optionally, increase the delay between retries & # of retries
//  radio.setRetries(15,15);
//  // optionally, reduce the payload size.  seems to
//  // improve reliability
//  //radio.setPayloadSize(8);
//  // Open pipes to writing
//  radio.openWritingPipe(pipe);
//  // print radio info
//  radio.printDetails();
//  // enable radio
//  radio.startListening();
//  radio.stopListening();
//
//  // initialize leds pins
//  pinMode(ledRed, OUTPUT);
//  pinMode(ledGreen, OUTPUT);
  
}

//
// Loop
//
void loop(void) 
{
  // Pump the network regularly
  network.update();
  
  // Is there anything ready for us?
  while ( network.available() ) 
  {  
    RF24NetworkHeader header;
    network.peek(header);
    
    // Dispatch the message to the correct handler.
    switch (header.type)
    {
    case 'T':
      handle_T(header);
      break;
    case 'N':
      handle_N(header);
      break;
    default:
      printf_P(PSTR("*** WARNING *** Unknown message type %c\n\r"),header.type);
      network.read(header,0,0);
      break;
    };
  }
  
  
  
  // Listen for a new node address
  nodeconfig_listen();
  
  
  
//  // green led
//  digitalWrite(ledRed, LOW);
//  digitalWrite(ledGreen, HIGH);
//  
//  // Read DHT11 sensor
//  int chk = DHT11.read(DHT11PIN);
//  char *state;
//  switch (chk)
//  {
//    case 0:  state="OK"; break;
//    case -1: state="Checksum error"; break;
//    case -2: state="Time out error"; break;
//    default: state="Unknown error"; break;
//  }
//  
//  // Put data to message
//  message[0] = DHT11.humidity;
//  message[1] = DHT11.temperature;
//  
//  // Sending...
//  bool ok = radio.write( message, message_size );     
//  char *rf_state;
//  if (ok)
//      rf_state="sent";
//  else
//      rf_state="failed!"; 
//  
//  // Debug Info
//  printf("\n\r The message[%d, %d] was %s by transmitter", message[0], message[1], rf_state);
//  
//  // calibrate ACS712-20A sensor zero
//  printf("\n\r Calibraiting power sensor zero ...");
//  float zeroValue = calibrateCurrentSensor(sensorPin);
//  printf(" Done! It is %d", zeroValue);
//  
//  // red led
//  digitalWrite(ledGreen, LOW);
//  digitalWrite(ledRed, HIGH);
//  
//  printf("\n\r Turning on Relay#1 ... ");
//  delay(1000);
//  digitalWrite(Relay1, RELAY_ON);
//  delay(3000);
//  
//  // read ACS712-20A sensor
//  printf("\n\r Reading power sensor value ...");
//  float currentValue = readACSensor(sensorPin, zeroValue);
//  printf(" Done! Current is %d, Power is %d", currentValue, readPower(currentValue, 220));
//  
//  printf("\n\r Turning on Relay#2 ... ");
//  delay(1000);
//  digitalWrite(Relay2, RELAY_ON);
//  delay(3000);
//  
//  // read ACS712-20A sensor
//  printf("\n\r Reading power sensor value ...");
//  currentValue = readACSensor(sensorPin, zeroValue);
//  printf(" Done! Current is %d, Power is %d", currentValue, readPower(currentValue, 220));
//  
//  delay(3000);
//  printf("\n\r Turning off relays ... ");
//  delay(1000);
//  digitalWrite(Relay1, RELAY_OFF);
//  delay(2000);
//  digitalWrite(Relay2, RELAY_OFF); 
//  delay(3000);
  
}

/****************************************************************************/

void readDHTSensor() {
  dht11 DHT11;
  int state = DHT11.read(DHT11PIN);
  switch (state) {
    case DHTLIB_OK:
      DHT11.humidity;
      DHT11.temperature;
      return;
    case DHTLIB_ERROR_CHECKSUM:  
      
      return;
    case DHTLIB_ERROR_TIMEOUT: 
      
      return;
    default: 
      
      return;
  }
}

/****************************************************************************/

void led(int pin) {
  switch (pin) {
    case RED:
      digitalWrite(RED, HIGH);
      digitalWrite(GREEN, LOW);
      return;
    case GREEN:
      digitalWrite(GREEN, HIGH);
      digitalWrite(RED, LOW);
      return;
    default: 
      digitalWrite(GREEN, LOW);
      digitalWrite(RED, LOW);
      return;
  }
}

/****************************************************************************/




//float readCurrentSensorVC(int sensorPin, float zeroValue) {
//  // turn on pullup resistors
//  pinMode(sensorPin, INPUT);
//  digitalWrite(sensorPin, HIGH);       
//  // sensitivity
//  int cycleCount = 700;
//  float sum = 0;
//  for(int i = 0; i < cycleCount; i++) {
//    sum = sum + analogRead(sensorPin);
//    delay(10);
//  }
//  // calculate
//  float sensorValue = sum / cycleCount;
//  float delta = sensorValue - zeroValue;
//  float voltage = delta * 0.00488 * 1000; // 5V/1024
//  float current = voltage / 100; // 100mv = 1A
//  
//  // Debug info
//  printf("\n\r sensorValue: %d, delta: %d, voltage: $d", sensorValue, delta, voltage);
//  
//  return current;
//}
//
//float calibrateCurrentSensor(int sensorPin) { 
//  // turn on pullup resistors
//  pinMode(sensorPin, INPUT);
//  digitalWrite(sensorPin, HIGH);       
//  // sensitivity
//  int cycleCount = 700;
//  float sum = 0;
//  for(int i = 0; i < cycleCount; i++) {
//    sum = sum + analogRead(sensorPin);
//    delay(10);
//  }
//  // returns zero value
//  return sum / cycleCount;
//}
//
//int readPower(float current, int volt) {
//  return current * volt;
//}
//
//float readACSensor(int sensorPin, float zeroValue) {
//  // turn on pullup resistors
//  pinMode(sensorPin, INPUT);
//  digitalWrite(sensorPin, HIGH);
//  // sensitivity
//  int cycleCount = 700;
//  float sum = 0;
//  for(int i = 0; i < cycleCount; i++) {
//    float sensor = analogRead(sensorPin);
//    
//    if(sensor-zeroValue < -2.5) {
//      sensor = zeroValue + ((sensor-zeroValue) * (-1));
//    }
//    sum = sum + sensor;
//    delay(10);
//  }
//  // calculate
//  float sensorValue = sum / cycleCount;
//  float delta = sensorValue - zeroValue;
//  float voltage = delta * 0.00488 * 1000; // 5V/1024
//  float current = voltage / 100; // 100mv = 1A
//  
//    // Debug info
//  printf("\n\r sensorValue: %d, delta: %d, voltage: $d", sensorValue, delta, voltage);
//  
//  return current;
//}
