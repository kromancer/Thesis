#include "tlm.h"
#include "../headers/SimpleBus.hpp"


using namespace std;
using namespace sc_core;


SimpleBus::SimpleBus(sc_module_name name)
    : sc_module(name)
{
    target_socket.register_b_transport(this, &SimpleBus::custom_b_transport);
}




void SimpleBus::custom_b_transport(tlm::tlm_generic_payload &trans, sc_core::sc_time& delay)
{
    // Does nothing, basically forwards the transaction
    initiator_socket->b_transport(trans, delay);
}


