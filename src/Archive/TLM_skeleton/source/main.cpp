#include "../headers/top.hpp"
#include "tlm.h"

int sc_main(int argc, char **argv)
{

    top top("top");
    
    sc_core::sc_start();

    return 0;
}
