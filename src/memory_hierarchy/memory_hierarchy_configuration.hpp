#ifndef MEM_HIER_CFG_H
#define MEM_HIER_CFG_H

#include "tlm.h"
#include <cstdint>
#include <cmath>


enum Operation {READ, WRITE, INVALIDATE, DATA_REPLY, EVICTION, WRITE_BACK, DOWNGRADE};
enum Component {CPU, CACHE, DIRECTORY, MEMORY};


struct Event: tlm::tlm_extension<Event>
{
    Event():timestamp(0){};
    ~Event();
    uint64_t  timestamp;
    Operation operation;
    uint64_t  address;
    Component destination;
    Component source;

    tlm::tlm_extension_base* clone() const;
    void copy_from(tlm::tlm_extension_base const &ext);
};


/*******************************************************
 * SYSTEM CONFIGURATION
 ******************************************************/

//80: Define number of CPUs in the system
#define N_CPUS 2

//101: Define arithmetic density
#define DENSITY 5

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
 * Directory Configuration
 ******************************************************/

//85: Define memory size in bytes
#define MEM_SIZE 10485760

//86: Define address offset
#define OFFSET 6358656

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
