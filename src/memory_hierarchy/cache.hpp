#ifndef CACHE_H
#define CACHE_H

#include <iostream>
#include <list>
#include "memory_hierarchy_configuration.hpp"
#include "msi_fsm.hpp"

namespace sc = boost::statechart;

struct Set
{
    Set();
    Msi_machine blocks[8];
    bool hitFlag;
    bool upgradeFlag;
    bool writeBackFlag;
    std::list<uint> invalidBlocks;
    uint killIndex;
    
    void process(const sc::event_base&);
    void insert(Operation op, uint tag);
};



struct Cache
{
    Set sets[numSets(CACHE_SIZE, BLOCK_SIZE, N_WAYS)];
    Operation pending;
    
    void compute( Event&, Event& );
};


#endif
