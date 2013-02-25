
#ifndef __MESH_H__
#define __MESH_H__

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "RF24Network.h"
#include "sleep.h"
#include "printf.h"

/**
 * Message
 */
struct Message
{
  uint16_t temp_reading;
  uint16_t voltage_reading;
  static char buffer[];
  Message(void): temp_reading(0), voltage_reading(0) {}
  char* toString(void);
};

class mesh
{
public:
  /**
   * Construct the network
   */
  mesh( RF24Network& _network );
  
  /**
   * Bring up the network
   */
  void begin(uint8_t _channel, uint16_t _id );
  
  /**
  * Send message to node with unique id.
  */
  bool send(const void* message, uint16_t to_id);

  /**
  * Update network state.
  */
  void update();

  /**
  * Check message box
  */
  bool available();

  /**
  * Read message
  */
  Message read();

private:
  RF24Network& network;
  // Delay manager
  const static uint16_t interval = 2000; // ms
  uint16_t last_time_sent;
  
  /**
  * Handle message with type A
  */
  void handle_A(RF24NetworkHeader header);

  /**
  * Send message with type A
  */
  bool send_A(uint16_t to);

  /**
  * Handle message with type I
  */
  void handle_I(RF24NetworkHeader header);

  /**
  * Send message with type I
  */
  bool send_I();
  
  /**
  * Handle message with type M
  */
  void handle_M(RF24NetworkHeader header);

  /**
  * Send message with type M
  */
  bool send_M(uint16_t to);

  /**
  * Find empty node address
  * 
  * @param relay_address Find iside this relay branch,
  * 	by default finding in the root
  */
  uint16_t get_address(uint16_t relay_address);

  /**
  * Initialize new node address
  */
  void set_address(uint16_t address);

};
#endif // __MESH_H__
