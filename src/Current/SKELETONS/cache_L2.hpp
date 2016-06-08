#ifndef CACHE_L2_H
#define CACHE_L2_H

#include "tlm.h"
#include "tlm_utils/multi_passthrough_initiator_socket.h"
#include "tlm_utils/multi_passthrough_target_socket.h"
#include "memory_hierarchy_configuration.hpp"
#include "cache_transaction.hpp"


/***********************************************************
 * CLASS CacheL2
 *----------------------------------------------------------
 * Models a N-way associative L2 cache including a directory
 * for enforcing coherence in multicore systems 
 **********************************************************/
class CacheL2
{
    
};

/***********************************************************
 * CLASS DirectoryEntry
 *----------------------------------------------------------
 * We interpret INVALID as UNCACHED
 * We use 8-bits for the sharers, thus 8 CPUs max
 **********************************************************/

/***********************************************************
 * CLASS Directory
 *----------------------------------------------------------
 **********************************************************/
class Directory
{
    class DirectoryEntry
    {
    private:
	Directory    *parent;
	BlockState    state;
	unsigned char sharers;
    public:
	void respondToInv( CacheTransaction *resp );
    };


public:
    Directory();
    
    tlm_utils::multi_passthrough_initiator_socket<Directory> i_socket;
    tlm_utils::multi_passthrough_target_socket<Directory>    t_socket;
    
    DirectoryEntry   dir[ numMemBlocks(MEM_SIZE, BLOCK_SIZE) ];
    void             respondToL1Caches( CacheTransaction* );
    void             sendInv(uint l1cache);
    
    
};







/*
Writing back the data value whenever the block becomes shared
simplifies the number of states in the protocol since 
     a. Any dirty block must be exclusive 
     b. Any shared block is always available in the home memory.
*/








#endif
