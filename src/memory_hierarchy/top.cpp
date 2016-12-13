#include "cache.hpp"
#include "directory.hpp"
#include "cpu.hpp"


struct Top: sc_module
{
    Cpu *cpu0, *cpu1;
    Cache *cache0, *cache1;
    Directory *dir;

    Top(sc_module_name);
};

Top::Top(sc_module_name _name):
    sc_module(_name)
{
    cpu0 = new Cpu("cpu0",0);
    cpu1 = new Cpu("cpu1",1);
    cache0 = new Cache("cache0", 0);
    cache1 = new Cache("cache1", 1);
    dir    = new Directory("directory");

    cpu0->socket.bind(cache0->cpuSocket);
    cpu1->socket.bind(cache1->cpuSocket);

    cache0->dirInitSocket.bind(dir->inSocket);
    cache1->dirInitSocket.bind(dir->inSocket);

    dir->outSocket.bind(cache0->dirTargSocket);
    dir->outSocket.bind(cache1->dirTargSocket);
}



int sc_main(int argc, char **argv)
{
    
    Top *top = new Top("top");

    sc_core::sc_report_handler::set_actions (SC_WARNING, SC_DO_NOTHING);
    sc_core::sc_start(200, sc_core::SC_NS); 

    return 0;
}
