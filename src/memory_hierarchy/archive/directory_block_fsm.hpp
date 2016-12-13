#ifndef DIRECTORY_BLOCK_FSM_H
#define DIRECTORY_BLOCK_FSM_H


#include "../boost/statechart/event.hpp"
#include "../boost/statechart/state_machine.hpp"
#include "../boost/statechart/simple_state.hpp"
#include "../boost/statechart/state.hpp"
#include "../boost/statechart/transition.hpp"
#include "../boost/statechart/custom_reaction.hpp"
#include "../boost/mpl/list.hpp"
#include <list>


namespace sc  = boost::statechart; 
namespace mpl = boost::mpl;

// Forward declaration of states
struct Uncached;
struct Exclusive;
struct Shared;

//*********************************************************
// Events
//*********************************************************
struct Ev_readMiss: sc::event<Ev_readMiss>
{
    uint id;
    uint tag;
    Ev_readMiss(uint id, uint tag): id(id), tag(tag){};
};

struct Ev_writeMiss: sc::event<Ev_writeMiss>
{
    uint id;
    uint tag;
    Ev_writeMiss(uint id, uint tag): id(id), tag(tag){};
};

struct Ev_writeBack: sc::event<Ev_writeMiss>
{
    uint id;
    uint tag;
    Ev_writeBack(uint id, uint tag): id(id), tag(tag){};
};

//********************************************************
// State Machine 
//********************************************************
struct Fsm_dirBlock : sc::state_machine<Fsm_dirBlock, Uncached>
{
    std::list<uint> sharers;

    void augmentSharers(const Ev_readMiss&);
    void newOwner(const Ev_writeMiss&);
    void clearSharers(const Ev_writeBack&);

    void downgrade(const Ev_readMiss&);
    void upgrade(const Ev_readMiss&);
};


void Fsm_dirBlock::augmentSharers(const Ev_readMiss &e)
{
    sharers.push_back(e.id);
}

void Fsm_dirBlock::newOwner(const Ev_writeMiss &e)
{
    sharers.clear();
    sharers.push_back(e.id);
}

void Fsm_dirBlock::clearSharers(const Ev_writeBack &e)
{
    sharers.clear();
}

//********************************************************
// Shared State
//********************************************************
struct Shared: sc::simple_state< Shared,  Fsm_dirBlock > {
    typedef mpl::list<
	sc::custom_reaction<Ev_readMiss>,
	sc::transition<Ev_writeMiss, Exclusive, Fsm_dirBlock, &Fsm_dirBlock::newOwner> > reactions;

    sc::result react( const Ev_readMiss& );
};

sc::result Shared::react(const Ev_readMiss &e)
{
    outermost_context().augmentSharers(e);
    return discard_event();
}

//********************************************************
// Exclusive State
//********************************************************
struct Modified: sc::simple_state< Modified, Fsm_dirBlock > {
    typedef mpl::list<
	sc::custom_reaction<Ev_writeMiss>,
	sc::transition<Ev_writeBack, Uncached, Fsm_dirBlock, &Fsm_dirBlock::clearSharers>,
	sc::transition<Ev_readMiss, Shared, Fsm_dirBlock, &Fsm_dirBlock::augmentSharers> > reactions;

    sc::result react( const Ev_writeMiss& );
};

sc::result Modified::react( const Ev_writeMiss& e )
{
    outermost_context().newOwner(e);
    return discard_event();
}

//********************************************************
// Uncached State
//********************************************************
struct Uncached: sc::simple_state< Uncached, Fsm_dirBlock >
{
    typedef mpl::list<
	sc::transition<Ev_readMiss, Shared, Fsm_dirBlock, &Fsm_dirBlock::augmentSharers>,
        sc::transition<Ev_writeMiss, Exclusive, Fsm_dirBlock, &Fsm_dirBlock::newOwner > > reactions;
};




#endif

