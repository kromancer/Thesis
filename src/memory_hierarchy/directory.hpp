#ifndef DIRECTORY_H
#define DIRECTORY_H


#include "memory_hierarchy_configuration.hpp"
#include "esu_fsm.hpp"
#include "systemc.h"
#include "tlm.h"
#include "tlm_utils/multi_passthrough_target_socket.h"
#include "tlm_utils/multi_passthrough_initiator_socket.h"

using namespace esu_fsm;

struct Directory: public sc_module
{
    Directory(sc_module_name);
    Esu_machine *blocks;
    tlm_utils::multi_passthrough_target_socket<Directory> inSocket;
    tlm_utils::multi_passthrough_initiator_socket<Directory> outSocket;
    void serviceCache(int, tlm::tlm_generic_payload&, sc_time&);
};

#endif
