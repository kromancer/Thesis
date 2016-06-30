#include "tlm.h"
#include "../headers/lt_initiator.hpp"

using namespace sc_core;
using namespace std;

SC_HAS_PROCESS(lt_initiator);
lt_initiator::lt_initiator( sc_module_name name)
    : sc_module( name )
{
    data = new unsigned char[4];
    SC_THREAD(behavior);
}



void lt_initiator::behavior()
{
    // Create data
    *data = 0xFF;
    
    // Create payload
    payload = new tlm::tlm_generic_payload ();
    payload->set_address(0xFF);
    payload->set_data_ptr(data);
    payload->set_data_length(1);
    payload->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

    
    // This is where the magic happens
    sc_time delay = SC_ZERO_TIME;	
    socket->b_transport(*payload, delay);


    // Now the transaction is completed, check status and proceed
    status = payload->get_response_status();
    if(status == tlm::TLM_OK_RESPONSE)
    {


	// This is how simulation time progresses
	// The process yields to the systemc kernel
	wait(delay);
	cout << sc_time_stamp() << " COMPLETED" << endl;
	cout << "Data " << (*data+'0') << endl;

    }
    else
    {

	cout << sc_time_stamp() << " ERROR" << endl;

    }

    return; 
}
