
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
   * Construct the network, create network object
   *
   * @param _radio The underlying radio driver instance
   */
  Mesh( RF24& _radio );
  
  /**
   * Bring up the network, configure network
   *
   * @warning Be sure to 'begin' the radio first.
   */
  void begin(uint8_t _channel, uint16_t _node_id );
  
  /**
  * Check connection, network ready to send/recieve 
  * payload message
  */
  bool ready(void);

  /**
  * Send payload message to node by unique id.
  */
  bool send(const void* message, uint16_t to_id);

  /**
  * Update node, handle new messages and
  * support internal comunication
  */
  void update(void);

  /**
  * Check message box for new payload message
  */
  bool available(void);

  /**
  * Read available messages, get payload message
  */
  void read(void*);

private:
  RF24& radio; /**< Underlying radio driver, provides link/physical layers */
  RF24Network network; /**< RF24Network layer */ 
  uint16_t node_address; /**< Logical node address of this unit */
  uint16_t node_id; /**< Node id of this unit */
  uint8_t channel; /**< The RF channel to operate on */
  const static uint16_t base = 00; /**< Base address */
  const static uint16_t homeless = 05; /**< homeless address is last address in the network */
  const static uint16_t interval = 2000; /**< Delay manager in ms */
  unsigned long last_time_sent; /** time's stamp when last message was sent */
  bool state_ready; /**< connection state */

  /**
  * Send message with type P, send Ping request.
  * Send ping to base.
  */
  bool send_P();

  /**
  * Handle message with type P, handle Ping request.
  * Save node id to collection or given new address.
  */
  void handle_P(RF24NetworkHeader& header);

  /**
  * Send message with type A, send new Address node request.
  * Request new address from base
  */
  bool send_A(uint16_t new_address);

  /**
  * Handle message with type A, handle new Address node request.
  * Reinitialize node with new address from message.
  */
  void handle_A(RF24NetworkHeader& header);

  /**
  * Find empty node address
  * 
  * @param relay_address Find inside this relay branch,
  * 	by default finding in the root
  */
  uint16_t get_new_address(uint16_t relay_address);

  /**
  * Reinitialize node with new address, 
  * and send ping from new address
  */
  void set_address(uint16_t new_address);
  
  /**
  * Reset node
  */
  void flush_node();
};
#endif // __MESH_H__
