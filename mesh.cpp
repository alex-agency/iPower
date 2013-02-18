
/*


  Node is homeless (has address 99)
    1. Send ping to someone (00 .. 99)
    2. If ping sent save this address to active nodes
  
  Homeless node got active nodes from someone
    1. Check if we already know this address
    2. Set it as relay or base (00)
    3. 
    4. Reinitialize radio with new address
    5. Send payload message to base or relay
    
    
   2. Set own adrress as next address after sender, if it not exists in the list 
   3. Initialize new address
   4. Send payload message

   Node got ping from Homless
   1. Reply with nodes list

   Node got payload
   1. Add new node to list, if not exists

*/

#include "mesh.h"


