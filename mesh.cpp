
#include "Mesh.h"
#include "HashMap.h"

/**
 * Nodes 
 * 
 * HashMap that pairs id to address and can hold 10 pairs
 */
CreateHashMap(nodes, uint16_t, uint16_t, 10);

/****************************************************************************/

Mesh::Mesh( RF24& _radio ): radio(_radio), network(radio)
{
}

/****************************************************************************/

void Mesh::begin(uint8_t _channel, uint16_t _node_id)
{
  node_id = _node_id;
  node_address = nodes[node_id];
  
  // initialize network
  network.begin(_channel, node_address);
}

/****************************************************************************/

bool Mesh::send(const void* message, uint16_t to_id)
{
  uint16_t to_address = nodes[to_id];

  RF24NetworkHeader header(to_address, 'M');

  bool ok = network.write(header,&message,sizeof(message));
  if(ok) {
    return true;
  } else {
    // reinitialize network
    network.begin(channel, homeless);
    return false;
  }
}

/****************************************************************************/

void Mesh::update()
{
  // update RF24Network
  network.update();
  
  // Is there anything ready?
  while ( network.available() ) 
  {
    RF24NetworkHeader header;
    network.peek(header);

    // Dispatch the message to the correct handler.
    switch (header.type)
    {
      case 'A':
        handle_A(header);
        break;
      case 'I':
        handle_I(header);
        break;
      case 'M':
        // do not handle payload
        break;
      default:
        // Unknown message type
        // skip this message
        network.read(header,0,0);
        break;
    };
  }

  unsigned long now = millis();
  if ( now - last_time_sent >= interval )
  {
    last_time_sent = now;

    if(node_address == homeless) {
      send_A(base);
    }
  }
}

/****************************************************************************/

bool Mesh::available()
{
  return network.available();
}

/****************************************************************************/

void Mesh::read(void* message)
{
  RF24NetworkHeader header;
  network.peek(header);
  
  if(header.type == 'M') {
    network.read(header,&message,sizeof(unsigned long));
  }
}

/****************************************************************************/

void Mesh::handle_A(RF24NetworkHeader& header)
{
  uint16_t message;
  network.read(header,&message,sizeof(message));

  uint16_t address;
  if(message == header.from_node) {
    address = get_address(base);
  } else {
    address = get_address(message);
  }

  set_address(address);
}

/****************************************************************************/

bool Mesh::send_A(uint16_t to_address)
{
  RF24NetworkHeader header(to_address, 'A');
  return network.write(header,&to_address,sizeof(to_address));
}

/****************************************************************************/

void Mesh::handle_I(RF24NetworkHeader& header)
{
  uint16_t id;
  network.read(header,&id,sizeof(id));
  nodes[id] = header.from_node;
}

/****************************************************************************/

bool Mesh::send_I()
{
  RF24NetworkHeader header(base, 'I');
  return network.write(header,&node_id,sizeof(node_id));
}

/****************************************************************************/

uint16_t Mesh::get_address(uint16_t relay_address)
{
  if(node_id != base)
    return 0;
}

/****************************************************************************/

void Mesh::set_address(uint16_t address)
{
  // reinitialize network
  network.begin(channel, address);
  // send unique ID to base 
  bool ok = send_I();

  if(ok == false) {
    // reinitialize network
    network.begin(channel, homeless);
  }
}

/****************************************************************************/
