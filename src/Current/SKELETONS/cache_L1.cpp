#include "cache_L1.hpp"
#include <iostream>


//STATUS: working on CPU Write hit, block SM actions

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
 * The state of the block is also maintained in the directory.
 *
 * For the exact state transition diagram see:
 *    Hennessy & Patterson "Computer Architecture: A Quantitative Approach"
 *    Fifth Edition, p.383 fig 5.22
 **********************************************************/
Block::Block()
{
    tag   = 0;
}


int Block::getTag()
{
    return tag;
}


void Block::respondInvalidate( ProtocolResponse *resp)
{
    if (state == MODIFIED)
	resp->bytes = bytes;

    state = INVALID;
}


void Block::respondFetch( ProtocolResponse *resp)
{
    resp->bytes = bytes;
    state = SHARED;
}


void Block::respondCpuReadHit( ProtocolResponse *resp)
{
    if (state == INVALID)
	resp->msg = READ_MISS;
    else
	resp->bytes = bytes;

    state = SHARED;
}

// The Set Class has already decided that this block will be evicted
// see Set::evict()
void Block::respondCpuReadMiss( ProtocolResponse *resp)
{
    if (state == MODIFIED)
	resp->bytes = bytes;
    else
	resp->bytes = NULL;

    state = SHARED;
}

void Block::respondCpuWriteHit( ProtocolResponse *resp)
{
    resp->bytes = bytes;
    if (state == INVALID)
    {
	resp->msg   = WRITE_MISS;
	resp->bytes = NULL;
    }
    else if (state == SHARED)
	resp->msg = INV;

    state = MODIFIED;
}

void Block::respondCpuWriteMiss( ProtocolResponse *resp)
{
    if (state == MODIFIED)
	resp->bytes = bytes;	
    else
	resp->bytes = NULL;

    state = MODIFIED;
}







/***********************************************************
 * CLASS SET
 *----------------------------------------------------------
 * Models the associative nature of the cache
 **********************************************************/
Set::Set()
    : numOfBlocks(0)
{
    
}

Block* Set::getBlock(BlockTag tag)
{
    for(int i=0; i < N_WAYS; i++)
    {
	if (tag == ways[i].tag)
	{
	    return &ways[i];
	}
    }
    return NULL;
}

bool Set::isSetFull()
{
    if ( numOfBlocks == N_WAYS )
	return true;
    else
	return false;
}


SetIndex Set::evict()
{
    switch(POLICY)
    {
    case 1: // ALWAYS THE FIRST WAY
    {
	free[0] = true;
	return 0;
    }
    }
}






/***********************************************************
 * CLASS CacheL1
 *----------------------------------------------------------
 * Models a N-way associative cache meant to operate under
 * a directory based coherence protocol
 **********************************************************/
CacheL1::CacheL1()
{
    
}

Block* CacheL1::checkBlockPresence(SetIndex i, BlockTag t)
{
    Block *result = sets[i].getBlock(t);
    if ( result )
	return result;
    else
	return NULL;
}


void CacheL1::respondToDirectory( ProtocolResponse *resp )
{
    // Do the proper analysis of the address to extract set index and block tag
    Address              a  = resp->address;
    CoherenceMessageType m  = resp->msg;
    SetIndex setIndex;
    BlockTag blockTag;
    getSetIndexAndTag(a, &setIndex, &blockTag);


    // Remember that the directory knows whether the block is present or not
    // We do not have to do any presence checks
    switch (m)
    {
    case INV:
    {
	// Another CPU has issued a WRITE_MISS
	sets[setIndex].getBlock(blockTag)->respondInvalidate(resp);
	break;
    }
    case FETCH_INV:
    {
	// Another CPU has issued a WRITE_MISS
	// The directory knows that this cache has the block in MODIFIED
	sets[setIndex].getBlock(blockTag)->respondInvalidate(resp);
	break;
    }
    case FETCH:
    {
	// The directory knows that this cache has the block in MODIFIED.
	// Another CPU has issued a READ_MISS.
	sets[setIndex].getBlock(blockTag)->respondFetch(resp);
	break;
    }
    default:
	break;
    }
}

void CacheL1::respondToCPU( ProtocolResponse *resp )
{
    // Do the proper analysis of the address to extract set index and block tag
    Address  a = resp->address;
    Operation op = resp->op;
    SetIndex setIndex;
    BlockTag blockTag;
    getSetIndexAndTag(a, &setIndex, &blockTag);

    switch(op){
    //----------------------------------------------------------------------
    case READ:
    {
	Block *block = checkBlockPresence(setIndex, blockTag);
	// READ_HIT
	if ( block )
	    block->respondCpuReadHit(resp);
	// READ_MISS
	else
	{
	    resp->msg = READ_MISS;
	    resp->address = a;
	    if ( sets[setIndex].isSetFull() )
	    {
		// Cache must first EVICT
		int i = sets[setIndex].evict();
		sets[setIndex].ways[i].respondCpuReadMiss(resp);
		// Cache must now fetch and place memory block
	    }
	    else
	    {
		// Cache must fetch and place memory block
	    }
	}
	break;
    }
    //----------------------------------------------------------------------
    case WRITE:
    {
	Block *block = checkBlockPresence(setIndex, blockTag);
	// WRITE_HIT
	if ( block )
	    sets[setIndex].getBlock(blockTag)->respondCpuWriteHit( resp );
	// WRITE_MISS
	else
	{
	    resp->msg = WRITE_MISS;
	    if ( sets[setIndex].isSetFull() )
	    {
		// Cache must first EVICT
		int i = sets[setIndex].evict();
		sets[setIndex].ways[i].respondCpuWriteMiss(resp);
		// Cache must now fetch and place memory block
		
	    }
	    else
	    {
		
	    }
	}
	break;
    }

    }

}














int main(int argc, char *argv[])
{
    CacheL1 c;
    SetIndex i;
    BlockTag t;

    getSetIndexAndTag(0x4000, &i, &t);
    std::cout << "Set Index: " << i << " Block Tag: " << t << std::endl;

    return 0;
}
