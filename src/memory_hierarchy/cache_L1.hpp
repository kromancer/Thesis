#ifndef CACHE_L1_H
#define CACHE_L1_H

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include "memory_hierarchy_configuration.hpp"
#include "cache_transaction.hpp"

/***********************************************************
 * CLASS BLOCK
 *----------------------------------------------------------
 * Models the individual block stored in the cache
 * Maintains the state machine of the block.
 *
 * The block can be in 3 states:
 *     1. SHARED
 *     2. MODIFIED
 *     3. INVALID
 *
 * For the exact state transition diagram see:
 *    Hennessy & Patterson "Computer Architecture: A Quantitative Approach"
 *    Fifth Edition, p.383 fig 5.22
 **********************************************************/
class Block{
public:
    Block();
    BlockState state;
    BlockTag tag;
    unsigned char bytes[BLOCK_SIZE];    
};


/***********************************************************
 * CLASS SET
 *----------------------------------------------------------
 * Models the associative nature of the cache
 **********************************************************/
class Set{
public:
    Set();
    Block  ways[N_WAYS];
    bool   free[N_WAYS];
    bool   isSetFull();
    int    freeBlocks;

    void       insert( Block newBlock );
    Block     *getBlock(BlockTag, int&); // return a pointer to the block and in which way it is placed
    Block     &getFreeBlock();
    SetIndex   evict();
};


/***********************************************************
 * CLASS CacheL1
 *----------------------------------------------------------
 * Models a N-way associative cache meant to operate under
 * a directory based coherence protocol
 **********************************************************/
class CacheL1: public sc_core::sc_module
{
public:
    tlm_utils::simple_initiator_socket<CacheL1, 32, cache_coherence_protocol> i_socket; // To Directory
    tlm_utils::simple_target_socket<CacheL1,    32, cache_coherence_protocol> t_socket; // From Directory
    tlm_utils::simple_target_socket<CacheL1> t_socket_CPU; // From CPU
    
    CacheL1( sc_core::sc_module_name name );
    void respondToDirectory(    CacheTransaction&, sc_core::sc_time& );
    void respondToCPU(  tlm::tlm_generic_payload&, sc_core::sc_time& );
    
private:
    vector<Set>    sets;
    MemoryManager &manager;
};




#endif
