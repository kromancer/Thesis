#include "../headers/top.hpp"


top::top( sc_core::sc_module_name name )
    : sc_core::sc_module( name ),
      initiator( "initiator" ),
      target   ( "target",
		 sc_core::sc_time(10, sc_core::SC_NS),
		 sc_core::sc_time(10, sc_core::SC_NS),
		 sc_core::sc_time(10, sc_core::SC_NS)),
      bus      ( "bus" )
{
    // Socket binding
    initiator.socket(bus.target_socket);

    bus.initiator_socket(target.socket);
}
