#include "cache.hpp"
#include <iostream>
#include <list>
#include "memory_hierarchy_configuration.hpp"
#include "msi_fsm.hpp"


Set::Set():
    killIndex(0)
{
    for(int i=0; i<N_WAYS; i++)
    {
	blocks[i].setId(i);
	blocks[i].setHitFlag(&hitFlag);
	blocks[i].setUpgradeFlag(&upgradeFlag);
	blocks[i].setWriteBackFlag(&writeBackFlag);
	blocks[i].setInvalidList(&invalidBlocks);
	blocks[i].initiate();	
    }

}

void Set::process(const sc::event_base& e)
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
	if( op == READ)
	    blocks[ killIndex++ % N_WAYS ].process_event( Ev_insertShared(tag) );
	else
	    blocks[ killIndex++ % N_WAYS ].process_event( Ev_insertModified(tag) );
    }

}


void Cache::compute( Event& e, Event& e2)
{
    uint setIndex;
    uint blockTag;
    
    getSetIndexAndTag(e.address, &setIndex, &blockTag);

    switch (e.operation)
    {
    case READ:
    {
	// Hit
	sets[setIndex].process(Ev_CPU_read(blockTag));
	if ( sets[setIndex].hitFlag )
	{
	    e.source      = CACHE;
	    e.destination = CPU;
	    e.operation   = DATA_REPLY;
	}
	// Miss
	else
	{
	    e.source      = CACHE;
	    e.destination = DIRECTORY;
	    e.operation   = READ;
	    pending       = READ;
	}
	break;
    }
    case WRITE:
    {
	// Hit
	sets[setIndex].process(Ev_CPU_write(blockTag));
	if (sets[setIndex].hitFlag)
	{
	    // Upgrade (shared -> modified)
	    if( sets[setIndex].upgradeFlag )
	    {
		e.source      = CACHE;
		e.destination = DIRECTORY;
		e.operation   = INVALIDATE;
	    }
	}
	// Miss
	else
	{
	    e.source      = CACHE;
	    e.destination = DIRECTORY;
	    e.operation   = WRITE;
	    pending       = WRITE;
	}
	break;
    }
    case INVALIDATE:
    {
	sets[setIndex].process(Ev_invalidate(blockTag));
	if ( sets[setIndex].writeBackFlag )
	{
	    e.source = CACHE;
	    e.destination = DIRECTORY;
	    e.operation = WRITE_BACK;
	}
	break;
    }
    case DATA_REPLY:
    {
	sets[setIndex].insert(pending, blockTag);
	if ( pending == READ)
	{
	    e.source = CACHE;
	    e.destination = CPU;
	    e.operation = DATA_REPLY;
	}
	if ( sets[setIndex].writeBackFlag )
	{
	    e2.source = CACHE;
	    e2.destination = DIRECTORY;
	    e2.operation = WRITE_BACK;
	}
	break;
    }
    default:
    {
	break;
    }
    }// End of switch

    
}

/*
  int main(int argc, char *argv[])
  {
  Cache c;
  Event e(READ,3);
  c.compute( e, e );
    
  Event e2(DATA_REPLY,3);
  c.compute(e2, e2);

  Event e3(READ,3);
  c.compute( e3, e3 );

  Event e4(WRITE,3);
  c.compute( e4, e4 );

  Event e5(READ,3);
  c.compute( e5, e5 );

  Event e6(INVALIDATE,3);
  Event e7(INVALIDATE,3);
  c.compute( e6, e7 );
  if( e6.operation == WRITE_BACK)
  std::cout << "write back" << std::endl;

    

  return 0;
  }
*/


