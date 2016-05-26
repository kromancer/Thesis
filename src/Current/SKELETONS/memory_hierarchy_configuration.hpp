#ifndef MEM_HIER_CFG_H
#define MEM_HIER_CFG_H

#include <cmath>

using Address  = unsigned int;
using SetIndex = unsigned int;
using BlockTag = unsigned int;
using uint     = unsigned int;

enum BlockState             {INVALID, SHARED, MODIFIED};
enum CoherenceMessageType   {INV, FETCH_INV, FETCH, READ_MISS, WRITE_MISS};
enum Operation              {READ, WRITE};

// Emulates a generic payload?
struct ProtocolResponse
{
    Address              address;
    Operation            op;
    unsigned char       *bytes;
    CoherenceMessageType msg;
};


/*******************************************************
 * L1 Cache Configuration
 ******************************************************/

//80: Define number of CPUs in the system
#define N_CPUS 2


/*******************************************************
 * L1 Cache Configuration
 ******************************************************/

//81: Define cache size in bytes
#define CACHE_SIZE 32768

//82: Define cache block size
#define BLOCK_SIZE 2

//83: Define cache associativity
#define N_WAYS 1

//84: Define replacement policy
#define POLICY 1

constexpr uint numSets (uint cacheSize, uint blockSize, uint ways)
{
    return cacheSize/(blockSize*ways);
}

/*******************************************************
 * L2 Cache & Memory Configuration
 ******************************************************/

//85: Define memory size in bytes
#define MEM_SIZE 1073741824

constexpr uint numMemBlocks (uint memSize, uint blockSize)
{
    return memSize/blockSize;
}



/*******************************************************
 * GENERIC HELPER FUNCTIONS
 ******************************************************/
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
