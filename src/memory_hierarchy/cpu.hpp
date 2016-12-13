#ifndef CPU_H
#define CPU_H

#include "memory_hierarchy_configuration.hpp"
#include "systemc.h"
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include <queue>
#include <fstream>


#define SC_INCLUDE_DYNAMIC_PROCESSES
struct Cpu : public sc_core::sc_module
{
    Cpu(sc_core::sc_module_name _name, int id);
    void run();
    int id;
    std::ifstream traceFile;
    std::queue<Event> memTrace;
    tlm_utils::simple_initiator_socket<Cpu> socket;
 
};

#endif
