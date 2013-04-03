
#include "Mesh.h"
#include "HashMap.h"

// Debug info
const bool DEBUG = true;

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
  // save settings
  node_id = _node_id;
  channel = _channel;
  // check if we already know this node 
  if( nodes.contains(node_id) )
  {
    node_address = nodes[node_id];
  } 
  // it could be base node
  else if( node_id == base ) 
  {
    node_address = base;
  }
  // otherwise it will homeless
  else 
  {
    node_address = homeless;
  }

  if(DEBUG) printf_P(PSTR("MESH: Info: Initializing Node: id: %u, address: 0%o \n\r"),
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
    // we should know about this node
    return false;
  }
  // get destination address
  uint16_t to_address = nodes[to_id];
  RF24NetworkHeader header(to_address, 'M');
  
  bool ok = network.write(header,&message,sizeof(message));
  if(ok) {
    return true;
  // we won't reset base
  } else if(node_id == base) {
    return false;
  // reset node if it can't send
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
  while ( network.available() ) {
    RF24NetworkHeader header;
    network.peek(header);

    if(DEBUG) printf_P(PSTR("MESH: Info: %u, 0%o: Got message: %s \n\r"),
                node_id, node_address, header.toString());
    // Dispatch the message to the correct handler.
    switch (header.type) {
      case 'P':
        handle_P(header);
        break;
      case 'A':
        handle_A(header);
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
  
  // Send message every 'interval' ms 
  unsigned long now = millis();
  if ( now >= interval + last_time_sent )
  {
    last_time_sent = now;
    // send ping if our node is homeless
    if(node_address == homeless) {
      bool ok = send_P();
      if(ok) {
        if(DEBUG) printf_P(PSTR("MESH: Info: %u, 0%o: Ping: Send: ok \n\r"),
                    node_id, node_address);
      } else {
        if(DEBUG) printf_P(PSTR("MESH: Info: %u, 0%o: Ping: Send: failed! \n\r"),
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

bool Mesh::send_P()
{
  RF24NetworkHeader header(base, 'P');
  if(DEBUG) printf_P(PSTR("MESH: Info: %u, 0%o: Send ping: %s \n\r"), 
              node_id, node_address, header.toString());
  return network.write(header,&node_id,sizeof(node_id));
}

/****************************************************************************/

void Mesh::handle_P(RF24NetworkHeader& header)
{
  if(header.from_node == homeless) {
    // find next empty address
    uint16_t new_address = get_new_address(base);
    // send new address to homeless
    send_A(new_address);
    return;
  }

  uint16_t id;
  network.read(header,&id,sizeof(id));
  
  if(nodes[id].exsits()) {
    
  }
  
  // add new or update existing node
  nodes[id] = header.from_node;
  if(DEBUG) {
    printf_P(PSTR("MESH: Info: %u, 0%o: Node is updated its map: "), 
              node_id, node_address);
    nodes.toString();
    printf_P(PSTR("\n\r"));
  }
}

/****************************************************************************/

bool Mesh::send_A(uint16_t new_address)
{
  RF24NetworkHeader header(homeless, 'A');
  if(DEBUG) printf_P(PSTR("MESH: Info: %u, 0%o: Send new address 0%o: %s \n\r"), 
              node_id, node_address, new_address, header.toString());
  return network.write(header,&new_address,sizeof(new_address));
}

/****************************************************************************/

void Mesh::handle_A(RF24NetworkHeader& header)
{
  uint16_t new_address;
  network.read(header,&new_address,sizeof(new_address));
  // add new or update existing node
  nodes[id] = header.from_node;
  
  // reinitialize node
  set_address(new_address);
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
  // update setting
  node_address = address;

  if(DEBUG) printf_P(PSTR("MESH: Info: Reinitializing Node: id: %u, new address: 0%o \n\r"),
              node_id, node_address);
  // apply new address
  network.begin(channel, node_address);
  // send ping
  bool ok = send_P();
  if(ok) {
    // change connection state
    state_ready = true;
    if(DEBUG) printf_P(PSTR("MESH: Info: %u, 0%o: Ping: Send: ok \n\r"),
                node_id, node_address);    
  } else {
    if(DEBUG) printf_P(PSTR("MESH: Info: %u, 0%o: Ping: Send: failed! \n\r"),
                node_id, node_address);
    // reset node
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
  
  if(DEBUG) printf_P(PSTR("MESH: Info: %u, 0%o: Node is flashed. \n\r"), 
              node_id, node_address);
  // reinitialize node
  network.begin(channel, homeless);
}

/****************************************************************************/
