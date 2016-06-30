#include "tlm.h"
#include "tlm_utils/simple_target_socket.h"



class lt_target : public sc_core::sc_module
{
public:
    // Socket
    tlm_utils::simple_target_socket<lt_target> socket;
    // Payload
    tlm::tlm_generic_payload *payload;

    // Constructor
    lt_target
    (sc_core::sc_module_name name,
     sc_core::sc_time accept_delay,
     sc_core::sc_time read_response_delay,
     sc_core::sc_time write_response_delay);

private:
    const sc_core::sc_time    accept_delay;         
    const sc_core::sc_time    read_response_delay; 
    const sc_core::sc_time    write_response_delay; 
    // Method required by the target socket
    void custom_b_transport(tlm::tlm_generic_payload &payload, sc_core::sc_time &delay_time);
};

