#include "cache_transaction.hpp"





TransactionObjectsPool::TransactionObjectsPool()
    :numAvailable(0),
     numObjects(0)
{}

CacheTransaction& TransactionObjectsPool::acquire()
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

void TransactionObjectsPool::release( CacheTransaction &return_ticket)
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
 *A rudimentary unit test for the TransactionObjectsPool:
 *  1. We require NUM_OBJECTS CacheTransaction objects
 *  2. We access them
 *  3. We release them
 *  4. We ask the pool how many objects are there in the pool, should be NUM_OBJECTS
 *  5. We repeat 1-4, 20 times.
 * 
 * Uncomment following section, and compile:
 *     g++ -std=c++11 cache_transaction.cpp -o test_TransactionObjectsPool
 *     ./test_TransactionObjectsPool    
 ***********************************************************/

/*
#define NUM_OBJECTS 1000

#include <iostream>
#include <array>
#include <chrono>

using namespace std;
using namespace std::chrono;

int main(int argc, char *argv[])
{
    TransactionObjectsPool pool;
    array< CacheTransaction, NUM_OBJECTS> objectHandles;



    for(int k=0; k<20; k++)
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

	cout << "--------------------------------------" << endl;
	cout << "Object Pool Properties after allocating and deallocating " << NUM_OBJECTS << " objects" << endl;
	cout << "Size of a CacheTransaction object: " << sizeof(CacheTransaction) << ", ";
	cout << "thus, total size: " << sizeof(CacheTransaction) * NUM_OBJECTS / 1024 << "KB" << endl;
	auto d1 = t1 - t0;
	auto d2 = t2 - t1;
	cout << "Allocating   took: " << duration_cast<microseconds>(d1).count() << "us" << endl;
	cout << "Deallocating took: " << duration_cast<microseconds>(d2).count() << "us" << endl;
	cout << "Object Pool size (should be " << NUM_OBJECTS << "): " << pool.object_pool.size() << endl;
    }
    return 0;
}
*/
