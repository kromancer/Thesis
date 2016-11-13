#ifndef PROCESS_HPP
#define PROCESS_HPP

#include "mpi.h"
#include <vector>
#include <queue>
#include <iostream>
#include <fstream>


struct Event
{
    int timestamp;
    int id;
    int destination;
    int source;

    bool operator<(const Event&  right) const {
	    if (timestamp != right.timestamp)
		return timestamp < right.timestamp;
	    else
		return id < right.id;
	}

    bool operator==(const Event& right) const { return (timestamp == right.timestamp) && (id == right.id);}
};

struct Link
{
    std::queue<Event> fifo;
    // Needed for establishing proper simulation termination condition
    int num_of_inbound_flights;
    bool ready_to_terminate;
};


// FlightSchedule as an alias for a FIFO
typedef std::queue<Event> FlightSchedule;




class Process
{
private:
    std::ofstream airport_log_file;
    std::vector<Event> airport_log;
    std::ifstream flight_schedule_file;
    Link *links;
    FlightSchedule schedule;
    int time;
    int rank;
    MPI_Comm my_neighbors;
    int num_neighbors;
    MPI_Datatype MPI_Event;
    bool no_inbound_flights_in_links;

    
public:    
    Process(int rank);
    ~Process();

    void run();
    void check_for_causality_violation();

    Event compute(Event, int);
    void  communicate(Event*, Event*);
};


#endif
