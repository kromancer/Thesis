#ifndef UTILS_H
#define UTILS_H

#include "mpi.h"
#include "tlm.h"


/******************************************
 * STRUCT: LINE
 *****************************************/
struct Line {
    int notNull;
    // 1 for not NULL - Synchronization messages
    
    int rank;
    
    int empty;
    // 1 for empty 

    int status;
    // 1 for pending

    
    int timestamp_in;
    int timestamp_out;

    int sizeIn;
    int sizeOut;

    int tagIn;
    int tagOut;
    
    unsigned char *bufIn;
    unsigned char *bufOut;
    

    MPI_Request reqIn;
    MPI_Request reqOut;
};


/**********************************************
 * Transform the MPI bufIN to a generic payload
 **********************************************/
tlm::tlm_generic_payload* bufToPayload(unsigned char *buf)
{
    tlm::tlm_generic_payload *gp = new tlm::tlm_generic_payload;
    gp->set_address((uint64_t)*buf);
    return gp;
}








#endif
