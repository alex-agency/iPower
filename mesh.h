
#ifndef __MESH_H__
#define __MESH_H__

#include <SPI.h>
#include "RF24Network.h"
#include "HashMap.h"
#include "timer.h"

/**
 * Payload which is sent as main message
 *
 * This object mapping key to value.
 * Size of payload cannot be more than 24 byte.
 */
struct Payload
{
    char key[20];
    int value;

    /**
     * Default constructor
     *
     * Simply constructs a blank payload
     */
    Payload() {}

    /**
     * Message constructor to create object 
     * and set values of key and value.
     *
     * @code
     *  Payload payload("temperature", 24);
     *  mesh.send(payload, base_id);
     * @endcode
     */
    Payload(const char* _key, int _value) {
      strcpy(key, _key);
      value = _value;
    }

    /**
     * Create debugging string
     *
     * @return String representation of this object
     */
    const char* toString(void) const;
};

/**
 * Mesh Network Layer for RF24 Network
 *
 * This class implements an Mesh Network Layer using nRF24L01(+) radios driven
 * by RF24 and RF24Network libraries.
 *
 * Currently MESH is not working correct!
 * We can't send message from 05555 node to 00 base!
 * Currently this network could have only 4 leaf node. (01-04)
 *
 * leaf node   -> base node
 * 01-05       -> 00
 * 021         -> 01
 * 051-055     -> 01-05
 * 0551-0555   -> 051-055
 * 05551-05555 -> 0551-0555
 * 05555       -> 0555
 * 
 * 1. Base node after start has base address (00) and waiting for messages.
 * 2. Leaf node after start has homeless address (05) which is last address 
 *    in the network.
 * 3. Homeless node sending ping to base node (message with type P) 
 *    every time interval.
 * 4. When base is recieved ping from homeless node then base sent to
 *    this node message with new address (type A) which is not used 
 *    by any other node.
 * 5. The homeless node applying new address from recived message (with type A).
 * 6. After applying address leaf node send to base ping with its unique id.
 * 7. If leaf node was sent ping corretly it means this leaf node is ready.
 * 8. When base is recieved ping from leaf node then base getting uiniqe id from 
 *    ping message and putting it to nodes map. Where key is unique id and value
 *    is address of this leaf.
 * 9. When leaf node has ready state it can send payload message to base.
 * 10.If leaf node can't send correctly payload message it should reset its address.
 *   
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
  bool send(Payload& payload, uint16_t to_id);

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
  void read(Payload& payload);

private:
  RF24& radio; /**< Underlying radio driver, provides link/physical layers */
  RF24Network network; /**< RF24Network layer */ 
  uint16_t node_address; /**< Logical node address of this unit */
  uint16_t node_id; /**< Node id of this unit */
  uint8_t channel; /**< The RF channel to operate on (0-127) */
  HashMap<uint16_t, uint16_t, 10> nodes; /**< HashMap that pairs id to address and can hold number pairs. */
  const static uint16_t base = 00; /**< Base address */
  const static uint16_t homeless = 05; /**< homeless address is last address in the network */
  //timer_t send_timer(10000); /**< Delay manager in ms */
  bool ready_to_send; /**< connection state */

  /**
  * Send message with type P, send Ping.
  * Send ping to base.
  */
  bool send_P();

  /**
  * Handle message with type P, handle Ping request.
  * Save node id to collection or given new address.
  */
  void handle_P(RF24NetworkHeader& header);

  /**
  * Send message with type A, message with Address.
  * Request for re-init with new address
  */
  bool send_A(const uint16_t new_address);

  /**
  * Handle message with type A, handle new Address request.
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
  void reset_node();
};

#endif // __MESH_H__
