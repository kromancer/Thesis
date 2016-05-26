#ifndef CACHE_L2_H
#define CACHE_L2_H

#include "memory_hierarchy_configuration.hpp"


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
class DirectoryEntry
{
private:
    BlockState    state;
    unsigned char sharers;
    void respondToInv( ProtocolResponse *resp );
    friend class Directory;
};

/***********************************************************
 * CLASS Directory
 *----------------------------------------------------------
 **********************************************************/
class Directory
{
public:
    DirectoryEntry   dir[ numMemBlocks(MEM_SIZE, BLOCK_SIZE) ];
    void             respondToL1Caches( ProtocolResponse*);
};







/*
Writing back the data value whenever the block becomes shared
simplifies the number of states in the protocol since 
     a. Any dirty block must be exclusive 
     b. Any shared block is always available in the home memory.
*/








#endif
