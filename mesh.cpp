
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
  channel = _channel;
  
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
  
  printf_P(PSTR("MESH: Info: Initializing Node: id: %u, address: 0%o \n\r"),
            node_id, node_address);
  network.begin(channel, node_address);
}

/****************************************************************************/

bool Mesh::ready()
{
  return state_ready;
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
    
    printf_P(PSTR("MESH: Info: %u, 0%o: Got message: %s \n\r"),
              node_id, node_address, header.toString());
    
    // Dispatch the message to the correct handler.
    switch (header.type)
    {
      case 'P':
        handle_P(header);
        break;
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
  if ( now >= interval + last_time_sent )
  {
    last_time_sent = now;
    
    if(node_address == homeless) {
      // send ping to base
      bool ok = send_P(base);
      
      if(ok) {
        printf_P(PSTR("MESH: Info: %u, 0%o: Ping base: Send: ok \n\r"),
                  node_id, node_address);
      } else {
        printf_P(PSTR("MESH: Info: %u, 0%o: Ping base: Send: failed \n\r"),
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

void Mesh::handle_P(RF24NetworkHeader& header)
{
  uint16_t address;
  network.read(header,&address,sizeof(address));
  // check if request received direct or through relay
  if(address == header.from_node) {
    address = get_new_address(base);
  } else {
    address = get_new_address(address); 
  }
  
  if(header.from_node == homeless) {
    // send next empty address to node
    send_A(address);
  }
}

/****************************************************************************/

bool Mesh::send_P(uint16_t to_address)
{
  RF24NetworkHeader header(to_address, 'P');
  printf_P(PSTR("MESH: Info: %u, 0%o: Send P, Header: %s \n\r"), 
    node_id, node_address, header.toString());
  return network.write(header,&to_address,sizeof(to_address));
}

/****************************************************************************/

void Mesh::handle_A(RF24NetworkHeader& header)
{
  uint16_t address;
  network.read(header,&address,sizeof(address));
  
  // reinitialize node
  set_address(address);
}

/****************************************************************************/

bool Mesh::send_A(uint16_t new_address)
{
  RF24NetworkHeader header(homeless, 'A');
  printf_P(PSTR("MESH: Info: %u, 0%o: Send A, Header: %s \n\r"), 
    node_id, node_address, header.toString());
  return network.write(header,&new_address,sizeof(new_address));
}

/****************************************************************************/

void Mesh::handle_I(RF24NetworkHeader& header)
{
  uint16_t id;
  network.read(header,&id,sizeof(id));
  // add new or update existing node
  nodes[id] = header.from_node;
  
  printf_P(PSTR("MESH: Info: %u, 0%o: Node is updated its map: %s \n\r"), 
    node_id, node_address, nodes.toString());
}

/****************************************************************************/

bool Mesh::send_I()
{
  RF24NetworkHeader header(base, 'I');
  printf_P(PSTR("MESH: Info: %u, 0%o: Send I, Header: %s \n\r"), 
    node_id, node_address, header.toString());
  return network.write(header,&node_id,sizeof(node_id));
}

/****************************************************************************/

uint16_t Mesh::get_new_address(uint16_t relay_address)
{
  // find next after relay empty address
  bool exists = false;
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
  node_address = address;
  
  printf_P(PSTR("MESH: Info: Reinitializing Node: id: %u, new address: 0%o \n\r"),
    node_id, node_address);
  network.begin(channel, node_address);
  
  // send unique ID to base 
  bool ok = send_I();
  if(ok) {
    // change connection state
    state_ready = true;
    printf_P(PSTR("MESH: Info: %u, 0%o: Send unique ID to base: ok \n\r"),
                  node_id, node_address);
  } else {
    printf_P(PSTR("MESH: Info: %u, 0%o: Send unique ID to base: failed \n\r"),
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
  // change connection state
  state_ready = false;
  
  printf_P(PSTR("MESH: Info: %u, 0%o: Node is flashed. \n\r"), node_id, node_address);
  // reinitialize node
  network.begin(channel, homeless);
}

/****************************************************************************/
