#ifndef MEM_HIER_CFG_H
#define MEM_HIER_CFG_H


#include <cmath>
//#include "tlm.h"
//#include "tlm_utils/multi_passthrough_initiator_socket.h"
//#include "tlm_utils/multi_passthrough_target_socket.h"

//using Address  = unsigned int;
//using SetIndex = unsigned int;
//using BlockTag = unsigned int;
//using uint     = unsigned int;

/*******************************************************
 * SYSTEM CONFIGURATION
 ******************************************************/

//80: Define number of CPUs in the system
#define N_CPUS 2


/*******************************************************
 * L1 Cache Configuration
 ******************************************************/

//81: Define cache size in bytes
#define CACHE_SIZE 32768

//82: Define cache block size in bytes
#define BLOCK_SIZE 64

//83: Define cache associativity
#define N_WAYS 8

//84: Define replacement policy
#define POLICY 1


/*******************************************************
 * Latencies
 ******************************************************/

//85: Define L1 miss in cycles
#define L1_MISS 5





constexpr unsigned int numSets (unsigned int cacheSize, unsigned int blockSize, unsigned int ways)
{
    return cacheSize/(blockSize*ways);
}

/*******************************************************
 * L2 Cache & Memory Configuration
 ******************************************************/

//85: Define memory size in bytes
#define MEM_SIZE 10485760

constexpr unsigned int numMemBlocks (unsigned int memSize, unsigned int blockSize)
{
    return memSize/blockSize;
}



/*******************************************************
 * GENERIC HELPER FUNCTIONS
 *******************************************************/

using Address  = unsigned int;
using SetIndex = unsigned int;
using BlockTag = unsigned int;

inline void getSetIndexAndTag(Address a, SetIndex *setIndex, BlockTag *blockTag)
{
    // Discard byte index
    Address discardedBlockOffset = a >> (unsigned int)(log2(BLOCK_SIZE) ) ;

    // Create set index mask
    Address setIndexLength = log2(numSets(CACHE_SIZE, BLOCK_SIZE, N_WAYS));
    Address setIndexMask   = numSets(CACHE_SIZE, BLOCK_SIZE, N_WAYS) - 1;

    // Find set index and block tag
    *blockTag = discardedBlockOffset >> setIndexLength;
    *setIndex = setIndexMask & discardedBlockOffset;
}




#endif
