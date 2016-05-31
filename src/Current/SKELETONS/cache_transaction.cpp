#include "cache_transaction.hpp"

TransactionObjectsPool::TransactionObjectsPool()
    :numAvailable(0)
{}

CacheTransaction* TransactionObjectsPool::acquire()
{
    CacheTransaction *temp;
    if (numAvailable == 0)
    {
	object_pool.emplace_back( CacheTransaction() );
	temp = &object_pool.back();
	numObjects++;
    }
    else
    {
	temp = ticket_pool.front();
	numAvailable--;
    }

    return temp;
}

void TransactionObjectsPool::release( CacheTransaction* return_ticket)
{
    ticket_pool.push_back( return_ticket );
    numAvailable++;
}

/*
int main(int argc, char *argv[])
{
    TransactionObjectsPool pool;

    CacheTransaction *t1 = pool.acquire();
    CacheTransaction *t2 = pool.acquire();
    CacheTransaction *t3 = pool.acquire();
    pool.release( t1 );
    pool.release( t2 );
    pool.release( t3 );

    return 0;
}
*/
