#include "systemc.h"


SC_MODULE(Counter)
{
	sc_in<bool> clock;
	sc_in<bool> reset;
	sc_in<bool> enable;
	sc_out<sc_uint<2> > counter_out;

	// Local Variables
	sc_uint<2> count;

	// Counter Logic
	void incr_count()
	{
	    cout << "Hello World from Counter Process!" << endl;
		while (true)
		{
			wait();
			if (reset.read() == 1)
			{
			    count = 0;
			}
			else if (enable.read() == 1)
			{
			    count++;
			    counter_out.write(count);

			}
		}
	} // End of incr_count


	// Constructor
	SC_CTOR(Counter)
	{
	    count=0;
	    SC_CTHREAD(incr_count, clock.pos());

	}
};

SC_MODULE(Testbench)
{
	sc_out<bool> reset;
	sc_out<bool> enable;
	sc_in<sc_uint<2> > input;

	void print_count()
	{
		while (true)
		{
			wait();
			cout << "@ " << sc_time_stamp() << " ::Counter Value: " << input.read() << endl;
		}
	}

	SC_CTOR(Testbench)
	{
		SC_THREAD(print_count);
		sensitive << input;
	}

};

int sc_main(int argc, char **argv)
{
	sc_signal<bool> signal_reset("reset",false), signal_enable("enable", true);
	sc_signal<sc_uint<2> > signal_counter;
	
	Counter		counter("counter");
	sc_clock	clk("main_clock", sc_time(1, SC_SEC));
	counter.enable(signal_enable);
	counter.reset(signal_reset);
	counter.counter_out(signal_counter);
	counter.clock(clk);

	Testbench   testbench("testbench");
	testbench.enable(signal_enable);
	testbench.reset(signal_reset);
	testbench.input(signal_counter);

	
	// Start Simulation
	cout << "============================" << endl;
	cout << "SIMULATION BEGINS" << endl;
	cout << "============================" << endl;

	sc_start(8, SC_SEC);
	
	cout << "============================" << endl;
	cout << "SIMULATION ENDS" << endl;
	cout << "============================" << endl;
	// END OF SIMULATION
	
	return 0;
}
