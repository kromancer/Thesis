#ifndef ESU_STATE_MACHINE_H
#define ESU_STATE_MACHINE_H

#include "boost/statechart/event.hpp"
#include "boost/statechart/state_machine.hpp"
#include "boost/statechart/simple_state.hpp"
#include "boost/statechart/state.hpp"
#include "boost/statechart/transition.hpp"
#include "boost/statechart/custom_reaction.hpp"
#include "boost/mpl/list.hpp"

namespace sc  = boost::statechart; 
namespace mpl = boost::mpl;


namespace esu_fsm{

// Forward declaration of states
    struct Uncached;
    struct Exclusive;
    struct Shared;

//*********************************************************
// Events
//*********************************************************
    struct Ev_Cache_read: sc::event<Ev_Cache_read>
    {
	uint cpuID;
	Ev_Cache_read(uint id): cpuID(id){};
    };

    struct Ev_Cache_write: sc::event<Ev_Cache_write>
    {
	uint cpuID;
	Ev_Cache_write(uint id): cpuID(id){};
    };

    struct Ev_Cache_eviction: sc::event<Ev_Cache_eviction>
    {
	uint cpuID;
	Ev_Cache_eviction(uint id): cpuID(id){};
    };

    struct Ev_Cache_writeBack: sc::event<Ev_Cache_writeBack>
    {
	uint cpuID;
	Ev_Cache_writeBack(uint id): cpuID(id){};
    };

    struct Ev_Cache_inv: sc::event<Ev_Cache_inv> {
	uint cpuID;
	Ev_Cache_inv(uint id): cpuID(id){};
    };

//********************************************************
// State Machine 
//********************************************************
    struct Esu_machine : sc::state_machine<Esu_machine, Uncached>
    {
	Esu_machine();
	uint tag;
	bool missFlag;
	bool upgradeFlag;
	bool downgradeFlag;
	bool dataRequestFlag;
	bool writeBackFlag;
	std::list<uint> sharers;
	std::list<uint> prevSharers;
    
	void setTag(uint);

	void signalUpgrade();
	void signalWriteBack();
	void signalDowngrade();

	void addSharer(uint);
	void removeSharer(uint);
	void clearSharers(){ sharers.clear();};
	bool isSharersEmpty(){ return sharers.empty();};
    };

//********************************************************
// Shared State
//********************************************************
    struct Shared: sc::simple_state< Shared,  Esu_machine > {
	typedef mpl::list<
	    sc::custom_reaction<Ev_Cache_read>,
	    sc::custom_reaction<Ev_Cache_write>,
	    sc::custom_reaction<Ev_Cache_eviction>,
	    sc::custom_reaction<Ev_Cache_inv> > reactions;
    
	Shared();
	sc::result react( const Ev_Cache_read&);
	sc::result react( const Ev_Cache_write&);
	sc::result react( const Ev_Cache_inv&);
	sc::result react( const Ev_Cache_eviction&);
    };


//********************************************************
// Exclusive State
//********************************************************
    struct Exclusive: sc::simple_state< Exclusive, Esu_machine > {
	typedef mpl::list<
	    sc::custom_reaction<Ev_Cache_writeBack>,
	    sc::custom_reaction<Ev_Cache_read>,
	    sc::custom_reaction<Ev_Cache_write> > reactions;

	bool pendingReply;
	Exclusive():pendingReply(false){};
	sc::result react( const Ev_Cache_read&);
	sc::result react( const Ev_Cache_write&);
	sc::result react( const Ev_Cache_writeBack&);
    };



//********************************************************
// Uncached State
//********************************************************
    struct Uncached: sc::simple_state< Uncached, Esu_machine >
    {
	typedef mpl::list<
	    sc::custom_reaction<Ev_Cache_read>,
	    sc::custom_reaction<Ev_Cache_write> > reactions;

	Uncached();
	sc::result react( const Ev_Cache_read&);
	sc::result react( const Ev_Cache_write&);
    };
}



#endif

