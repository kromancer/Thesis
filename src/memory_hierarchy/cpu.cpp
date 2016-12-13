#include "cpu.hpp"
#include "utils.hpp"

#include <sstream>
#include <string>
#include <boost/lexical_cast.hpp>





void Cpu::run()
{
    Event *temp = new Event;
    tlm::tlm_generic_payload payload;

    while( !memTrace.empty() )
    {
	*temp = memTrace.front();
	memTrace.pop();

	payload.set_extension(temp);
	payload.set_command(tlm::TLM_IGNORE_COMMAND);
	payload.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

	sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
	socket->b_transport( payload, delay);

	// Now the transaction is completed, check status and proceed
	tlm::tlm_response_status status = payload.get_response_status();
	if(status == tlm::TLM_OK_RESPONSE)
	{
	    // This is how simulation time progresses
	    // The process yields to the systemc kernel
	    sc_core::wait(delay);
	    //std::cout << sc_core::sc_time_stamp() << " COMPLETED" << std::endl;
	}
	else
	    ;//std::cout << sc_core::sc_time_stamp() << " ERROR" << std::endl;
    }
    return; 
}


// The constructor is mainly conserned with parsing the data from the trace
SC_HAS_PROCESS(Cpu);
Cpu::Cpu(sc_module_name _name, int id):
    id(id),
    sc_core::sc_module(_name)
{
    traceFile.open("thread_"+ std::to_string(id+1) + ".out");
    std::string line;
    uint64_t time = 0;
    while( std::getline(traceFile, line) )
    {
	// Parse data from trace
	std::string address, op;
	std::istringstream lineS(line);
	lineS >> op >> address;

	// Incorporate them in an event
	Event temp;
	temp.source = CPU;
	temp.destination = CACHE;
	temp.timestamp = time;
	time += DENSITY;
	temp.address = boost::lexical_cast<uint64_t>(address) - OFFSET;
	if( op=="r")
	    temp.operation = READ;
	else
	    temp.operation = WRITE;
	memTrace.push(temp);
    }
    SC_THREAD(run);
    Debug("Processor "  << id << " number of Mem Ops: " << memTrace.size());
}



