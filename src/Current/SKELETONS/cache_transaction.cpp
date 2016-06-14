#include "cache_transaction.hpp"

MemoryManager* MemoryManager::manager_Instance = nullptr;

MemoryManager::MemoryManager()
    :numAvailable(0),
     numObjects(0)
{
    assert(manager_Instance == nullptr);
    manager_Instance = static_cast<MemoryManager*>(this);
}

MemoryManager::~MemoryManager()
{
    manager_Instance = nullptr;
}

MemoryManager& MemoryManager::GetSingleton()
{
    return *manager_Instance;
}

CacheTransaction& MemoryManager::acquire()
{
    if (numAvailable == 0)
    {
	object_pool.emplace_back();
	numObjects++;
    }
    else
	numAvailable--;

    
    CacheTransaction &temp = object_pool.back();
    object_pool.pop_back();

    return temp;
}

void MemoryManager::release( CacheTransaction &return_ticket)
{
    // Clear all fields of the transaction object
    return_ticket.clear();
    // Make it available for future use
    object_pool.push_back(return_ticket);
    numAvailable++;
}


/************************************************************
 * UNIT TEST
 *-----------------------------------------------------------
 *A rudimentary unit test for the MemoryManager:
 *  1. We require NUM_OBJECTS CacheTransaction objects
 *  2. We access them
 *  3. We release them
 *  4. We ask the pool how many objects are there in the pool, should be NUM_OBJECTS
 *  5. We repeat 1-4, 20 times.
 * 
 * Uncomment following section, compile and run:
 *     g++ -g -std=c++11 `pkg-config --cflags --libs systemc` cache_transaction.cpp -o test_MemoryManager
 *     ./test_MemoryManager    
 ***********************************************************/

/*
// Investigate why we get segfault for 350000 and above
#define NUM_OBJECTS 300000

#include <iostream>
#include <array>
#include <chrono>

using namespace std;
using namespace std::chrono;

int sc_main(int argc, char *argv[])
{
    new MemoryManager();
    MemoryManager& pool = MemoryManager::GetSingleton();

    array< CacheTransaction, NUM_OBJECTS> objectHandles;



    cout << "--------------------------------------" << endl;
    cout << "Object Pool Properties after allocating and deallocating " << NUM_OBJECTS << " objects" << endl;
    cout << "Size of a CacheTransaction object: " << sizeof(CacheTransaction) << ", ";
    cout << "thus, total size: " << sizeof(CacheTransaction) * NUM_OBJECTS / 1024 << "KB" << endl;

    for(int k=0; k<2; k++)
    {
	auto t0 = system_clock::now();
	for( int i=0; i<NUM_OBJECTS; i++)
	    objectHandles[i] = pool.acquire();
	auto t1 = system_clock::now();
    
	// Try to access and see if mem allocation went well
	for( int i=0; i<NUM_OBJECTS; i++)
	    objectHandles[i].address = i;

	for( int i=0; i<NUM_OBJECTS; i++)
	    pool.release(objectHandles[i]);
	auto t2 = system_clock::now();
	auto d1 = t1 - t0;
	auto d2 = t2 - t1;

	cout << "--------------------------------------" << endl;
	if (k==0)
	    cout << "Initial allocation took: " << duration_cast<microseconds>(d1).count() << "us" << endl;
	else
	    cout << "Reacquisition took:      " << duration_cast<microseconds>(d1).count() << "us" << endl;
	cout     << "Deallocating took:       " << duration_cast<microseconds>(d2).count() << "us" << endl;
	cout     << "Object Pool size (should be " << NUM_OBJECTS << "): " << pool.object_pool.size() << endl;
    }
    cout << "--------------------------------------" << endl;

    return 0;
}
*/
