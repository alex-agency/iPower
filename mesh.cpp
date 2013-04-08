
#include "Mesh.h"
#include "HashMap.h"
#include "RF24.h"

// Debug info
const bool DEBUG = true;

// HashMap that pairs id to address and can hold number pairs
HashMap<uint16_t, uint16_t, 5> nodes;

// Payload message. Network has size limit of 24 byte per message.
struct Payload {
    char key[20];
    int value;

    const int& operator[](const char* _key) const {
      return operator[](_key);
    };
    
    int& operator[](const char* _key) {
      if (key == _key) {
        return value;
      }
      else {
        key = _key;
        value = int;
        return int;
      }
    };
    
    const char* toString() const {
        static char buffer[30];
        snprintf_P(buffer,sizeof(buffer),PSTR("{%s=%d}"), key, value);
        return buffer;   
    };
};

/****************************************************************************/

Mesh::Mesh( RF24& _radio ): radio(_radio), network(radio) {}

/****************************************************************************/

void Mesh::begin(uint8_t _channel, uint16_t _node_id)
{  
  // dynamic payload feature
  //radio.enableDynamicPayloads();
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
  
  if(DEBUG) printf_P(PSTR("MESH: Info: %u, %s, %d byte: Sending payload.\n\r"), 
              node_id, header.toString(), sizeof(payload));

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
  
  // Send message every 'interval' ms 
  unsigned long now = millis();
  if ( now >= interval + last_time_sent )
  {
    last_time_sent = now;
    // if homeless send ping to base
    if(node_address == homeless) {
      bool ok = send_P();
      if(ok) {
        if(DEBUG) printf_P(PSTR("MESH: Info: %u, 0%o: Ping: Sent: ok.\n\r"),
                    node_id, node_address);
      } else {
        if(DEBUG) printf_P(PSTR("MESH: Info: %u, 0%o: Ping: Sent: failed!\n\r"),
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
    uint8_t size = sizeof(payload);//radio.getDynamicPayloadSize();
    uint8_t received = network.read(header,&payload,size);
    if(DEBUG) printf_P(PSTR("MESH: Info: %u, %s, %d byte: Received payload.\n\r"),
                node_id, header.toString(), received);
  }
}

/****************************************************************************/

bool Mesh::send_P()
{
  RF24NetworkHeader header(base, 'P');
  if(DEBUG) printf_P(PSTR("MESH: Info: %u, %s, %d byte: Sending ping...\n\r"), 
              node_id, header.toString(), sizeof(node_id));
  return network.write(header,&node_id,sizeof(node_id));
}

/****************************************************************************/

void Mesh::handle_P(RF24NetworkHeader& header)
{
  uint16_t id;
  uint8_t size = sizeof(id);//radio.getDynamicPayloadSize();
  uint8_t received = network.read(header,&id,size);
  if(DEBUG) printf_P(PSTR("MESH: Info: %u, %s, %d byte: Received ping.\n\r"),
              node_id, header.toString(), received);
  
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
  if(DEBUG) printf_P(PSTR("MESH: Info: %u, 0%o: Node is updated its map: %s.\n\r"), 
              node_id, node_address, nodes.toString());
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
  uint8_t size = sizeof(new_address);//radio.getDynamicPayloadSize();
  uint8_t received = network.read(header,&new_address,size);
  if(DEBUG) printf_P(PSTR("MESH: Info: %u, %s, %d byte: Received address '0%o'.\n\r"),
              node_id, header.toString(), received, new_address);
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

  if(DEBUG) printf_P(PSTR("MESH: Info: Reinitializing Node: id: %u, new address: 0%o.\n\r"),
              node_id, node_address);
  // apply new address
  network.begin(channel, node_address);
  // send ping
  bool ok = send_P();
  if(ok) {
    // change connection state
    state_ready = true;
    if(DEBUG) printf_P(PSTR("MESH: Info: %u, 0%o: Ping: Send: ok.\n\r"),
                node_id, node_address);    
  } else {
    if(DEBUG) printf_P(PSTR("MESH: Info: %u, 0%o: Ping: Send: failed!\n\r"),
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
  
  if(DEBUG) printf_P(PSTR("MESH: Info: %u, 0%o: Node is flashed.\n\r"), 
              node_id, node_address);
  // reinitialize node
  network.begin(channel, homeless);
}

/****************************************************************************/
