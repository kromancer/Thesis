#include "cpu_configuration.hpp"
#include "cache.hpp"
#include <fstream>

#include <queue>
#include <string>
#include <sstream>
#include <iostream>
#include <boost/lexical_cast.hpp>






class Cpu
{
public:
    Cpu(int id);
    Event run();
private:
    int id;
    std::ifstream traceFile;
    std::queue<Event> memTrace;
};


Event Cpu::run()
{
    Event temp = memTrace.front();
    memTrace.pop();
    return temp;
}



// The constructor is mainly conserned with parsing the data from the trace
Cpu::Cpu(int id):
    id(id)
{
    traceFile.open("thread_"+ std::to_string(id+1) + ".out");
    std::string line;
    uint64_t time = 0;
    while( std::getline(traceFile, line) )
    {
	// Parse data from trace
	std::string address, op;
	std::istringstream lineS(line);
	lineS >> op >> address;

	// Incorporate them in an event
	Event temp;
	temp.source = CPU;
	temp.destination = CACHE;
	temp.timestamp = time;
	time += DENSITY;
	temp.address = boost::lexical_cast<uint64_t>(address);
	if( op=="r")
	    temp.operation = READ;
	else
	    temp.operation = WRITE;
	memTrace.push(temp);
    }
}





int main(int argc, char *argv[])
{
    Cpu cpu(0);
    Cache cache;


    Event test = cpu.run();
    Event temp;

    cache.compute(test, temp);

    test.operation = DATA_REPLY;
    cache.compute(test, temp);
    
    return 0;
}




