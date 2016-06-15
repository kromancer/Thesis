#include "cache_L1.hpp"
#include <iostream>
#include <cassert>

/***********************************************************
 * CLASS BLOCK
 **********************************************************/
Block::Block()
    : state(INVALID),
      tag(-1)
{}



/***********************************************************
 * CLASS SET
 **********************************************************/

Set::Set()
    : freeBlocks(N_WAYS),
      free{ true }
{
    
}

Block* Set::getBlock(BlockTag tag, int &way)
{
    for(int i=0; i < N_WAYS; i++)
    {
	if (tag == ways[i].tag)
	{
	    way = i;
	    return &ways[i];
	}
    }
    way = -1;
    return nullptr;
}

bool Set::isSetFull()
{
    if ( freeBlocks == 0)
	return true;
    else
	return false;
}

 
SetIndex Set::evict()
{
    return rand() % N_WAYS;
}






/***********************************************************
 * CLASS CacheL1
 **********************************************************/

CacheL1::CacheL1()
    : sc_module("CacheL1"),
      sets( numSets(CACHE_SIZE, BLOCK_SIZE, N_WAYS) ),
      manager{ MemoryManager::GetSingleton() },
      i_socket("i_socket"),
      t_socket("t_socket"),
      t_socket_CPU("t_socket_CPU")
{
    t_socket.register_b_transport(     this, &CacheL1::respondToDirectory);
    t_socket_CPU.register_b_transport( this, &CacheL1::respondToCPU);
}


void CacheL1::respondToDirectory( CacheTransaction &trans, sc_core::sc_time &delay )
{
    // Do the proper analysis of the address to extract set index and block tag
    Address              a  = trans.address;
    CoherenceMessageType m  = trans.msg;
    SetIndex setIndex;
    BlockTag blockTag;
    getSetIndexAndTag(a, &setIndex, &blockTag);
    int way;
    Block *block = sets[setIndex].getBlock(blockTag, way);

    // Remember that the directory knows whether the block is present or not
    // There should not be any incosistency between the directory and the cache
    assert( block != nullptr && way != -1);
    switch (m)
    {
    case INV:
    {
	// Another CPU has issued a WRITE_MISS
	// The directory knows that this cache has the block in SHARED
	assert( sets[setIndex].freeBlocks < N_WAYS-1 );
	sets[setIndex].freeBlocks++;
	sets[setIndex].free[way] = true;
	block->state = INVALID;
	break;
    }
    case FETCH_INV:
    {
	// Another CPU has issued a WRITE_MISS
	// The directory knows that this cache has the block in MODIFIED
	assert( sets[setIndex].freeBlocks < N_WAYS-1 );
        sets[setIndex].freeBlocks++;
	sets[setIndex].free[way] = true;
	block->state = INVALID;
	trans.bytes  = block->bytes;
	break;
    }
    case FETCH:
    {
	// The directory knows that this cache has the block in MODIFIED.
	// Another CPU has issued a READ_MISS.
        block->state = SHARED;
	trans.bytes  = block->bytes;
	break;
    }
    default:
	break;
    }
}

void CacheL1::respondToCPU( tlm::tlm_generic_payload &trans, sc_core::sc_time &delay )
{
    // Do the proper analysis of the address to extract set index and block tag
    Address          a  = trans.get_address();
    tlm::tlm_command op = trans.get_command();
    SetIndex setIndex;
    BlockTag blockTag;
    getSetIndexAndTag(a, &setIndex, &blockTag);

    // If the block is not present we get a nullptr
    int way;
    Block *block = sets[setIndex].getBlock(blockTag, way);

    switch(op){
    //----------------------------------------------------------------------
    case READ: // Case 1
    {
	// Case 1.1: READ_HIT 
	if ( block )
	{
	    // Case 1.1.a: The block resides in the cache, but is in INVALID state, therefore READ_MISS
	    if (block->state == INVALID)
	    {
		// Issue a READ_MISS to the directory
		CacheTransaction trans2 = manager.acquire();
		sc_core::sc_time delay2(0, sc_core::SC_NS);
		trans2.address = a;
		trans2.msg     = READ_MISS;
		trans2.evicting = false;
		i_socket->b_transport(trans2, delay2);

		// Place block in Cache
		assert( sets[setIndex].freeBlocks > 1 );
		sets[setIndex].freeBlocks--;
		sets[setIndex].free[ way ] = false;
		block->tag = blockTag;
		block->state = SHARED;
		block->bytes[0] = trans2.bytes[0];
		block->bytes[1] = trans2.bytes[1];
		
		// Give the data back to the CPU
		trans.set_data_ptr(trans2.bytes);
	    }
	    // Case 1.1.b: The block resides in the cache
	    else
	    {
		// Give the data back to the CPU
		trans.set_data_ptr(block->bytes);
		// The state of the block remains the same, either MODIFIED or SHARED
	    }
	}
	// Case 1.2: READ_MISS
	else
	{
	    // Prepare a READ_MISS transaction to the directory
	    // Do not issue it yet, in case of eviction we need to provide more info
	    CacheTransaction trans2 = manager.acquire();
	    sc_core::sc_time delay2(0, sc_core::SC_NS);
	    trans2.address = a;
	    trans2.msg     = READ_MISS;


	    // Place block in Cache
	    // Case 1.2.a: Set is full, therefore we need to follow a replacement policy
	    if ( sets[setIndex].isSetFull() )
	    {
		// Cache must first EVICT
		int i = sets[setIndex].evict();
		BlockState state = sets[setIndex].ways[i].state;
		trans2.evicting = true;
		trans2.evictedBlockState = state;
		// If evicted's block state was MODIFIED, we need to write-back the data
		if (state == MODIFIED)
		    trans2.bytes = sets[setIndex].ways[i].bytes;
		// At last we can issue the transaction
		i_socket->b_transport(trans2, delay2);

		// Place data on Cache
		block->tag      = blockTag;
		block->bytes[0] = trans2.bytes[0];
		block->bytes[1] = trans2.bytes[1];
		block->state    = SHARED;
		
		// Give the data back to the CPU
		trans.set_data_ptr(block->bytes);

	    }
	    // Case 1.2.b: Set has at least one available spot
	    else
	    {
		// Issue READ_MISS to directory
		trans2.evicting = false;
		i_socket->b_transport(trans2, delay2);

                // Place data on Cache
		block->tag      = blockTag;
		block->bytes[0] = trans2.bytes[0];
		block->bytes[1] = trans2.bytes[1];
		block->state    = SHARED;
		
		// Give the data back to the CPU
		trans.set_data_ptr(block->bytes);

	    }
	}
	break;
    }
    //----------------------------------------------------------------------
    case WRITE: //Case 2
    {
	// Case 2.1: WRITE_HIT
	if ( block )
	{
	    // Case 2.1.a: The block resides in the cache, but is in INVALID state, therefore WRITE_MISS
	    if (block->state == INVALID)
	    {
		// Issue a WRITE_MISS to the directory
		CacheTransaction trans2 = manager.acquire();
		sc_core::sc_time delay2(0, sc_core::SC_NS);
		trans2.address  = a;
		trans2.msg      = WRITE_MISS;
		trans2.evicting = false;
		i_socket->b_transport(trans2, delay2);

		// Place block in Cache
		assert( sets[setIndex].freeBlocks > 1);
		sets[setIndex].freeBlocks--;
		sets[setIndex].free[ way ] = false;
		block->tag = blockTag;
		block->state = SHARED;
		block->bytes[0] = trans2.bytes[0];
		block->bytes[1] = trans2.bytes[1];
		
		// Give the data back to the CPU
		trans.set_data_ptr(trans2.bytes);

	    }
	}

	// WRITE_MISS
	else
	{
	    //trans->msg = WRITE_MISS;
	    if ( sets[setIndex].isSetFull() )
	    {
		// Cache must first EVICT
		int i = sets[setIndex].evict();

		// Cache must now fetch and place memory block
		
	    }
	    else
	    {
		
	    }
	}
	break;
    }
    // TLM_IGNORE_COMMAND is not handled, it does not make any sense
    default:
	break;
    }

}





/*************************************************
 * UNIT TEST
 *------------------------------------------------
 * Compilation check
 *
 * Uncomment following section, compile and run:
 *     g++ -g -std=c++11 `pkg-config --cflags --libs systemc` cache_L1.cpp -o cache_L1
 *     ./cache_L1
 *************************************************/


int sc_main(int argc, char *argv[])
{
    //CacheL1 c;

    sc_core::sc_start();
    
    return 0;
}
