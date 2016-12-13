#include "memory_hierarchy_configuration.hpp"

tlm::tlm_extension_base* Event::clone() const
{
    Event *ev = new Event;
    return ev;
}

void Event::copy_from(tlm_extension_base const &ext)
{
    
}

Event::~Event()
{

}
