
#include "Mesh.h"
#include "HashMap.h"

/**
 * Nodes 
 * 
 * HashMap that pairs id to address and can hold 10 pairs
 */
CreateHashMap(nodes, uint16_t, uint16_t, 10);

/****************************************************************************/

Mesh::Mesh( RF24& _radio ): radio(_radio), network(radio) {}

/****************************************************************************/

void Mesh::begin(uint8_t _channel, uint16_t _node_id)
{
  node_id = _node_id;
  if( nodes.contains(node_id) )
  {
    node_address = nodes[node_id];
  } 
  else if( node_id == base ) 
  {
    node_address = base;
  }
  else 
  {
    node_address = homeless;
  }
  
  printf_P(PSTR("Initializing Node: id: %u, address: %u"),
            node_id, node_address);
  
  network.begin(_channel, node_address);
}

/****************************************************************************/

bool Mesh::send(const void* message, uint16_t to_id)
{
  if( nodes.contains(to_id) == false ) {
    return false;
  }
  // get destination address
  uint16_t to_address = nodes[to_id];
  RF24NetworkHeader header(to_address, 'M');
  
  bool ok = network.write(header,&message,sizeof(message));
  if(ok) {
    return true;
  } else {
    // reset
    flush_node();
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
    
    printf_P(PSTR("%u, %u: Got message: %s from %u \n\r"),
              node_id, node_address, header.type, header.from_node);
    
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
  
  // Send every 'interval' ms 
  unsigned long now = millis();
  if ( now - last_time_sent >= interval )
  {
    last_time_sent = now;
    
    if(node_address == homeless) {
      // send request to base for giving address
      bool ok = send_A(base);
      
      if(ok) {
        printf_P(PSTR("%u, %u: Address request: Send: ok\n\r"),
                  node_id, node_address);
      } else {
        printf_P(PSTR("%u, %u: Address request: Send: failed\n\r"),
                  node_id, node_address);
      }
    }
  }
}

/****************************************************************************/

bool Mesh::available()
{
  if( network.available() ) {
    RF24NetworkHeader header;
    network.peek(header);
    
    if(header.type == 'M') {
      return true;
    }
  }
  return false;
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
  uint16_t address;
  network.read(header,&address,sizeof(address));
  // check if request received direct or through relay
  if(address == header.from_node) {
    address = get_new_address(base);
  } else {
    address = get_new_address(address); 
  }
  // reinitialize node
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
  // add new or update existing node
  nodes[id] = header.from_node;
  
  printf_P(PSTR("%u, %u: Node is updated its map: %s"), 
    node_id, node_address, nodes.toString());
}

/****************************************************************************/

bool Mesh::send_I()
{
  RF24NetworkHeader header(base, 'I');
  return network.write(header,&node_id,sizeof(node_id));
}

/****************************************************************************/

uint16_t Mesh::get_new_address(uint16_t relay_address)
{
  // find next after relay empty address
  bool exists;
  for(uint16_t address=relay_address+1; address<homeless; address++) {
    for(int index=0; index<nodes.size(); index++) {
      if( nodes.valueAt(index) == address ) {
         exists = true;
      }
    }
    if(exists == false) {
      return address;
    }
  }
  return homeless;
}

/****************************************************************************/

void Mesh::set_address(uint16_t address)
{
  network.begin(channel, address);
  // send unique ID to base 
  bool ok = send_I();
  
  if(ok) {
    printf_P(PSTR("%u, %u: Send unique ID to base: ok\n\r"),
                  node_id, node_address);
  } else {
    printf_P(PSTR("%u, %u: Send unique ID to base: failed\n\r"),
                  node_id, node_address);
    // reset
    flush_node();
  }
}

/****************************************************************************/

void Mesh::flush_node()
{ 
  // clear knowing nodes
  for(int index=0; index<nodes.size(); index++) {
    nodes.remove(index);
  }
  
  printf_P(PSTR("%u, %u: Node is flashed.\n\r"), node_id, node_address);
  // reinitialize node
  network.begin(channel, homeless);
}

/****************************************************************************/
