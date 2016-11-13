#ifndef PROCESS_H
#define PROCESS_H

#include "utils.hpp"
#include "wrapper_if.hpp"
#include <array>
#include <queue>
#include "mpi.h"
using namespace std;

//1: Define simulation duration





/******************************************
 * CLASS: PROCESS
 *****************************************/
/* Issues:
 * 1. Data Type for time: use sc_core::sc_time and define the appropriate transformation functions
 */

template<int NUM_LINES>
class Process {
public:
    Process(array<Line, NUM_LINES>&);
    void eventLoop();
    int  lookahead();
    
private:
    WrapperIF *module;
    array<Line, NUM_LINES>        lines;
    //array<queue<Line>, NUM_LINES> inFIFOs;
    int myTime;
};

#endif
