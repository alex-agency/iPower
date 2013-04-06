
#include "Mesh.h"
#include "HashMap.h"
#include "Payload.h"

// Debug info
const bool DEBUG = true;

/**
 * Nodes 
 * 
 * HashMap that pairs id to address and can hold number pairs
 */
CreateHashMap(nodes, uint16_t, uint16_t, 5);

/****************************************************************************/

Mesh::Mesh( RF24& _radio ): radio(_radio), network(radio) {}

/****************************************************************************/

void Mesh::begin(uint8_t _channel, uint16_t _node_id)
{
  // save settings
  node_id = _node_id;
  channel = _channel;
  // is it base node?
  if( node_id == base ) {
    node_address = base;
  }
  // otherwise it is homeless
  else {
    node_address = homeless;
  }
  if(DEBUG) printf_P(PSTR("MESH: Info: Initializing Node: id: %u, address: 0%o \n\r"),
              node_id, node_address);
  // set address
  network.begin(channel, node_address);
}

/****************************************************************************/

bool Mesh::ready()
{
  return state_ready;
}

/****************************************************************************/

bool Mesh::send(const Payload payload, uint16_t to_id)
{
  if(to_id != base && nodes.contains(to_id) == false) {
    // we should know about this node
    return false;
  }
  // get destination address
  uint16_t to_address = nodes[to_id];
  RF24NetworkHeader header(to_address, 'M');
  
  if(DEBUG) printf_P(PSTR("MESH: Info: %u, 0%o: Sending payload: %s \n\r"), 
              node_id, node_address, header.toString());

  bool ok = network.write(header,&payload,sizeof(payload));
  if(ok) {
    return true;
  // we won't reset base
  } else if(node_id == base) {
    return false;
  // reset node if it can't send
  } else {
    reset_node();
    return false;
  }
}

/****************************************************************************/

void Mesh::update()
{
  // update RF24Network
  network.update();
  
  // Is there anything ready?
  if ( network.available() ) {
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
    // if homeless send ping to base
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

void Mesh::read(Payload& payload)
{
  RF24NetworkHeader header;
  network.peek(header);

  if(header.type == 'M') {
    network.read(header,&payload,sizeof(payload));
  }
}

/****************************************************************************/

bool Mesh::send_P()
{
  RF24NetworkHeader header(base, 'P');
  if(DEBUG) printf_P(PSTR("MESH: Info: %u, 0%o: Sending ping: %s \n\r"), 
              node_id, node_address, header.toString());
  return network.write(header,&node_id,sizeof(node_id));
}

/****************************************************************************/

void Mesh::handle_P(RF24NetworkHeader& header)
{
  uint16_t id;
  network.read(header,&id,sizeof(id));
  
  if(header.from_node == homeless) {
    uint16_t new_address;
    if(nodes.contains(id)) {
      new_address = nodes[id];
    } else {
      // find next empty address
      new_address = get_new_address(base);
    }
    // send new address to homeless
    send_A(new_address);
    return;
  }
  // add new or update existing node
  nodes[id] = header.from_node;
  if(DEBUG) {
    printf_P(PSTR("MESH: Info: %u, 0%o: Node is updated its map: "), 
              node_id, node_address);
    nodes.print();
    printf_P(PSTR("\n\r"));
  }
}

/****************************************************************************/

bool Mesh::send_A(const uint16_t new_address)
{
  RF24NetworkHeader header(homeless, 'A');
  if(DEBUG) printf_P(PSTR("MESH: Info: %u, 0%o: Sending new address '0%o': %s \n\r"), 
              node_id, node_address, new_address, header.toString());
  return network.write(header,&new_address,sizeof(new_address));
}

/****************************************************************************/

void Mesh::handle_A(RF24NetworkHeader& header)
{
  uint16_t new_address;
  network.read(header,&new_address,sizeof(new_address));
  // reinitialize node
  set_address(new_address);
}

/****************************************************************************/

uint16_t Mesh::get_new_address(uint16_t relay)
{
  // find next after relay empty address
  bool exists = false;
  for(uint16_t address=relay+1; address<homeless; address++) {
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
    // reset state
    reset_node();
  }
}

/****************************************************************************/

void Mesh::reset_node()
{ 
  // clear knowing nodes
  for(int index=0; index<nodes.size(); index++) {
    nodes.remove(index);
  }
  // change connection state
  state_ready = false;
  // reset setting
  node_address = homeless;
  
  if(DEBUG) printf_P(PSTR("MESH: Info: %u, 0%o: Node is flashed. \n\r"), 
              node_id, node_address);
  // reinitialize node
  network.begin(channel, homeless);
}

/****************************************************************************/
