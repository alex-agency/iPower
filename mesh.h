
#ifndef __MESH_H__
#define __MESH_H__

#include <SPI.h>
#include "RF24Network.h"

/**
 * Mesh Network Layer for RF24 Network
 *
 * This class implements an Mesh Network Layer using nRF24L01(+) radios driven
 * by RF24 and RF24Network libraries.
 */
class Mesh
{
public:
  /**
   * Construct the network
   *
   * @param _radio The underlying radio driver instance
   */
  Mesh( RF24& _radio );
  
  /**
   * Bring up the network
   *
   * @warning Be sure to 'begin' the radio first.
   */
  void begin(uint8_t _channel, uint16_t _node_id );
  
  /**
  * Send message to node with unique id.
  */
  bool send(const void* message, uint16_t to_id);

  /**
  * Update network state.
  */
  void update(void);

  /**
  * Check message box
  */
  bool available(void);

  /**
  * Read message
  */
  void read(void*);

private:
  RF24& radio; /**< Underlying radio driver, provides link/physical layers */
  RF24Network network; /**< RF24Network layer */ 
  uint16_t node_address; /**< Logical node address of this unit */
  uint16_t node_id; /**< Node id of this unit */
  uint8_t channel; /**< The RF channel to operate on */
  const static uint16_t base = 00000; /**< Base address */
  const static uint16_t homeless = 05555; /**< homeless address is last address in the network */
  const static uint16_t interval = 2000; /**< Delay manager in ms */
  uint16_t last_time_sent;
  
  /**
  * Handle message with type A, handle Address Node request
  */
  void handle_A(RF24NetworkHeader& header);

  /**
  * Send message with type A, send Address Node request
  */
  bool send_A(uint16_t to_address);

  /**
  * Handle message with type I, handle Id Node request
  */
  void handle_I(RF24NetworkHeader& header);

  /**
  * Send message with type I, send Id Node request
  */
  bool send_I();

  /**
  * Find empty node address
  * 
  * @param relay_address Find iside this relay branch,
  * 	by default finding in the root
  */
  uint16_t get_new_address(uint16_t relay_address);

  /**
  * Initialize new node address
  */
  void set_address(uint16_t address);
  
  /**
  * Reset
  */
  void flush_node();
};
#endif // __MESH_H__
