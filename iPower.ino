// Import libraries
#include <SPI.h>
//#include "nRF24L01.h"
//#include "RF24.h"
//#include "Mesh.h"
#include "DHT11.h"
#include "SimpleMap.h"
#include "acs712.h"
#include "timer.h"
#include "led.h"
#include "MemoryFree.h"
#include "Watchdog.h"
#include "OneButton.h"
#include "LowPower.h"

// debug console
#define DEBUG

// Declare output functions
#ifdef DEBUG
  static int serial_putchar(char c, FILE *) {
    Serial.write(c);
    return 0;
  };
  FILE serial_out = {0};
#endif

// Avoid spurious warnings
#if ! defined( NATIVE ) && defined( ARDUINO )
#undef PROGMEM
#define PROGMEM __attribute__(( section(".progmem.data") ))
#undef PSTR
#define PSTR(s) (__extension__({static const char __c[] PROGMEM = (s); &__c[0];}))
#endif

// Declare SPI bus pins
//#define CE_PIN  9
//#define CS_PIN  10
// Set up nRF24L01 radio
//RF24 radio(CE_PIN, CS_PIN);
// Set up network
//Mesh mesh(radio);
// Declare radio channel 0-127
//const uint8_t channel = 76;
// Declare unique node id
//const uint16_t node_id = 111;
// Declare base id, base always has 00 id
//const uint16_t base_id = 00;

// Declare DHT11 sensor digital pin
#define DHT11PIN  3
// Declare state map keys
#define HUMIDITY  "humidity"
#define COMPUTER_TEMP  "comp. temp" // temperature inside

// Set up LED digital pins
Led led(5, 6); // (green, red) 

// Declare relays digital pins
#define RELAY1PIN  8
#define RELAY2PIN  7
// Declare state map keys
#define RELAY_1  "relay_1"
#define RELAY_2  "relay_2"

// Declare push button
OneButton button(4, true);

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

// Declare delay managers in ms
timer_t timer(30000);

// Declare critical state
const int warm_temp = 40;
const int warm_humid = 80;
const int warm_amp = 8000;

//
// Setup
//
void setup()
{
  // Configure output
  #ifdef DEBUG
    Serial.begin(9600);
    fdev_setup_stream(&serial_out, serial_putchar, NULL, _FDEV_SETUP_WRITE);
    stdout = stderr = &serial_out;
  #endif

  #ifdef DEBUG
    printf_P(PSTR("Free memory: %d bytes.\n\r"), freeMemory());
  #endif
  // prevent continiously restart
  delay(500);
  // restart if memory lower 512 bytes
  softResetMem(512);
  // restart after freezing for 8 sec
  softResetTimeout();

  // initialize radio
  //radio.begin();
  // initialize network
  //mesh.begin(channel, node_id);

  // Configure button
  button.attachClick( buttonClick );
  button.attachLongPressStart( buttonLongPress );
}

//
// Loop
//
void loop()
{
  // watchdog
  heartbeat();
  // enable warning led if power on 
  if(states[RELAY_1] || states[RELAY_2])
    led.set(LED_RED);
  else
    led.set(LED_OFF);

  if(timer) {
  	// reading sensors
  	read_DHT11();
  	read_ACS712();
  	// checking for critical state
  	if(states[COMPUTER_TEMP]>=warm_temp ||
  	   states[HUMIDITY]>=warm_humid || 
  	   states[AMPERAGE]>=warm_amp) 
  	{      
      printf_P(PSTR("WARNING: Device sensors found critical value!"));
  	  printf_P(PSTR(" Device power will be shut down!\n\r"));
  	  // power off
      relayOff(RELAY_1);
      relayOff(RELAY_2);
      printf_P(PSTR("WARNING: Temperature: %d, Humidity: %d, Amperage: %d\n\r"),
      states[COMPUTER_TEMP], states[HUMIDITY], states[AMPERAGE]);
      	  
      led.set_blink(LED_RED, 5);
  	}
    // if network ready send values to base
  	/*if( mesh.ready() ) {
      led.set(LED_GREEN);	    
  	  // send DHT11 sensor values
  	  Payload payload1(HUMIDITY, states[HUMIDITY]);
  	  mesh.send(payload1, base_id);
  	  Payload payload2(COMPUTER_TEMP, states[COMPUTER_TEMP]);
  	  mesh.send(payload2, base_id);
  	  // send ACS712 sensor value
  	  Payload payload3(AMPERAGE, states[AMPERAGE]);
  	  mesh.send(payload3, base_id);
      // send relays state
      Payload payload4(RELAY_1, states[RELAY_1]);
      mesh.send(payload4, base_id);
      Payload payload5(RELAY_2, states[RELAY_2]);
      mesh.send(payload5, base_id);
  	}*/
  }
  // update network
  //mesh.update();
  // is new payload message available?
  /*while( mesh.available() ) {
    Payload payload;
    mesh.read(payload);
    // accept payload form base only
    if(payload.id == base_id) {
	    
	  if(payload.value)
      relayOn(payload.key);
	  else if(payload.value == false)
	    relayOff(payload.key);    	
    }
  }*/
  // update led
  led.update();
  // update push button
  button.tick();
  
  // sleeping
  #ifdef DEBUG
    printf_P(PSTR("SLEEP: Info: Go to Sleep.\n\r"));
  #endif
  // Power down the radio.  Note that the radio will get powered back up
  // on the next write() call.
  //radio.powerDown();
  Serial.flush();
  // set all pin to low with pullup.
  //for(int i=1; i<=21; i++) {
  //  pinMode(i, INPUT_PULLUP);
  //  digitalWrite(i, LOW);
  //}
  // Enter power down state for X*X sec with ADC and BOD module disabled
  LowPower.powerDown(SLEEP_120MS, 1, ADC_OFF, BOD_OFF);
  #ifdef DEBUG
    printf_P(PSTR("SLEEP: Info: WakeUp\n\r"));
  #endif
}

/****************************************************************************/

void relayOn(const char* relay) {
  if(states[relay]) {
    // relay is already on
    return;
  }
  bool status = relays(relay, 0); // 0 is ON
  if(status) {
    #ifdef DEBUG
      printf_P(PSTR("RELAY: Info: '%s' is enabled.\n\r"), relay);
    #endif
    states[relay] = true;
  }
}

void relayOff(const char* relay) {
  if(states[relay] == false) {
    // relay is already off
    return;
  }
  bool status = relays(relay, 1); // 1 is OFF
  if(status) {
    #ifdef DEBUG
      printf_P(PSTR("RELAY: Info: '%s' is disabled.\n\r"), relay);
    #endif
    states[relay] = false;
  }
}

bool relays(const char* relay, uint8_t state) {
  if(strcmp(relay, RELAY_1) == 0) {
    pinMode(RELAY1PIN, OUTPUT);
    digitalWrite(RELAY1PIN, state);
    return true;
  } 
  if(strcmp(relay, RELAY_2) == 0) {
    pinMode(RELAY2PIN, OUTPUT);
    digitalWrite(RELAY2PIN, state);
    return true;
  } 
  printf_P(PSTR("RELAY: Error: '%s' is unknown!\n\r"), relay);
  return false;
}

/****************************************************************************/

void buttonClick() {
  #ifdef DEBUG
    printf_P(PSTR("BUTTON: Info: Short click.\n\r"));
  #endif
  relayOff(RELAY_1);
  relayOff(RELAY_2);
}

void buttonLongPress() {
  #ifdef DEBUG
    printf_P(PSTR("BUTTON: Info: Long press.\n\r"));
  #endif
  relayOn(RELAY_1);
  relayOn(RELAY_2);
}

/****************************************************************************/

bool read_DHT11() {
  dht11 DHT11;
  if( DHT11.read(DHT11PIN) == DHTLIB_OK ) {
    states[HUMIDITY] = DHT11.humidity;
    states[COMPUTER_TEMP] = DHT11.temperature;
    #ifdef DEBUG
      printf_P(PSTR("DHT11: Info: Air humidity: %d, temperature: %dC.\n\r"), 
        states[HUMIDITY], states[COMPUTER_TEMP]);
    #endif
    return true;
  }
  printf_P(PSTR("DHT11: Error: Communication failed!\n\r"));
  return false;
}

/****************************************************************************/

bool read_ACS712() {
  acs712 ACS712;
  int state = ACS712.read(ACS712PIN);
  switch (state) {
    case ACSLIB_OK:
      states[AMPERAGE] = ACS712.amperage;
      #ifdef DEBUG
        printf_P(PSTR("ACS712: Info: Sensor value: amperage: %d.\n\r"), 
                          states[AMPERAGE]);
      #endif
      return true;
    default: 
      printf("ACS712: Error: Unknown error!\n\r");
      return false;
  }
}

/****************************************************************************/
