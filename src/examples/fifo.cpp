#include "systemc.h"
#include <iostream>
using namespace sc_core;

//****************
// FIFO Interfaces
//****************
// Fifo interface exposed to Producers
class fifo_write_if: virtual public sc_interface
{
public:
    // blocking write
    virtual void write(char) = 0;
    // number of free entried
    virtual int numFree() const = 0;
};

// Fifo interface exposed to Consumers
class fifo_read_if: virtual public sc_interface
{
public:
    // blocking read
    virtual char read() = 0;
    // number of available entries
    virtual int numAvailable() const = 0;
};

//*************
// FIFO channel
//*************
class Fifo: public sc_prim_channel, public fifo_write_if, public fifo_read_if
{
protected:
    int   size;
    char *buf;
    int   free;
    int   ri; // read index
    int   wi; // write index
    int   numReadable, numRead, numWritten;

    // For notifying Producer and Consumer
    sc_event Ev_dataRead;
    sc_event Ev_dataWritten;

public:
    // Constructor
    explicit Fifo(int _size=16):
	sc_prim_channel(sc_gen_unique_name("thefifo"))
	{
	    size = _size;
	    buf = new char[_size];
	    reset();
	}
    // Destructor
    ~Fifo(){ delete [] buf; }

    int  numAvailable() const { return numReadable - numRead; }
    int  numFree() const { return size - numReadable; }
    void reset() { free=size; ri=0; wi=0; }

    
    // Blocking write implementation
    void write(char c)
	{
	    if (numFree() == 0)
		wait( Ev_dataRead );
	    numWritten++;
	    buf[wi] = c;
	    wi = (wi+1) % size; // Circular buffer
	    free--;
	    request_update();
	}

    // Blocking read implementation
    char read()
	{
	    if (numAvailable() == 0)
		wait( Ev_dataWritten );
	    numRead++;
	    char temp  = buf[ri];
	    ri = (ri+1) % size; // Circular buffer
	    free++;
	    request_update();
	    return temp;
	}

    // Update method called in the UPDATE phase of the simulation
    void update()
	{
	    if (numRead > 0)
		Ev_dataRead.notify(SC_ZERO_TIME);
	    if (numWritten > 0)
		Ev_dataWritten.notify(SC_ZERO_TIME);
	    numReadable = size - free;
	    numRead = 0;
	    numWritten = 0;
	}
};





class Producer: public sc_module
{
public:
    sc_port<fifo_write_if> out;
    sc_in<bool> clock;

    void run()
	{
	    while(1)
	    {
		wait(); // wait for clock edge
		out->write(1);
		cout << "Produced at: " << sc_time_stamp() << endl;
	    }
	}

    // Constructor
    SC_CTOR(Producer)
	{
	    SC_THREAD(run);
	    sensitive << clock.pos();
	}
};


class Consumer: public sc_module
{
public:
    sc_port<fifo_read_if> in;
    sc_in<bool> clock;

    void run()
	{
	    while(1)
	    {
		wait(); // wait for clock edge
		char temp = in->read();
		cout << "Consumed at: " << sc_time_stamp() << endl;
	    }
	}

    SC_CTOR(Consumer)
	{
	    SC_THREAD(run);
	    sensitive << clock.pos();
	}

};


int sc_main(int argc, char *argv[])
{
    sc_set_time_resolution(1, SC_FS);
    sc_clock clkFast("ClkFast", 1.0, SC_NS);
    sc_clock clkSlow("ClkSlow", 1.0, SC_SEC);

    Fifo fifo1;

    Producer p1("p1");
    p1.out(fifo1);
    p1.clock(clkFast);

    Consumer c1("c1");
    c1.in(fifo1);
    c1.clock(clkSlow);

    sc_start(1, SC_SEC);
    
    return 0;
}
