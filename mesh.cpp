
#include "Mesh.h"
#include "RF24.h"
#include "timer.h"

// Delay manager in ms
timer_t ping_timer(30000);

// Debug info
const bool DEBUG = true;

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
  if(DEBUG) printf_P(PSTR("MESH: Info: Initializing Node: id: %u, address: 0%o.\n\r"),
              node_id, node_address);
  // set address and channel
  network.begin(channel, node_address);
}

/****************************************************************************/

bool Mesh::ready()
{
  return ready_to_send;
}

/****************************************************************************/

bool Mesh::send(Payload& payload, uint16_t to_id)
{
  if(to_id != base && nodes.contains(to_id) == false) {
    // we should know about this node
    return false;
  }
  // get destination address
  uint16_t to_address = nodes[to_id];
  RF24NetworkHeader header(to_address, 'M');
  if(DEBUG) printf_P(PSTR("MESH: Info: %u, %s, %d byte: Sending payload to %d: %s..."), 
              node_id, header.toString(), sizeof(payload), to_id, payload.toString());
              
  bool ok = network.write(header,&payload,sizeof(payload));
  if(ok) {
    if(DEBUG) printf_P(PSTR(" ok.\n\r"));
    return true;
  // is it base?
  } else if(node_id == base) {
    if(DEBUG) printf_P(PSTR(" failed!\n\r"));
    // delete bad node
    nodes.remove(to_id);
    if(DEBUG) printf_P(PSTR("MESH: Info: %u: Node '%u' id is removed from address map!: %s.\n\r"), 
                node_id, to_id, nodes.toString());
    if(nodes.size() == 0)
      ready_to_send = false;
    return false;
  // otherwise reset our node
  } else {
    if(DEBUG) printf_P(PSTR(" failed! Reset!\n\r"));
    reset_node();
    return false;
  }
}

/****************************************************************************/

const char* Payload::toString() const {
  static char buffer[30];
  snprintf_P(buffer,sizeof(buffer),PSTR("{%s=%d}"), key, value);
  return buffer;   
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

    if(DEBUG) printf_P(PSTR("MESH: Info: %u, 0%o: Got message %c from 0%o.\n\r"),
                node_id, node_address, header.type, header.from_node);
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
    
  if ( ping_timer ) {
    // is it homeless?
    if(node_address == homeless) {
      //send ping to base
      send_P();
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
    uint8_t received = network.read(header,&payload,sizeof(payload));
    if(DEBUG) printf_P(PSTR("MESH: Info: %u, %s, %d byte: Received payload: %s.\n\r"),
                node_id, header.toString(), received, payload.toString());
  }
}

/****************************************************************************/

bool Mesh::send_P()
{
  RF24NetworkHeader header(base, 'P');
  if(DEBUG) printf_P(PSTR("MESH: Info: %u, %s, %d byte: Sending ping..."), 
              node_id, header.toString(), sizeof(node_id));
  bool ok = network.write(header,&node_id,sizeof(node_id));
  if(ok) {
    if(DEBUG) printf_P(PSTR(" ok.\n\r"));
  } else {
    if(DEBUG) printf_P(PSTR(" failed!\n\r"));
  }
  return ok;
}

/****************************************************************************/

void Mesh::handle_P(RF24NetworkHeader& header)
{
  uint16_t id;
  uint8_t received = network.read(header,&id,sizeof(id));
  if(DEBUG) printf_P(PSTR("MESH: Info: %u, %s, %d byte: Received ping from %d.\n\r"),
              node_id, header.toString(), received, id);
  // who send ping?
  if(header.from_node == homeless) {
    uint16_t new_address;
    // do we known this node?
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
  // if node isn't homeless we should add its to our network
  // add new or update existing node
  nodes[id] = header.from_node;
  if(DEBUG) printf_P(PSTR("MESH: Info: %u, 0%o: Address map is updated: %s.\n\r"), 
              node_id, node_address, nodes.toString());
  // change network state
  ready_to_send = true;
}

/****************************************************************************/

bool Mesh::send_A(const uint16_t new_address)
{
  RF24NetworkHeader header(homeless, 'A');
  if(DEBUG) printf_P(PSTR("MESH: Info: %u, %s, %d byte: Sending new address '0%o'.\n\r"), 
              node_id, header.toString(), sizeof(new_address), new_address);
  return network.write(header,&new_address,sizeof(new_address));
}

/****************************************************************************/

void Mesh::handle_A(RF24NetworkHeader& header)
{
  uint16_t new_address;
  uint8_t received = network.read(header,&new_address,sizeof(new_address));
  if(DEBUG) printf_P(PSTR("MESH: Info: %u, %s, %d byte: Received address '0%o'.\n\r"),
              node_id, header.toString(), received, new_address);
  // reinitialize node address
  set_address(new_address);
}

/****************************************************************************/

uint16_t Mesh::get_new_address(uint16_t relay)
{
  // find next after relay empty address
  bool exists = false;
  for(uint16_t address = relay + 1; address < homeless; address++) {
    for(int index = 0; index < nodes.size(); index++) {
      if(nodes.valueAt(index) == address) {
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
  if(DEBUG) printf_P(PSTR("MESH: Info: Reinitializing Node: id: %u, new address: 0%o.\n\r"),
              node_id, node_address);
  // apply new address
  network.begin(channel, node_address);
  // send ping to base
  bool ok = send_P();
  if(ok) {
    // change connection state
    ready_to_send = true;
  } else {
    // reset settings
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
  ready_to_send = false;
  // set as homeless
  node_address = homeless;
  if(DEBUG) printf_P(PSTR("MESH: Info: %u, 0%o: Node is flashed.\n\r"), 
              node_id, node_address);
  // reinitialize node
  network.begin(channel, homeless);
}

/****************************************************************************/
