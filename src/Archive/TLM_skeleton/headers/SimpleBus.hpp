#include "tlm.h"
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/simple_initiator_socket.h"


class SimpleBus : public sc_core::sc_module
{
public:
    // Sockets
    tlm_utils::simple_target_socket<SimpleBus>    target_socket;
    tlm_utils::simple_initiator_socket<SimpleBus> initiator_socket;

    // Constructor
    SimpleBus(sc_core::sc_module_name name);

private:
    // Method required by the target socket
    void custom_b_transport(tlm::tlm_generic_payload &trans, sc_core::sc_time& delay);    
};
