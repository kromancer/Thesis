#ifndef MSI_STATE_MACHINE_H
#define MSI_STATE_MACHINE_H


#include "boost/statechart/event.hpp"
#include "boost/statechart/state_machine.hpp"
#include "boost/statechart/simple_state.hpp"
#include "boost/statechart/state.hpp"
#include "boost/statechart/transition.hpp"
#include "boost/statechart/custom_reaction.hpp"
#include "boost/mpl/list.hpp"


namespace sc  = boost::statechart; 
namespace mpl = boost::mpl;

namespace msi_fsm{

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

    struct Ev_downgrade: sc::event<Ev_downgrade> {
	uint tag;
	Ev_downgrade(uint tag): tag(tag){};
    };

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
	bool *evictionFlag;
	std::list<uint> *invalidList;

	void setId(uint);
	void setTag(uint);
	void setInvalidList(std::list<uint>*);
	void setHitFlag(bool*);
	void setUpgradeFlag(bool*);
	void setWriteBackFlag(bool*);
	void setEvictionFlag(bool*);
    
	void augmentInvalidList();
	void contractInvalidList();
	void signalHit();
	void signalUpgrade();
	void signalWriteBack();
	void signalEviction();
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
	sc::result react( const Ev_downgrade&);
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
}



#endif

