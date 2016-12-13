#include "utils.hpp"
#include "cache.hpp"
#include <iostream>
#include <list>
#include "memory_hierarchy_configuration.hpp"
#include "msi_fsm.hpp"

using namespace msi_fsm;
namespace sc = boost::statechart;

Set::Set():
    killIndex(0)
{
    for(int i=0; i<N_WAYS; i++)
    {
	blocks[i].setId(i);
	blocks[i].setHitFlag(&hitFlag);
	blocks[i].setUpgradeFlag(&upgradeFlag);
	blocks[i].setWriteBackFlag(&writeBackFlag);
	blocks[i].setEvictionFlag(&evictionFlag);
	blocks[i].setInvalidList(&invalidBlocks);
	blocks[i].initiate();	
    }

}

void Set::dispatch(const sc::event_base& e)
{
    hitFlag = false;
    upgradeFlag = false;
    writeBackFlag = false;
    
    for( int i=0; i<N_WAYS; i++)
	blocks[i].process_event(e);

}

void Set::insert(Operation op, uint tag)
{
    if( !invalidBlocks.empty() )
    {
	if( op == READ)
	    blocks[invalidBlocks.front()].process_event( Ev_insertShared(tag) );
	else
	    blocks[invalidBlocks.front()].process_event( Ev_insertModified(tag) );
    }
    else
    {
	killIndex = (killIndex+1) % N_WAYS;
	if( op == READ)
	    blocks[ killIndex ].process_event( Ev_insertShared(tag) );
	else
	    blocks[ killIndex ].process_event( Ev_insertModified(tag) );
    }

}

SC_HAS_PROCESS(Cache);
Cache::Cache(sc_core::sc_module_name _name, uint _id):
    sc_core::sc_module(_name),
    id(_id)
{
    sets = new Set[numSets(CACHE_SIZE, BLOCK_SIZE, N_WAYS)];
    cpuSocket.register_b_transport(this, &Cache::serviceCpu);
    dirTargSocket.register_b_transport(this, &Cache::serviceDir);
    Debug("Cache " << id << " Size: " << CACHE_SIZE);
}

void Cache::serviceCpu(tlm::tlm_generic_payload &payload, sc_core::sc_time &delay)
{
    // Standard cache access latency
    // delay += sc_core::sc_time(5, sc_core::SC_NS);
    wait(5, sc_core::SC_NS);

    Event *e = payload.get_extension<Event>();
    uint setIndex;
    uint blockTag;
    getSetIndexAndTag(e->address, &setIndex, &blockTag);

    
    switch (e->operation)
    {
    case READ:
    {
	// Hit
	sets[setIndex].dispatch(Ev_CPU_read(blockTag));
	if ( sets[setIndex].hitFlag )
	{
	    ; // Just Return control to CPU
	}
	// Miss
	else
	{
	    pending = READ;
	    Debug("Cache " << id << " RMiss on " << e->address);
	    dirInitSocket->b_transport(payload, delay);
	}
	break;
    }
    case WRITE:
    {
	// Hit
	sets[setIndex].dispatch(Ev_CPU_write(blockTag));
	if (sets[setIndex].hitFlag)
	{
	    // Upgrade(shared -> modified) need to inform Directory
	    if( sets[setIndex].upgradeFlag )
	    {
		e->operation   = INVALIDATE;
		pending = INVALIDATE;
		Debug("Cache " << id << "Upgrade on " << e->address);
		dirInitSocket->b_transport(payload, delay);
	    }
	}
	// Miss
	else
	{
	    pending = WRITE;
	    Debug("Cache " << id << " WMiss on " << e->address);
	    dirInitSocket->b_transport(payload, delay);
	}
	break;
    }
    default:
	Debug("Cache " << id << " ERROR: Invalid operation from CPU");
    }// END_SWITCH

    payload.set_response_status(tlm::TLM_OK_RESPONSE);
    return;
}

void Cache::serviceDir(tlm::tlm_generic_payload &payload, sc_core::sc_time &delay)
{
    // Standard cache-directory access latency
    wait(10, sc_core::SC_NS);

    Event *e = payload.get_extension<Event>();
    uint setIndex;
    uint blockTag;
    getSetIndexAndTag(e->address, &setIndex, &blockTag);

    switch(e->operation)
    {
    case INVALIDATE:
    {
	sets[setIndex].dispatch(Ev_invalidate(blockTag));
	if( sets[setIndex].writeBackFlag )
	{
	    e->operation = WRITE_BACK;
	    dirInitSocket->b_transport(payload, delay);
	    sets[setIndex].writeBackFlag = false;
	}
    }
    case DOWNGRADE:
    {
	sets[setIndex].dispatch(Ev_downgrade(blockTag));
	e->operation = WRITE_BACK;
	dirInitSocket->b_transport(payload, delay);
    }
    case DATA_REPLY:
    {
	sets[setIndex].insert(pending, blockTag);
	if(sets[setIndex].writeBackFlag)
	{
	    e->operation = WRITE_BACK;
 	    dirInitSocket->b_transport(payload, delay);
	    sets[setIndex].writeBackFlag = false;
	}
	if(sets[setIndex].evictionFlag)
	{
	    e->operation = EVICTION;
	    dirInitSocket->b_transport(payload, delay);
	    sets[setIndex].evictionFlag = false;
	}
    }
    default:
	Debug("Cache " << id <<" ERROR: Invalid operation from Directory");
    }

    payload.set_response_status(tlm::TLM_OK_RESPONSE);
    return;
}




