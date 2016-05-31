#include "cache_L2.hpp"

// Currently working on respondToInv
Directory::Directory()
{
    
}

void Directory::respondToL1Caches(CacheTransaction *resp)
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

void Directory::DirectoryEntry::respondToInv( CacheTransaction *resp)
{
    for( int i=0; i<8; i++ )
    {
	if ( (sharers >> i) & 1 )
	    parent->sendInv(i);
    }

    state = INVALID;
}

void Directory::sendInv(uint l1Cache)
{
    i_socket[l1Cache]->b_transport();
}
