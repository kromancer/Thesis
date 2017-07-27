#include "systemc"
#include "tlm.h"
#define BASE_ADDRESS 400
#define NUM_MEM_OPS  128 // How many transactions will the Initiator generate
#define MEM_SIZE     256 // The Target acts like a memory

using namespace std;
using namespace sc_core;
using namespace tlm;


struct Initiator: sc_module, tlm_bw_transport_if<>
{
    tlm_initiator_socket<> socket; 
    int data;

    SC_CTOR(Initiator) : socket("socket")
	{
	    socket.bind(*this);
	    SC_THREAD(run);
	}

void run()
{
    tlm::tlm_generic_payload trans;
    sc_time delay=SC_ZERO_TIME;

    for(int i=0; i<NUM_MEM_OPS; i+=4)
    {
	// Generate 4-byte aligned addresses
	int address = BASE_ADDRESS + i;
	// Generate a random command (either read or write)
	tlm_command cmd = static_cast<tlm_command>(rand()%2);
	// If write then put some junk data
	if (cmd == TLM_WRITE_COMMAND) data = address;
	// Set payload's fields
	trans.set_command(cmd);
	trans.set_address( address );
	trans.set_data_ptr( reinterpret_cast<unsigned char*>(&data) );
	trans.set_data_length( 4 );
	trans.set_streaming_width( 4 ); // not used
	trans.set_byte_enable_ptr( 0 ); // not used
	trans.set_dmi_allowed( false ); // not used
	trans.set_response_status( tlm::TLM_INCOMPLETE_RESPONSE );

// Initiate Transaction
	socket->b_transport( trans, delay );
	// Transaction is completed

	// Check if everything went well
	if (trans.is_response_error())
	    SC_REPORT_ERROR("TLM-2", trans.get_response_string().c_str());

	// Display payload
	cout << name() << " completed " << (cmd ? "write" : "read") 
             << ", addr = " << hex << i
	     << ", data = " << hex << data << ", time " << sc_time_stamp()
	     << " delay = " << delay << endl;

	// Syncrhonize with simulation time
	wait(delay);
	delay = SC_ZERO_TIME;
    }
}

// TLM-2 backward non-blocking transport method
    virtual tlm::tlm_sync_enum nb_transport_bw( tlm::tlm_generic_payload& trans,
						tlm::tlm_phase& phase, 
                                                sc_time& delay )
	{
	    // Dummy method
	    return tlm::TLM_ACCEPTED;
	}

    // TLM-2 backward DMI method
    virtual void invalidate_direct_mem_ptr(sc_dt::uint64 start_range,
					   sc_dt::uint64 end_range)
	{
	    // Dummy method
	}
}; //END_INITIATOR_DEFINITION

struct Memory: sc_module, tlm::tlm_fw_transport_if<>
{
    int *mem;
    tlm::tlm_target_socket<> socket;


    SC_CTOR(Memory)
	: socket("socket")
	{
	    mem = new int[MEM_SIZE];
	    socket.bind(*this);

	    // Initialize memory with random data
	    for (int i = 0; i < MEM_SIZE; i++)
		mem[i] = rand() % 256;
	}

// TLM-2 blocking transport method
virtual void b_transport( tlm_generic_payload& trans, sc_time& delay )
    {
	tlm::tlm_command cmd = trans.get_command();
	sc_dt::uint64    adr = trans.get_address();
	unsigned char*   ptr = trans.get_data_ptr();
	unsigned int     len = trans.get_data_length();
	unsigned char*   byt = trans.get_byte_enable_ptr();
	unsigned int     wid = trans.get_streaming_width();

	// Obliged to check address range and check for unsupported features,
	//   i.e. byte enables, streaming, and bursts
	// Can ignore DMI hint and extensions
	// SystemC's report handler is an acceptable way of signalling an error
	if (adr >= sc_dt::uint64(MEM_SIZE) * 4 
            || adr % 4 || byt != 0 
            || len > 4 || wid < len)
	    SC_REPORT_ERROR("TLM-2", "Target does not support this transaction");

	// Obliged to implement read and write commands
	if ( cmd == tlm::TLM_READ_COMMAND )
	    memcpy(ptr, &mem[adr/4], len);
	else if ( cmd == tlm::TLM_WRITE_COMMAND )
	    memcpy(&mem[adr/4], ptr, len);

	// Memory access time
	delay = delay + sc_time(100, SC_NS);

	// Obliged to set response status to indicate successful completion
	trans.set_response_status( tlm::TLM_OK_RESPONSE );
    }

// TLM-2 forward non-blocking transport method
    virtual tlm::tlm_sync_enum nb_transport_fw( tlm::tlm_generic_payload& trans,
						tlm::tlm_phase& phase, 
                                                sc_time& delay )
	{
	    // Dummy method (not TLM-2.0 compliant)
	    return tlm::TLM_ACCEPTED;
	}

    // TLM-2 forward DMI method
    virtual bool get_direct_mem_ptr(tlm::tlm_generic_payload& trans,
				    tlm::tlm_dmi& dmi_data)
	{
	    // Dummy method
	    return false;
	}

    // TLM-2 debug transport method
    virtual unsigned int transport_dbg(tlm::tlm_generic_payload& trans)
	{
	    // Dummy method
	    return 0;
	}
}; // END_TARGET_DEFINITION

int sc_main(int argc, char *argv[])
{
    Initiator proc("proc");
    Memory    mem("mem");

    proc.socket.bind( mem.socket );

    sc_start();
    return 0;
}
