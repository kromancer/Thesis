#include "directory.hpp"
#include "utils.hpp"

using namespace sc_core;
using namespace tlm;


SC_HAS_PROCESS(Directory);
Directory::Directory(sc_module_name _name):
    sc_module(_name)
{
    blocks = new Esu_machine[numMemBlocks(MEM_SIZE,BLOCK_SIZE)];
    inSocket.register_b_transport(this, &Directory::serviceCache);
}


void Directory::serviceCache(int cacheID, tlm::tlm_generic_payload &payload, sc_core::sc_time &delay)
{
    // Standard cache-directory access latency
    // delay += sc_core::sc_time(5, sc_core::SC_NS);
    wait(10, sc_core::SC_NS);

    Event *e = payload.get_extension<Event>();
    uint setIndex;
    uint blockTag;
    getSetIndexAndTag(e->address, &setIndex, &blockTag);

    switch (e->operation)
    {
    case READ:
    {
	blocks[blockTag].process_event(Ev_Cache_read(cacheID));
	break;
    }
    case WRITE:
    {
	blocks[blockTag].process_event(Ev_Cache_write(cacheID));
	break;
    }
    case INVALIDATE:
    {
	blocks[blockTag].process_event(Ev_Cache_inv(cacheID));
	break;
    }
    case EVICTION:
    {
	blocks[blockTag].process_event(Ev_Cache_eviction(cacheID));
	break;
    }
    case WRITE_BACK:
    {
	blocks[blockTag].process_event(Ev_Cache_writeBack(cacheID));
	if( blocks[blockTag].writeBackFlag )
	{
	    ;// Push data to memory
	    blocks[blockTag].writeBackFlag = false;
	}
	break;
    }
    default:
    {
	Debug("Dir ERROR: Invalid operation from Cache");
   	break;	
    }
    }//END_SWITCH


    if( blocks[blockTag].missFlag)
    {
	//***********************
	// Access to Main Memory
	// (This wait statement ideally should belong to the b_transport of the main memory module)
	//**********************
	wait(100, sc_core::SC_NS);
	blocks[blockTag].missFlag = false;
    }
    
    if( blocks[blockTag].upgradeFlag)
    {
	e->operation = INVALIDATE;
	std::list<uint> *sharers = &blocks[blockTag].prevSharers;
	for( auto it=sharers->begin(); it != sharers->end(); it++ )
	{
	    outSocket[*it]->b_transport(payload, delay);
	}
	blocks[blockTag].upgradeFlag = false;
    }

    if( blocks[blockTag].downgradeFlag)
    {
	e->operation = DOWNGRADE;
	std::list<uint> *sharers = &blocks[blockTag].prevSharers;
	outSocket[sharers->front()]->b_transport(payload, delay);
	blocks[blockTag].downgradeFlag = false;
    }

}
    
