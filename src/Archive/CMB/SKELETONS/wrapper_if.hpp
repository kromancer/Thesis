#ifndef WRAPPER_IF_H
#define WRAPPER_IF_H

#include "tlm.h"

class WrapperIF
{
public:
    virtual void run()=0;
    virtual void xmit(unsigned char, unsigned int);
    virtual void recv(tlm::tlm_generic_payload&, sc_core::sc_time);
};

#endif
