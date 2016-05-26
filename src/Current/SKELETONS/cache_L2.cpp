#include "cache_L2.hpp"

// Currently working on respondToInv
  

void Directory::respondToL1Caches(ProtocolResponse *resp)
{
    CoherenceMessageType msg = resp->msg;
    Address                i = resp->address >> BLOCK_SIZE;
     
    switch (msg) {
    case INV:
    {
	dir[i].respondToInv(resp);
	break;
    }
    default:
	break;
    }

}

void DirectoryEntry::respondToInv( ProtocolResponse *resp)
{
    
    state = MODIFIED;
}
