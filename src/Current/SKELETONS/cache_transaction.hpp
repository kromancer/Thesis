#include <deque>
#include <vector>
using namespace std;

enum BlockState             {INVALID, SHARED, MODIFIED};
enum CoherenceMessageType   {INV, FETCH_INV, FETCH, READ_MISS, WRITE_MISS};
enum Operation              {READ, WRITE};

using uint = unsigned int;
using Address  = unsigned int;
using SetIndex = unsigned int;
using BlockTag = unsigned int;
using uint     = unsigned int;


// Emulates a generic payload?
class CacheTransaction
{
    friend class TransactionObjectsPool;
private:
    CacheTransaction()
	:address(0),
	 op(READ),
	 bytes(nullptr),
	 msg(READ_MISS)
	{}

public:
    Address              address;
    Operation            op;
    unsigned char       *bytes;
    CoherenceMessageType msg;
};


/*******************************************
 * CLASS TransactionObjectsPool
 *------------------------------------------
 * A simple manager of transaction objects, 
 * that enables reuse, 
 * thus minimizing the perforamce overhead of 
 * creating new CacheTransaction objects.
 ******************************************/

class TransactionObjectsPool
{
private:
    uint numObjects;
    uint numAvailable;
    deque <CacheTransaction*> ticket_pool;
    vector<CacheTransaction>  object_pool;
public:
    TransactionObjectsPool();
    CacheTransaction* acquire();
    void              release( CacheTransaction* );
};

