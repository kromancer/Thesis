#ifndef CACHE_TRANSACTION_H
#define CACHE_TRANSACTION_H

#include "tlm"
#include <cassert>
#include <vector>
using namespace std;

enum BlockState             {INVALID, SHARED, MODIFIED};
enum CoherenceMessageType   {INV,   FETCH_INV,   FETCH,   READ_MISS,   WRITE_MISS};
enum Operation              {READ, WRITE};
enum Phases                 {REQ, RESP};

using uint = unsigned int;
using Address  = unsigned int;
using SetIndex = unsigned int;
using BlockTag = unsigned int;
using uint     = unsigned int;

/*******************************************
 * CLASS CacheTransaction
 *------------------------------------------
 * Meant to replace tlm_generic_paylooad
 * 
 ******************************************/
class CacheTransaction
{

public:
    CacheTransaction()
	:address(0),
	 op(READ),
	 bytes(nullptr),
	 msg(READ_MISS)
	{}

    //Copy Assingment Operator
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

// Dummy functions, in order to allow the usage of a CacheTransaction payload
// with the utility sockets (e.g. simple_target_socket, simple_initiator_socket).
// Warning, the dummy implementation of these functions make the sockets not able
// to automatically transform blocking <-> non-blocking interface calls. 
    void set_mm(void *mm) {};
    bool has_mm()         {return 1;}
    void acquire()        {};
    void release()        {};
    int  get_ref_count()  {return 1;};
    void set_auto_extension(void *mm){};
    
public:
    Address              address;
    Operation            op;
    unsigned char       *bytes;
    CoherenceMessageType msg;
    bool                 evicting;
    BlockState           evictedBlockState;
    Address              evictedBlockAddress;



/*****************************************
 * For Debugging Purposes: Printing the contents of transaction
 ****************************************/
#ifndef NDEBUG
static void printTransaction( tlm::tlm_generic_payload &trans, const sc_core::sc_time &time_stamp)
{
    // Pretty printing of enumerations when debugging
    static const string string_blockState[]   = {"INVALID", "SHARED", "MODIFIED"};
    static const string string_coherenceMsg[] = {"INV",   "FETCH_INV",   "FETCH",   "READ_MISS",   "WRITE_MISS"};
    static const string string_gpCommand[]    = {"READ", "WRITE"};

    std::cout << "@ " << time_stamp << std::endl;
    std::cout << "Type: " << string_gpCommand[trans.get_command()] << std::endl;
    std::cout << "For Address: " << trans.get_address() << std::endl << std::endl;
}

static void printTransaction( CacheTransaction &trans, const sc_core::sc_time &time_stamp)
{
    // Pretty printing of enumerations when debugging
    static const string string_blockState[]   = {"INVALID", "SHARED", "MODIFIED"};
    static const string string_coherenceMsg[] = {"INV",   "FETCH_INV",   "FETCH",   "READ_MISS",   "WRITE_MISS"};
    static const string string_gpCommand[]    = {"READ", "WRITE"};


    std::cout << "@ " << time_stamp << std::endl;
    std::cout << "Type: " << string_coherenceMsg[trans.msg] << std::endl;
    std::cout << "Address: " << trans.address << std::endl << std::endl;
    std::cout << "Eviction: " << ((trans.evicting) ? "True" : "False") << std::endl;
    
}
#endif
}; // End of CacheTransaction class declaration



struct cache_coherence_protocol
{
    typedef CacheTransaction         tlm_payload_type;
    typedef tlm::tlm_phase           tlm_phase_type;          
};

/*******************************************
 * CLASS MemoryManager
 *------------------------------------------
 * A simple manager of transaction objects, 
 * that enables reuse of created transaction objects.
 *
 * The object_pool: is a vector of transaction objects.
 * numObjects:      transaction objects that have been created
 * numAvailable:    available objects. Should be between 0 <= numAvailable <= numObjects
 * 
 * acquire(): request for a reference to a transaction object. 
 *            If numbAvailable == 0, create a new transaction object.
 *            else  pop one from the vector.
 * release(): notify the manager that a transaction object is not being used.
 *            push back the object to the vector.
 * 
 ******************************************/

class MemoryManager
{
private:
    static MemoryManager *manager_Instance;
public:
    MemoryManager();
    ~MemoryManager();
    static MemoryManager& GetSingleton();
    CacheTransaction& acquire();
    void              release( CacheTransaction& );
public:
    uint numObjects;
    uint numAvailable;
    vector<CacheTransaction> object_pool;
};

#endif


