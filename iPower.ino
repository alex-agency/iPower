// Import libraries
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "Mesh.h"
#include "printf.h"

// Set up nRF24L01 radio on SPI bus pin CE and CS
RF24 radio(9,10);
// Set up Mesh network
Mesh mesh(radio);
// Declare radio channel 1-128
const uint8_t channel = 100;
// Declare this node id
const uint16_t node_id = 00102;

/**
 * Message
 */
struct Message
{
  //uint16_t temp_reading;
  //uint16_t voltage_reading;
  //static char buffer[];
  //Message(void): temp_reading(0), voltage_reading(0) {}
  //char* toString(void);
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
     Message message = mesh.read();
  }
}

