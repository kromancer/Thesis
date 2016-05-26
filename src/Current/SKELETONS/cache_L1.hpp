#ifndef CACHE_L1_H
#define CACHE_L1_H

#include "memory_hierarchy_configuration.hpp"

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
    
    void respondInvalidate(   ProtocolResponse *resp);
    void respondFetch(        ProtocolResponse *resp);
    void respondCpuReadHit(   ProtocolResponse *resp);
    void respondCpuReadMiss(  ProtocolResponse *resp);
    void respondCpuWriteHit(  ProtocolResponse *resp);
    void respondCpuWriteMiss( ProtocolResponse *resp);
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

    void       insert( Block newBlock );
    Block     *getBlock(BlockTag);
    SetIndex   evict();
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
    void respondToDirectory( ProtocolResponse*  );
    void respondToCPU(       ProtocolResponse* );
    Block *checkBlockPresence( SetIndex, BlockTag );
    
private:
    Set sets[ numSets(CACHE_SIZE, BLOCK_SIZE, N_WAYS) ];
};




#endif
