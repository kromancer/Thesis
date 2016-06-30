#include "tlm.h"
#include "../headers/lt_target.hpp"

using namespace std;
using namespace sc_core;


lt_target::lt_target(
    sc_module_name name,
    sc_time accept_delay,
    sc_time read_response_delay,
    sc_time write_response_delay)
    :
    sc_module (name),
    accept_delay(accept_delay),
    read_response_delay(read_response_delay),
    write_response_delay(write_response_delay)
{
    socket.register_b_transport(this, &lt_target::custom_b_transport);
}


void lt_target::custom_b_transport( tlm::tlm_generic_payload &payload, sc_core::sc_time &delay)
{
    cout << "Target Executing" << endl;
    
    *(payload.get_data_ptr()) = 0x00;
    payload.set_response_status(tlm::TLM_OK_RESPONSE);
    delay = delay + accept_delay + write_response_delay ;
    

    return;
}
