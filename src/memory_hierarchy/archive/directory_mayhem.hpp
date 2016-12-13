#ifndef CACHE_L2_H
#define CACHE_L2_H

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include "memory_hierarchy_configuration.hpp"
#include "cache_transaction.hpp"
#include <vector>

/***********************************************************
 * CLASS CacheL2
 *----------------------------------------------------------
 **********************************************************/
class CacheL2
{
    
};



/***********************************************************
 * CLASS DirectoryEntry
 *----------------------------------------------------------
 **********************************************************/
class DirectoryEntry
{
public:
    DirectoryEntry();
    BlockState       state;
    bool             sharers[N_CPUS];
    int              owner;
};



/***********************************************************
 * CLASS Directory
 *----------------------------------------------------------
 * The specifications for the directory can be found in: 
 *      Hennesy & Patterson, Computer Architecture : A Quantitative Approach (Fifth),
 *      p. 380-386
 **********************************************************/
class Directory : public sc_core::sc_module
{
    

public:
    Directory( sc_core::sc_module_name );
    
    tlm_utils::simple_initiator_socket_tagged<Directory,32,cache_coherence_protocol>* i_socket[N_CPUS];
    tlm_utils::simple_target_socket_tagged<Directory,32,cache_coherence_protocol>*    t_socket[N_CPUS];

    std::vector<DirectoryEntry> dir;
    std::vector<int>            memory;
    
    void                   respondToL1Caches( int, CacheTransaction&, sc_core::sc_time& );
    MemoryManager          &manager;
    unsigned char dummy;
};







/*
Writing back the data value whenever the block becomes shared
simplifies the number of states in the protocol since 
     a. Any dirty block must be exclusive 
     b. Any shared block is always available in the home memory.
*/








#endif
