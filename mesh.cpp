
#include "mesh.h"
#include "stl_map.h"



  // last address in the network, homeless address
  uint8_t homeless = 05555;
  uint8_t base = 00000;
  // current node address
  uint8_t this_node = homeless;
  
/*
 * Test std::map
 */
 
struct TestMap {
 
  static void RunTest() {

 
    std::map<int,const char *> days;
    int i;
 
    days[1]="Monday";
    days[2]="Tuesday";
    days[3]="Wednesday";
    days[4]="Thursday";
    days[5]="Friday";
    days[6]="Saturday";
    days[7]="Sunday";
 
    for(i=1;i<7;i++)
      
  }
};

/****************************************************************************/

mesh::mesh( RF24Network& _network ): network(_network) 
{
}

/****************************************************************************/

void mesh::begin(uint8_t _channel, uint16_t _id)
{
  // initialize network
  network.begin(_channel, this_node);
}

/****************************************************************************/

bool mesh::send(const void* message, uint16_t to_id)
{
  uint16_t to = map.find(to_id);
  RF24NetworkHeader header(to, 'M');

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

void mesh::update()
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

    if(this_node == homeless) {
      send_A(base);
    }
  }
}

/****************************************************************************/

bool mesh::available()
{
  return network.available();
}

/****************************************************************************/

Message mesh::read()
{
  RF24NetworkHeader header;
  network.peek(header);

  if(header.type == 'M') {
    network.read(header,&message,sizeof(unsigned long));
  }
  return message;
}

/****************************************************************************/

void mesh::handle_A(RF24NetworkHeader& header)
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

bool mesh::send_A(uint16_t to)
{
  RF24NetworkHeader header(to, 'A');
  return network.write(header,to,sizeof(to));
}

/****************************************************************************/

void mesh::handle_I(RF24NetworkHeader header)
{
  uint16_t id;
  network.read(header,&id,sizeof(id));

  map.update(id, header.from_node);
}

/****************************************************************************/

bool mesh::send_I()
{
  RF24NetworkHeader header(base, 'I');
  return network.write(header,id,sizeof(id));
}

/****************************************************************************/

uint16_t mesh::get_address(uint16_t relay_address)
{
  if(this_node != base) {
    
  }
}

/****************************************************************************/

void mesh::set_address(uint16_t address)
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
