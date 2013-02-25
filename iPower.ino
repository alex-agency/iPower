
// Import libraries
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "RF24Network.h"
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
// base node address
const uint16_t base = 00;
// current node address
uint8_t this_node = 99; // homeless

// The watchdog timer (4 sec*1 cycle)
const wdt_prescalar_e wdt_prescalar = wdt_4s;
const int sleep_cycles_per_transmission = 1;

// Delay manager
const unsigned long interval = 2000; // ms
unsigned long last_time_sent;

// Declare messages types
const char ping = 'P';
const char nodes = 'N';
const char payload = 'M';

// Declare active nodes array
const short max_active_nodes = 10;
uint16_t active_nodes[max_active_nodes];

// Declare payload message
struct Message
{
  int humidity;
  int temperature;
  bool relay_1;
  bool relay_2;
  float power; 
  char* DHT11_state;
  char* ACS712_state;
};


// Initialize timer for regulate sending interval
//Timer send_timer(2000); // ms


//
// Setup
//
void setup(void) 
{
  // Configure console
  Serial.begin(57600);
  printf_begin();
  
  // initialize sleep system
  Sleep.begin(wdt_prescalar,sleep_cycles_per_transmission);
    
  // initialize radio
  radio.begin();
  // initialize network
  network.begin(channel, this_node);
  



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
  
  // Is there anything ready?
  while ( network.available() ) 
  {
    RF24NetworkHeader header;
    network.peek(header);

//    // Dispatch the message to the correct handler.
//    switch (header.type)
//    {
//      case ping: // get a ping from another node
//        handle_ping(header);
//        break;
//      case nodes: // get active nodes which another node known
//        handle_nodes(header);
//        break;
//      case payload: // get payload message
//        handle_payload(header);
//        break;
//      default:
//        // Unknown message type
//        // skip this message
//        network.read(header,0,0);
//        break;
//    };
  }
  
  unsigned long now = millis();
  if ( now - last_time_sent >= interval )
  {
    last_time_sent = now;
    
    if(this_node == 99) {
      //send_ping();
    }
    
    // go to sleep
    if ( Sleep ) 
    {
      // Power down the radio.  Note that the radio will get powered back up
      // on the next write() call.
      radio.powerDown();
      
      // Be sure to flush the serial first before sleeping, so everything
      // gets printed properly
      Serial.flush();
      
      // Sleep the MCU.  The watchdog timer will awaken in a short while, and
      // continue execution here.
      Sleep.go();
    }
  }
  
  
  
  
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

//bool send_ping() {
//  
//  // send ping to someone
//  for(uint16_t to = 0; to < 99; to++) {
//    if( active_nodes.exists(to) && send_ping(to) ) {
//      // save new active node
//      add_nodes(to);
//      return true;
//    }
//  }
//  return false;
//}
//
///****************************************************************************/
//
//bool send_ping(uint16_t to) {
//  RF24NetworkHeader header(to, ping);
//  if( network.write(header,0,0) ) {
//    return true;
//  }
//  return false;
//}
//
///****************************************************************************/
//
//bool send_nodes(uint16_t to) {
//  RF24NetworkHeader header(to, nodes);
//  
//  return network.write(header,active_nodes,sizeof(active_nodes));
//}

/****************************************************************************/




/****************************************************************************/

//bool send_payload(uint16_t to) {
//  RF24NetworkHeader header(to, payload);
//  Message message;
//  return network.write(header,&message,sizeof(message));
//}
//
///****************************************************************************/
//
//void handle_ping(RF24NetworkHeader& header) {
//  unsigned long message;
//  network.read(header,&message,sizeof(unsigned long));
//}
//
///****************************************************************************/
//
//void handle_nodes(RF24NetworkHeader& header) {
//  static uint16_t incoming_nodes[max_active_nodes];
//  network.read(header,&incoming_nodes,sizeof(incoming_nodes));
//}
//
///****************************************************************************/
//
//void handle_payload(RF24NetworkHeader& header) {
//  unsigned long message;
//  network.read(header,&message,sizeof(unsigned long));
//}

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
