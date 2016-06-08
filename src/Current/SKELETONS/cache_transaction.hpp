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

public:
    CacheTransaction()
	:address(0),
	 op(READ),
	 bytes(nullptr),
	 msg(READ_MISS)
	{}

    CacheTransaction& operator=(const CacheTransaction& a)
	{
	    address = a.address;
	    bytes = a.bytes;
	    op = a.op;
	    msg = a.msg;
	    return *this;
	}
    
    void clear()
    {
	address = 0;
	op      = READ;
	bytes   = nullptr;
	msg     = READ_MISS;
    };

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
 * that enables reuse of created transaction objects, 
 * thus minimizing the perforamce overhead of allocation.
 *
 * Private:
 * The object_pool: is a vector of objects.
 * The ticket_pool: is a queue of object pointers (let us call them handles),
 *                  pointing in an object somewhere inside the vector.
 * numObjects:      objects that have been created
 * numAvailable:    available objects. Should be between 0 <= numAvailable <= numObjects
 * 
 * acquire(): request for a pointer to a transaction object. 
 *            If numbAvailable == 0, create one.
 *            else  pop a handle from the front of the queue.
 * release: notify the manager that a transaction object is not being used.
 *          push back the object handle.
 * 
 ******************************************/

class TransactionObjectsPool
{
public:
    TransactionObjectsPool();
    CacheTransaction& acquire();
    void              release( CacheTransaction& );
    
public:
    uint numObjects;
    uint numAvailable;
    vector<CacheTransaction> object_pool;


};

