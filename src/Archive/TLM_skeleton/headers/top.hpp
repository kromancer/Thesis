#include "tlm.h"
#include "lt_initiator.hpp"
#include "lt_target.hpp"
#include "SimpleBus.hpp"


class top : public sc_core::sc_module
{
public:
    top( sc_core::sc_module_name name);

private:
    lt_initiator    initiator;
    lt_target       target;
    SimpleBus       bus;
};
