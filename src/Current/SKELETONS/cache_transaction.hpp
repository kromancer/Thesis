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
 * The object_pool: is a vector of objects.
 * numObjects:      objects that have been created
 * numAvailable:    available objects. Should be between 0 <= numAvailable <= numObjects
 * 
 * acquire(): request for a reference to a transaction object. 
 *            If numbAvailable == 0, create a new transaction object.
 *            else  pop one from the vector.
 * release: notify the manager that a transaction object is not being used.
 *          push back the object to the vector.
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

