#include "../boost/statechart/event.hpp"
#include "../boost/statechart/state_machine.hpp"
#include "../boost/statechart/simple_state.hpp"
#include "../boost/statechart/transition.hpp"
#include <iostream>

namespace sc = boost::statechart; 

// Forward declaration of initial state
struct Invalid;

// Event declaration
struct Ev_CPU_read:      sc::event<Ev_CPU_read>  {};
struct Ev_CPU_read_miss: sc::event<Ev_CPU_read_miss> {};
struct Ev_CPU_read_hit:  sc::event<Ev_CPU_read_hit> {};

struct Ev_CPU_write:      sc::event<Ev_CPU_write> {};
struct Ev_CPU_write_miss: sc::event<Ev_CPU_write_miss> {};
struct Ev_CPU_write_hit:  sc::event<Ev_CPU_write_hit> {};

struct Ev_invalidate :       sc::event<Ev_invalidate> {};
struct Ev_fetch_invalidate : sc::event<Ev_fetch_invalidate> {};





struct Machine : sc::state_machine< Machine, Invalid > {}; 

struct Shared    : sc::simple_state< Shared,  Machine > {};
struct Modified  : sc::simple_state< Modified, Machine > {};
struct Invalid   : sc::simple_state< Invalid, Machine >
{
    typedef sc::transition< CPU_read, Shared >    read_miss;
    typedef sc::transition< CPU_write, Modified > write_miss;
};




int main(int argc, char *argv[])
{

    Machine myMachine;

    myMachine.initiate();

    myMachine.process_event( CPU_read() );
    myMachine.process_event( CPU_write() );

    return 0;
}

