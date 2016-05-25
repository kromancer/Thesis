#include "cache_L1.hpp"
#include <iostream>
#include <cmath>

//STATUS: working on CPU Write Miss from SHARED

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
    state = INVALID;
    switch (state)
    {
    case SHARED:
    {
	break;
    }
    case MODIFIED:
    {
	resp->bytes = bytes;
	break;
    }
    case INVALID:
    {
	cout << "This should not have happened: INV from INVALIDATE" << endl;
	break;
    }
    }
}


void Block::respondFetch( ProtocolResponse *resp)
{
    switch (state)
    {
    case MODIFIED:
    {
	state = SHARED;
	resp->bytes = bytes;
	break;
    }
    case SHARED:
    {
	cout << "This should not have happened: FETCH from SHARED" << endl;
	break;
    }
    case INVALID:
    {
	cout << "This should not have happened: FETCH from INVALID" << endl;
	break;
    }
    }
}


void Block::respondCpuReadMiss( ProtocolResponse *resp)
{
    switch (state) {
    case MODIFIED:
    {
	state = SHARED;
	pending = true;
	resp->bytes = bytes;
	break;
    }
    case SHARED:
    {
	break;
    }
    case INVALID:
    {
	break;
    }
    }
}

void Block::respondCpuWriteHit( ProtocolResponse *resp)
{
    switch (state) {
    case MODIFIED:
    {
	break;
    }
    case SHARED:
    {
	state = MODIFIED;
	resp->msg = INV;
	break;
    }
    case INVALID:
    {
	break;
    }
    }
}

void Block::respondCpuWriteMiss( ProtocolResponse *resp)
{
    switch (state) {
    case MODIFIED:
    {
	break;
    }
    case SHARED:
    {
	state = MODIFIED;
	resp->msg = INV;
	break;
    }
    case INVALID:
    {
	break;
    }
    }
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


void Set::evict( ProtocolResponse *resp)
{
    switch(POLICY)
    {
    case 1: // ALWAYS THE FIRST WAY
    {
	free[0] = true;
	ways[0].respondCpuReadMiss(resp);
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

bool CacheL1::isBlockPresent(SetIndex i, BlockTag t)
{
    if ( sets[i].getBlock(t) )
	return true;
    else
	return false;
}


void CacheL1::getSetIndexAndTag(Address a, SetIndex *setIndex, BlockTag *blockTag)
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



void CacheL1::respondToDirectory( CoherenceMessageType m, Address a, ProtocolResponse *resp )
{
    // Do the proper analysis of the address to extract set index and block tag
    SetIndex setIndex;
    BlockTag blockTag;
    getSetIndexAndTag(a, &setIndex, &blockTag);


    switch (m)
    {
    case INV:
    {
	sets[setIndex].getBlock(blockTag)->respondInvalidate(resp);
	break;
    }
    case FETCH_INV:
    {
	sets[setIndex].getBlock(blockTag)->respondInvalidate(resp);
	break;
    }
    case FETCH:
    {
	sets[setIndex].getBlock(blockTag)->respondFetch(resp);
	break;
    }
    default:
	break;
    }
}

void CacheL1::respondToCPU(Operation op, Address a, ProtocolResponse *resp)
{
    // Do the proper analysis of the address to extract set index and block tag
    SetIndex setIndex;
    BlockTag blockTag;
    getSetIndexAndTag(a, &setIndex, &blockTag);

    switch(op){
    case READ:
    {
	if ( isBlockPresent(setIndex, blockTag) )
	    ;
	else
	{
	    if ( sets[setIndex].isSetFull() )
	    {
		sets[setIndex].evict(resp);
	    }
	    else
	    {

	    }
	    resp->op = READ;
	    resp->a  = a;
	}
	break;
    }
    case WRITE:
    {
	if ( isBlockPresent(setIndex, blockTag) )
	    sets[setIndex].getBlock(blockTag)->respondCpuWriteHit( resp );
	else
	{

	    if ( sets[setIndex].isSetFull() )

		
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

    c.getSetIndexAndTag(0x4000, &i, &t);
    std::cout << "Set Index: " << i << " Block Tag: " << t << std::endl;

    return 0;
}
