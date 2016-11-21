#include "msi_fsm.hpp"
#include <iostream>



//********************************************************
// State Machine 
//**********************************n**********************
void Msi_machine::setId(uint i)
{
    id = i;
}

void Msi_machine::setTag(uint t)
{
    tag = t;
}

void Msi_machine::setHitFlag(bool *f)
{
    hitFlag = f;
}

void Msi_machine::setUpgradeFlag(bool *f)
{
    upgradeFlag = f;
}

void Msi_machine::setInvalidList(std::list<uint>* l)
{
    invalidList = l;
}

void Msi_machine::setWriteBackFlag(bool *f)
{
    writeBackFlag = f;
}
void Msi_machine::augmentInvalidList()
{
    invalidList->push_front(id);
}

void Msi_machine::contractInvalidList()
{
    invalidList->remove(id);
}

void Msi_machine::signalHit()
{
    std::cout << "hit" << std::endl;
    (*hitFlag) = true;
}

void Msi_machine::signalUpgrade()
{
    std::cout << "upgrade" << std::endl;
    (*hitFlag) = true;
    (*upgradeFlag) = true;
}

void Msi_machine::signalWriteBack()
{
    (*writeBackFlag) = true;
}

//********************************************************
// Shared State
//********************************************************
Shared::Shared()
{
    std::cout << "--Shared--" << std::endl;
}

sc::result Shared::react(const Ev_invalidate &e)
{
    if( e.tag == outermost_context().tag)
	return transit<Invalid>();
    else
	return discard_event();
}

sc::result Shared::react(const Ev_CPU_read& e)
{
    if( e.tag == outermost_context().tag )
	outermost_context().signalHit();

    return discard_event();
}


sc::result Shared::react(const Ev_CPU_write &e)
{
    if( e.tag == outermost_context().tag )
    {
	outermost_context().signalUpgrade();
	return transit<Modified>();
    }

    return discard_event();
}

sc::result Shared::react(const Ev_insertShared &e)
{
    outermost_context().setTag(e.tag);
    return discard_event();
}

sc::result Shared::react(const Ev_insertModified &e)
{
    outermost_context().setTag(e.tag);
    return transit<Modified>();
}

//********************************************************
// Modified State
//********************************************************
Modified::Modified()
{
    std::cout << "--Modified--" << std::endl;
}

sc::result Modified::react(const Ev_invalidate &e)
{
    if( e.tag == outermost_context().tag)
    {
	outermost_context().signalWriteBack();
	return transit<Invalid>();	
    }
    else
	return discard_event();
}

sc::result Modified::react( const Ev_CPU_read &e)
{
    if( e.tag == outermost_context().tag)
    {
	outermost_context().signalHit();
	return discard_event();
    }
    else
	return discard_event();
}

sc::result Modified::react( const Ev_CPU_write &e)
{
    if( e.tag == outermost_context().tag)
    {
	outermost_context().signalHit();
	return discard_event();
    }
    else
	return discard_event();
}

sc::result Modified::react(const Ev_insertModified &e)
{
    //outermost_context().signalWriteBack();
    return discard_event();
}

sc::result Modified::react(const Ev_insertShared &e)
{
    //outermost_context().signalWriteBack();
    return transit<Shared>();
}

//********************************************************
// Invalid State
//********************************************************
Invalid::Invalid( my_context ctx):
    my_base(ctx)
{
    context<Msi_machine>().augmentInvalidList();
}

void Invalid::exit()
{
    context<Msi_machine>().contractInvalidList();
}

sc::result Invalid::react(const Ev_insertShared &e)
{
    context<Msi_machine>().setTag(e.tag);
    return transit<Shared>();
}

sc::result Invalid::react(const Ev_insertModified &e)
{
    context<Msi_machine>().setTag(e.tag);
    return transit<Modified>();

}
