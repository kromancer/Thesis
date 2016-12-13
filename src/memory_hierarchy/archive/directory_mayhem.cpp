#include "directory.hpp"

DirectoryEntry::DirectoryEntry()
    : state {INVALID},
      owner {0}
{
    for( int i=0; i < N_CPUS; i++)
	sharers[i] = false;
}

Directory::Directory( sc_core::sc_module_name name)
    :sc_module(name),
     dir(numMemBlocks(MEM_SIZE, BLOCK_SIZE)),
     memory( 2 * CACHE_SIZE ),
     manager{ MemoryManager::GetSingleton() }
{
    for (int i=0; i < N_CPUS; i++)
    {
	char txt[20];
	sprintf(txt, "i_socket_%d", i);
	i_socket[i] = new tlm_utils::simple_initiator_socket_tagged<Directory,32,cache_coherence_protocol>(txt);

	sprintf(txt, "t_socket_%d", i);
	t_socket[i] = new tlm_utils::simple_target_socket_tagged<Directory,32,cache_coherence_protocol>(txt);
	t_socket[i]->register_b_transport( this, &Directory::respondToL1Caches, i );
    }
    dummy = 3;
}

// This is the b_transport method bound to ALL the t_sockets ( simple_target_socket_tagged )
void Directory::respondToL1Caches(int id, CacheTransaction &trans, sc_core::sc_time& delay)
{
    CoherenceMessageType msg   = trans.msg;
    Address              block = trans.address >> BLOCK_SIZE;

#ifndef NDEBUG
    CacheTransaction::printTransaction(trans, sc_core::sc_time_stamp());
#endif
    
    switch (msg)
    {
    //***********************************************************************
    case INV: // Case 1: A cache is requesting for every other sharing cache to invalidate their copy (e.g. WRITE_HIT on a shared block)
    //***********************************************************************
    {
	// That cache should already belong to the sharers
	assert( dir[block].sharers[id]==true );

	for( int i=0; i<N_CPUS; i++ )
	{
	    // if a cache is sharing the block (and of course is not the one sending the invalidate!)
	    if ( id != i && dir[block].sharers[i] )
	    {
		( *i_socket[i] )->b_transport(trans, delay);
		dir[block].sharers[i] = false;
	    }
	}
	// Only one cache has a dirty copy of the block
	dir[block].state = MODIFIED;
	dir[block].owner = id;
	break;
    }// End Case 1
    //***********************************************************************
    case READ_MISS: // Case 2: A cache has issued a READ_MISS
    //***********************************************************************
    {
	// Check if there is eviction
	if ( trans.evicting )
	{
	    Address temp = trans.evictedBlockAddress;
	    Address evictedBlock = temp >> BLOCK_SIZE;

	    // There should not be any discrepancy between directory and cache
	    assert( dir[evictedBlock].state == trans.evictedBlockState);

	    dir[evictedBlock].sharers[id] = false;
	    if ( trans.evictedBlockState == SHARED )
	    {
		// Check if this was the last sharer
		bool flag = true;
		for (int i=0; i < N_CPUS; i++)
		    flag |= dir[evictedBlock].sharers[i];
		if ( !flag )
		    dir[evictedBlock].state = INVALID;
	    }
	    else
	    {
		dir[evictedBlock].state = INVALID;
		// Maintain inclusion property?
		// Write back to memory?
	    }

	}
	switch ( dir[block].state)
	{
	// 2.1: The block is in INVALID state (uncached)
	//-------------------------------------------------------------------
	case INVALID:
	{
	    // Request block from Memory and store it in L2 cache
	    // In case of eviction in this level you need to send a BACK-INV

	    // Reply to cache
	    trans.bytes = &dummy;
	    dir[block].state       = SHARED;
	    dir[block].sharers[id] = true;
	    break;
	}
	// 2.2: The block is in SHARED state
	//-------------------------------------------------------------------
	case SHARED:
	{
	    // Request block from L2 cache
	    trans.bytes = &dummy;
	    dir[block].sharers[id] = true;
	    break;
	}
	// 2.3: The block is in MODIFIED state
	//-------------------------------------------------------------------
	case MODIFIED:
	{
	    // Request data from cache that owns block
	    trans.msg = FETCH;
	    ( *i_socket[ dir[block].owner ] )->b_transport(trans, delay);

	    // Store data to L2 cache

	    // Reply to requesting cache
	    // The reply is implicit, since the byte filed of the payload should be already modified
	    // by the previous transaction.
	    dir[block].state       = SHARED;
	    dir[block].sharers[id] = true;
	    break;
	}
	}// End Cases 2.*
	break;
    }// End Case 2
    //*********************************************************
    case WRITE_MISS: // Case 3: A cache has issued a WRITE_MISS
    //*********************************************************
    {
	switch ( dir[block].state )
	{
	// 3.1: The block is in INVALID state (uncached)
	//----------------------------------------------------
	case INVALID:
	{
	    // Update directory entry
	    dir[block].state       = MODIFIED;
	    dir[block].sharers[id] = true;
	    break;
	}
	// 3.2: The block is in SHARED state
	//----------------------------------------------------
	case SHARED:
	{
	    // Invalidate all sharers
	    trans.msg = INV;
	    for( int i=0; i<N_CPUS; i++ )
	    {
		if ( dir[block].sharers[i] )
		{
		    ( *i_socket[i] )->b_transport(trans, delay);
		    dir[block].sharers[i] = false;
		}
	    }

	    // Update directory entry
	    dir[block].state       = MODIFIED;
	    dir[block].sharers[id] = true;
	    break;
	}
	// 3.3: The block is in MODIFIED state 
	//----------------------------------------------------
	case MODIFIED:
	{
	    // This is the FETCH_INV situation
	    trans.msg = FETCH_INV;
	    ( *i_socket[ dir[block].owner ] )->b_transport(trans, delay);

	    // Reply to requesting cache
	    // The reply is implicit, since the byte filed of the payload should be already modified
	    // by the previous transaction.
	    dir[block].sharers[id] = true;
	    dir[block].owner = id;
	    break;
	}
	}// End Cases 3.*
	break;
    }// End Case 3
    }
}





/*************************************************
 * UNIT TEST
 *------------------------------------------------
 * 1. We connect the directory to one dummy cache, cache 0 ( l.238-239 ). 
 *    We initialize the Directory ( l.245-248 ). 
 *    We issue 3 transactions from the cache( run_module() ).
 *    The address always remains the same 
 * 2. We issue an INV. (This should happen if the cache had to react to a write hit.) 
 * 3. The directory should not perform any transaction (to this cache).
 * 4. We issue an READ_MISS.
 * 5. The directory should issue a FETCH to this cache.
 * 6. We issue a WRITE_MISS.
 * 7. The directory should issue an INV to this cache.
 *
 * Uncomment following section, compile and run:
 *     g++ -g -std=c++11 `pkg-config --cflags --libs systemc` directory.cpp cache_transaction.cpp -o directory
 *     ./directory
 *************************************************/
/*
#include <iostream>

using namespace std;

// Pretty Print Enumeration, for debug purposes
string PrintCoherenceMessage[] {"INV", "FETCH_INV", "FETCH", "READ_MISS", "WRITE_MISS"};

SC_MODULE(Dummy_cache)
{
    MemoryManager &manager;
    tlm_utils::simple_initiator_socket<Dummy_cache,32,cache_coherence_protocol> i_socket;
    tlm_utils::simple_target_socket<Dummy_cache,32,cache_coherence_protocol>    t_socket;

    void run_module()
    {
	CacheTransaction trans = manager.acquire();
	sc_core::sc_time delay(0,sc_core::SC_NS);
	trans.address = MEM_SIZE / 2;

	// Issue an INV
	trans.msg = INV;
	i_socket->b_transport(trans, delay);
	
	// Issue a READ_MISS
	trans.msg = READ_MISS;
	i_socket->b_transport(trans, delay);

	// Issue a WRITE_MISS
	trans.msg = WRITE_MISS;
	i_socket->b_transport(trans, delay);
    }
    
    void receive_trans(CacheTransaction &trans, sc_core::sc_time& delay)
    {
	cout << "Received a: " << PrintCoherenceMessage[trans.msg] << endl;
    };

    SC_CTOR(Dummy_cache)
	: i_socket("i_socket"),
	  t_socket("t_socket"),
	  manager{ MemoryManager::GetSingleton() }
	{
	    t_socket.register_b_transport(this, &Dummy_cache::receive_trans);
	    SC_THREAD(run_module);
	}
};
    

int sc_main(int argc, char **argv)
{
    new MemoryManager();
    Dummy_cache cache("dummy_cache");
    Directory   dir;

    
    dir.i_socket[0]->bind(cache.t_socket);
    cache.i_socket.bind(*dir.t_socket[0]);

    // Do a meaningless binding of the non-used directory sockets, so that the systemc runtime will not complain
    for( int i=1; i < N_CPUS; i++)
	dir.i_socket[i]->bind(*dir.t_socket[i]);

    // Directory Initialization
    dir.dir[MEM_SIZE/2 >> BLOCK_SIZE].state      = SHARED; 
    dir.dir[MEM_SIZE/2 >> BLOCK_SIZE].sharers[0] = true;

    sc_core::sc_start();

    return 0;
}
*/
