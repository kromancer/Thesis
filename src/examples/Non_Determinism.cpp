#include "systemc.h"

using namespace sc_core;


std::string sharedVariable;

SC_MODULE(chaos1)
{
    sc_in<bool> clock;

    void run()
    {
	sharedVariable = "chaos";
    }

    SC_CTOR(chaos1)
    {
	SC_THREAD(run);
	sensitive << clock.pos(); // static sensitivity
    }
};

SC_MODULE(chaos2)
{
    sc_in<bool> clock;

    void run()
    {
	sharedVariable = "and destruction";
    }

    SC_CTOR(chaos2)
    {
	SC_THREAD(run);
	sensitive << clock.pos(); // static sensitivity
    }
    
};

SC_MODULE(observer)
{
    sc_in<bool> clock;

    void run()
    {
	while(2)
	{
	    wait();
	    cout << sharedVariable << endl;
	}
    }

    SC_CTOR(observer)
    {
	SC_THREAD(run);
	sensitive << clock.pos(); // static sensitivity
    }
    
};





int sc_main(int argc, char *argv[])
{
    sc_clock clock("clock", 1, SC_NS);
    chaos1 c1("c1");
    chaos2 c2("c2");
    observer ob("ob");
	
    c1.clock(clock);
    c2.clock(clock);
    ob.clock(clock);
    
    sc_start(100, SC_NS);
    
    return 0;
}

