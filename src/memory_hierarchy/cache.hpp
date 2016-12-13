#ifndef CACHE_H
#define CACHE_H

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

#include <iostream>
#include <list>
#include "memory_hierarchy_configuration.hpp"
#include "msi_fsm.hpp"

using namespace msi_fsm;

struct Set
{
    Set();
    Msi_machine blocks[8];
    bool hitFlag;
    bool upgradeFlag;
    bool writeBackFlag;
    bool evictionFlag;
    std::list<uint> invalidBlocks;
    uint killIndex;
    
    void dispatch(const sc::event_base&);
    void insert(Operation op, uint tag);
};



struct Cache: public sc_core::sc_module
{
    uint id;
    Set *sets;
    Operation pending;
    Cache(sc_core::sc_module_name, uint id);
    ~Cache(){ delete [] sets; };

    tlm_utils::simple_target_socket<Cache>    cpuSocket;
    tlm_utils::simple_initiator_socket<Cache> dirInitSocket;
    tlm_utils::simple_target_socket<Cache>    dirTargSocket;

    void serviceCpu(tlm::tlm_generic_payload&, sc_core::sc_time&);
    void serviceDir(tlm::tlm_generic_payload&, sc_core::sc_time&);
};


#endif
