#include "esu_fsm.hpp"
#include "utils.hpp"
#include <iostream>


using namespace esu_fsm;
//********************************************************
// State Machine 
//********************************************************
Esu_machine::Esu_machine():
    missFlag(false),
    upgradeFlag(false),
    downgradeFlag(false),
    dataRequestFlag(false)
{
    
}

void Esu_machine::setTag(uint t)
{
    tag = t;
}

void Esu_machine::addSharer(uint cpuID)
{
    sharers.push_front(cpuID);
}

void Esu_machine::removeSharer(uint cpuID)
{
    sharers.remove(cpuID);
}

void Esu_machine::signalUpgrade()
{
    Debug("Dir: Upgrade in " << tag);
    upgradeFlag = true;
    prevSharers = sharers;
}

void Esu_machine::signalDowngrade()
{
    Debug("Dir: Downgrage in " << tag);
    downgradeFlag = true;
    prevSharers   = sharers;
}

void Esu_machine::signalWriteBack()
{
    writeBackFlag = true;
    prevSharers = sharers;
}

//********************************************************
// Shared State
//********************************************************
Shared::Shared()
{
}

sc::result Shared::react(const Ev_Cache_read &e)
{
    outermost_context().addSharer(e.cpuID);
    return discard_event();
}

sc::result Shared::react(const Ev_Cache_write &e)
{
    outermost_context().signalUpgrade();
    outermost_context().clearSharers();
    outermost_context().addSharer(e.cpuID);
    return transit<Exclusive>();
}

sc::result Shared::react(const Ev_Cache_eviction &e)
{
    outermost_context().removeSharer(e.cpuID);
    if(outermost_context().isSharersEmpty())
	return transit<Uncached>();
    else
	return discard_event();
}

// The same reaction as with Ev_Cache_write
sc::result Shared::react(const Ev_Cache_inv &e)
{
    outermost_context().signalUpgrade();
    outermost_context().clearSharers();
    outermost_context().addSharer(e.cpuID);
    return transit<Exclusive>();
}

//********************************************************
// Modified State
//********************************************************
sc::result Exclusive::react(const Ev_Cache_read &e)
{
    outermost_context().signalDowngrade();
    outermost_context().addSharer(e.cpuID);
    return transit<Shared>();
}

sc::result Exclusive::react( const Ev_Cache_write &e)
{
    pendingReply = true;
    outermost_context().signalUpgrade();
    outermost_context().clearSharers();
    outermost_context().addSharer(e.cpuID);
    return transit<Exclusive>();
}

sc::result Exclusive::react( const Ev_Cache_writeBack &e)
{
    if( pendingReply )
    {
	pendingReply = false;
	return discard_event();	
    }
    else
    {
	outermost_context().signalWriteBack();
	outermost_context().clearSharers();
	return transit<Uncached>();
    }

}




//********************************************************
// Invalid State
//********************************************************
Uncached::Uncached()
{
}

sc::result Uncached::react(const Ev_Cache_read &e)
{
    outermost_context().missFlag = true;
    outermost_context().addSharer(e.cpuID);
    return transit<Shared>();
}

sc::result Uncached::react(const Ev_Cache_write &e)
{
    outermost_context().missFlag = true;
    outermost_context().addSharer(e.cpuID);
    return transit<Exclusive>();

}
