#ifndef MSI_STATE_MACHINE_H
#define MSI_STATE_MACHINE_H


#include "../boost/statechart/event.hpp"
#include "../boost/statechart/state_machine.hpp"
#include "../boost/statechart/simple_state.hpp"
#include "../boost/statechart/state.hpp"
#include "../boost/statechart/transition.hpp"
#include "../boost/statechart/custom_reaction.hpp"
#include "../boost/mpl/list.hpp"


namespace sc  = boost::statechart; 
namespace mpl = boost::mpl;

// Forward declaration of states
struct Invalid;
struct Modified;
struct Shared;

//*********************************************************
// Events
//*********************************************************
struct Ev_CPU_read: sc::event<Ev_CPU_read>
{
    unsigned int tag;
    Ev_CPU_read(uint tag): tag(tag){};
};

struct Ev_CPU_write: sc::event<Ev_CPU_write>
{
    uint tag;
    Ev_CPU_write(uint tag): tag(tag){};
};

struct Ev_invalidate: sc::event<Ev_invalidate> {
    unsigned int tag;
    Ev_invalidate(uint tag): tag(tag){};
};

struct Ev_insertShared: sc::event<Ev_insertShared> {
    uint tag;
    Ev_insertShared(uint tag): tag(tag){};
};

struct Ev_insertModified: sc::event<Ev_insertModified> {
    uint tag;
    Ev_insertModified(uint tag): tag(tag){};
};


//*********************************************************
// Event related helpers
//*********************************************************
/*
bool isCPUread(const sc::event_base& e)
{
    try {
        const Ev_CPU_read& a = dynamic_cast<const Ev_CPU_read&>(e); 
	return true;
    }
    catch (const std::bad_cast& f) {
	return false;
    }
}

bool isCPUwrite(const sc::event_base& e)
{
    try {
        const Ev_CPU_write& a = dynamic_cast<const Ev_CPU_write&>(e); 
	return true;
    }
    catch (const std::bad_cast& f) {
	return false;
    }
}

bool isInvalidate(const sc::event_base& e)
{
    try {
        const Ev_invalidate& a = dynamic_cast<const Ev_invalidate&>(e); 
	return true;
    }
    catch (const std::bad_cast& f) {
	return false;
    }
}
*/

//********************************************************
// State Machine 
//********************************************************
struct Msi_machine : sc::state_machine<Msi_machine, Invalid>
{

    uint id;
    uint tag;
    bool *hitFlag;
    bool *upgradeFlag;
    bool *writeBackFlag;
    std::list<uint> *invalidList;

    void setId(uint);
    void setTag(uint);
    void setInvalidList(std::list<uint>*);
    void setHitFlag(bool*);
    void setUpgradeFlag(bool*);
    void setWriteBackFlag(bool*);
    
    void augmentInvalidList();
    void contractInvalidList();
    void signalHit();
    void signalUpgrade();
    void signalWriteBack();
};

//********************************************************
// Shared State
//********************************************************
struct Shared: sc::simple_state< Shared,  Msi_machine > {
    typedef mpl::list<
	sc::custom_reaction<Ev_invalidate>,
	sc::custom_reaction<Ev_CPU_read>,
	sc::custom_reaction<Ev_CPU_write>,
	sc::custom_reaction<Ev_insertShared>,
	sc::custom_reaction<Ev_insertModified> > reactions;

    Shared();
    sc::result react( const Ev_CPU_read&);
    sc::result react( const Ev_CPU_write&);
    sc::result react( const Ev_invalidate&);
    sc::result react( const Ev_insertShared&);
    sc::result react( const Ev_insertModified&);
};


//********************************************************
// Modified State
//********************************************************
struct Modified: sc::simple_state< Modified, Msi_machine > {
    typedef mpl::list<
	sc::custom_reaction<Ev_invalidate>,
	sc::custom_reaction<Ev_CPU_read>,
	sc::custom_reaction<Ev_CPU_write> > reactions;


    Modified();
    sc::result react( const Ev_CPU_read&);
    sc::result react( const Ev_CPU_write&);
    sc::result react( const Ev_invalidate&);
    sc::result react( const Ev_insertModified&);
    sc::result react( const Ev_insertShared&);
};



//********************************************************
// Invalid State
//********************************************************
struct Invalid: sc::state< Invalid, Msi_machine >
{
    typedef mpl::list<
	sc::custom_reaction<Ev_insertShared>,
	sc::custom_reaction<Ev_insertModified> > reactions;


    Invalid(my_context);
    void exit();
    sc::result react( const Ev_insertShared&);
    sc::result react( const Ev_insertModified&);
};




#endif

