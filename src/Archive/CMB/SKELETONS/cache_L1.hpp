#ifndef CACHE_L1_H
#define CACHE_L1_H

#include <string>

using namespace std;
using Address  = unsigned int;
using SetIndex = unsigned int;
using BlockTag = unsigned int;

enum BlockState             {INVALID, SHARED, MODIFIED};
enum CoherenceMessageType   {INV, FETCH_INV, FETCH, READ_MISS, WRITE_MISS};
enum Operation              {READ, WRITE};

//81: Define cache size in bytes
#define CACHE_SIZE 32768

//82: Define cache block size
#define BLOCK_SIZE 2

//83: Define cache associativity
#define N_WAYS 1

//84: Define replacement policy
#define POLICY 1


constexpr int numSets (int cacheSize, int blockSize, int ways) { return cacheSize/(blockSize*ways); }


// Emulates a generic payload?
struct ProtocolResponse
{
    Address       a;
    Operation     op;
    unsigned char *bytes;
    CoherenceMessageType msg;
};


/***********************************************************
 * CLASS BLOCK
 *----------------------------------------------------------
 * Models the individual block stored in the cache
 * Maintains the state machine of the block.
 *
 * The block changes states in response to actions from
 *     1. CPU: [read|write] [miss|hit] )
 *     2. The directory: invalidate|fetch|fetch invalidate
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
    bool pending;
    
    void respondInvalidate( ProtocolResponse *resp);
    void respondFetch( ProtocolResponse *resp);
    void respondCpuReadMiss( ProtocolResponse *resp);
    void respondCpuWriteHit( ProtocolResponse *resp);
    void setDirty();
    int  getTag();
    
    bool   dirty;


    
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
    int    numOfBlocks;
    void   evict( ProtocolResponse *resp);
    void   insert( Block newBlock );
    Block *getBlock(BlockTag);
};

/***********************************************************
 * CLASS CacheL1
 *----------------------------------------------------------
 * Models a N-way associative cache meant to operate under
 * a directory based coherence protocol
 **********************************************************/
class CacheL1 
{
public:
    CacheL1();
    void getSetIndexAndTag( Address, SetIndex*, BlockTag*);
    void respondToDirectory( CoherenceMessageType, Address, ProtocolResponse*  );
    void respondToCPU(       Operation,            Address, ProtocolResponse* );
    bool isBlockPresent( SetIndex, BlockTag );
    
private:
    Set sets[ numSets(CACHE_SIZE, BLOCK_SIZE, N_WAYS) ];
};




#endif
