// Import libraries
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
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

#define ACS712-20A





// Set up nRF24L01 radio on SPI bus pin CE and CS
RF24 radio(9,10);
// The radio pipe address.
const uint64_t pipe = 0xE8E8F0F0E1LL;
// Declare radio message
uint8_t message[2];
uint8_t message_size = sizeof(message);






// Declare ACS712-20A sensor analog pin
const int sensorPin = A0;


//
// Setup
//
void setup(void)
{
  // Configure console
  Serial.begin(57600);
  printf_begin();
  
  // initialize relays, turned off
  digitalWrite(Relay1, RELAY_OFF);
  digitalWrite(Relay2, RELAY_OFF);
  // initialize relays pins
  pinMode(Relay1, OUTPUT);  
  pinMode(Relay2, OUTPUT);
  
  // initialize radio
  radio.begin();
  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);
  // optionally, reduce the payload size.  seems to
  // improve reliability
  //radio.setPayloadSize(8);
  // Open pipes to writing
  radio.openWritingPipe(pipe);
  // print radio info
  radio.printDetails();
  // enable radio
  radio.startListening();
  radio.stopListening();

  // initialize leds pins
  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  
}

//
// Loop
//
void loop(void)
{
  
  
  // green led
  digitalWrite(ledRed, LOW);
  digitalWrite(ledGreen, HIGH);
  
  // Read DHT11 sensor
  int chk = DHT11.read(DHT11PIN);
  char *state;
  switch (chk)
  {
    case 0:  state="OK"; break;
    case -1: state="Checksum error"; break;
    case -2: state="Time out error"; break;
    default: state="Unknown error"; break;
  }
  
  // Put data to message
  message[0] = DHT11.humidity;
  message[1] = DHT11.temperature;
  
  // Sending...
  bool ok = radio.write( message, message_size );     
  char *rf_state;
  if (ok)
      rf_state="sent";
  else
      rf_state="failed!"; 
  
  // Debug Info
  printf("\n\r The message[%d, %d] was %s by transmitter", message[0], message[1], rf_state);
  
  // calibrate ACS712-20A sensor zero
  printf("\n\r Calibraiting power sensor zero ...");
  float zeroValue = calibrateCurrentSensor(sensorPin);
  printf(" Done! It is %d", zeroValue);
  
  // red led
  digitalWrite(ledGreen, LOW);
  digitalWrite(ledRed, HIGH);
  
  printf("\n\r Turning on Relay#1 ... ");
  delay(1000);
  digitalWrite(Relay1, RELAY_ON);
  delay(3000);
  
  // read ACS712-20A sensor
  printf("\n\r Reading power sensor value ...");
  float currentValue = readACSensor(sensorPin, zeroValue);
  printf(" Done! Current is %d, Power is %d", currentValue, readPower(currentValue, 220));
  
  printf("\n\r Turning on Relay#2 ... ");
  delay(1000);
  digitalWrite(Relay2, RELAY_ON);
  delay(3000);
  
  // read ACS712-20A sensor
  printf("\n\r Reading power sensor value ...");
  currentValue = readACSensor(sensorPin, zeroValue);
  printf(" Done! Current is %d, Power is %d", currentValue, readPower(currentValue, 220));
  
  delay(3000);
  printf("\n\r Turning off relays ... ");
  delay(1000);
  digitalWrite(Relay1, RELAY_OFF);
  delay(2000);
  digitalWrite(Relay2, RELAY_OFF); 
  delay(3000);
  
}

/****************************************************************************/

readDHTSensor() {
  int state = dht11.read(DHT11PIN);
  switch (state) {
    case DHTLIB_OK:
      dht11.humidity;
      dht11.tempearature;
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







float readCurrentSensorVC(int sensorPin, float zeroValue) {
  // turn on pullup resistors
  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);       
  // sensitivity
  int cycleCount = 700;
  float sum = 0;
  for(int i = 0; i < cycleCount; i++) {
    sum = sum + analogRead(sensorPin);
    delay(10);
  }
  // calculate
  float sensorValue = sum / cycleCount;
  float delta = sensorValue - zeroValue;
  float voltage = delta * 0.00488 * 1000; // 5V/1024
  float current = voltage / 100; // 100mv = 1A
  
  // Debug info
  printf("\n\r sensorValue: %d, delta: %d, voltage: $d", sensorValue, delta, voltage);
  
  return current;
}

float calibrateCurrentSensor(int sensorPin) { 
  // turn on pullup resistors
  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);       
  // sensitivity
  int cycleCount = 700;
  float sum = 0;
  for(int i = 0; i < cycleCount; i++) {
    sum = sum + analogRead(sensorPin);
    delay(10);
  }
  // returns zero value
  return sum / cycleCount;
}

int readPower(float current, int volt) {
  return current * volt;
}

float readACSensor(int sensorPin, float zeroValue) {
  // turn on pullup resistors
  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);
  // sensitivity
  int cycleCount = 700;
  float sum = 0;
  for(int i = 0; i < cycleCount; i++) {
    float sensor = analogRead(sensorPin);
    
    if(sensor-zeroValue < -2.5) {
      sensor = zeroValue + ((sensor-zeroValue) * (-1));
    }
    sum = sum + sensor;
    delay(10);
  }
  // calculate
  float sensorValue = sum / cycleCount;
  float delta = sensorValue - zeroValue;
  float voltage = delta * 0.00488 * 1000; // 5V/1024
  float current = voltage / 100; // 100mv = 1A
  
    // Debug info
  printf("\n\r sensorValue: %d, delta: %d, voltage: $d", sensorValue, delta, voltage);
  
  return current;
}
