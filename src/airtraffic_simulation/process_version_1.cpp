// Compile:      mpicxx -g --std=c++11 process.cpp
// Run:          mpirun -n 3 a.out
// Debug:        mpirun -np 3 xterm -e gdb a.out


#include <sstream>
#include <iostream>
#include "mpi.h"
#include "topology.hpp"
#include <queue>
#include <fstream>
#include <climits>


using namespace std;


////////////////////////////////////////////////////////////
struct Event
{
    int timestamp;
    int id;
    int destination;
    int source;
};

MPI_Datatype MPI_Event;

// Link as an alias for a FIFO
typedef queue<Event> Link;
typedef queue<Event> FlightSchedule;
////////////////////////////////////////////////////////////


class Process
{
private:
    ofstream airport_log;
    ifstream flight_schedule_file;
    
    Link *links;
    FlightSchedule schedule;
    
    int time;
    int rank;
    MPI_Comm my_neighbors;
    int num_neighbors;
    //MPI_Group world, neighbors_group;
    //MPI_Comm neighbors_comm;
    
public:

    Process(int rank)
	: rank(rank),
	  time(0),
	  num_neighbors(indegree[rank])
	{
	    // ***************************
	    // Initialize airport's flight schedule from appopriate file e.g. flight_schedule_SKG.csv
	    // Also, add one dummy event in the end, with timestamp INT_MAX and ID -2
	    // Necessary when the flight_schedule becomes empty (or is initiated with an empty file)
	    // ***************************
	    flight_schedule_file.open("flight_schedule_"+ airport_to_string[rank] + ".csv");
	    string line;
	    while( getline(flight_schedule_file,line) )
	    {
		stringstream lineStream(line);
		string number;
		Event temp;
		temp.source = rank;
		int i = 0;
		while( getline(lineStream, number, ',') )
		{
		    switch (i) {
		    case 0: {
			temp.timestamp = stoi(number);
			i++;
			break;
		    }
		    case 1: {
			temp.id = stoi(number);
			i++;
			break;
		    }
		    default: {
			temp.destination = stoi(number);
			break;
		    }
		    }
		}
		schedule.push(temp);
	    }
	    Event dummy;
	    dummy.timestamp = INT_MAX;
	    dummy.id = -2;
	    schedule.push(dummy);

	    // Open airport's log file e.g. airport_log_SKG.txt
	    airport_log.open("airport_log_"+ airport_to_string[rank] + ".txt");

	    links = new Link[num_neighbors];

	    MPI_Info info;
	    MPI_Dist_graph_create_adjacent(MPI_COMM_WORLD, num_neighbors, sources[rank], sourceweights[rank], num_neighbors, sources[rank], sourceweights[rank], info, 1, &my_neighbors);
	}

    void run()
	{
	    Event recvbuf[num_neighbors];
	    Event sendbuf;
	    Event currentEvent;
	    
	    // Initialize sendbuf with a null message
	    // This is essential for starting computation by allowing a special event, with timestamp -1, to be selected
	    sendbuf.timestamp = 0;
	    sendbuf.id = -1;

	    while (1)
	    {
		// ***********
		// Communicate
		// ***********
		MPI_Neighbor_allgather(&sendbuf, 1, MPI_Event, recvbuf, 1, MPI_Event, my_neighbors);

		// *****************************************
		// Store messages in Links
		// *****************************************
		for( int i=0; i < num_neighbors; i++)
		{
		    recvbuf[i].timestamp += lookahead[sources[rank][i]][rank];
		    links[i].push( recvbuf[i] );
		}
		
		// ***************************************************************************
		// Select appropriate event to process (currentEvent)
		// It belongs to the link with the minimum timestamp
		// Apart from incoming links, take into consideration the flight_schedule link
		// ***************************************************************************
select_appropriate_event:
		int min   = schedule.front().timestamp;
		int index = -1;
		for( int i=0; i < num_neighbors; i++)
		{
		    if (min>links[i].front().timestamp)
		    {
			min = links[i].front().timestamp;
			index = i;
		    }
		}
		if (index > -1)
		{
		    // Event came from incoming links (some neighbor)
		    currentEvent = links[index].front();
		    links[index].pop();
		}
		else
		{
		    // Event came from flight_schedulre
		    currentEvent = schedule.front();
		    schedule.pop();
		}
		
		//******************************************
		// Compute and create new events
		// Computation is based on the type of event
		// There are 4 different kinds of events
		//******************************************
		if ( rank == 0)
		    cout << time << endl;

		bool eventAdvancedTime = false;
		if ( currentEvent.timestamp > time ) eventAdvancedTime = true;

		// The essence of a DE simulation
		time = currentEvent.timestamp;
		if (index == -1)
		{
		    // Outbound flight
		    // Create an outbound flight and update airport's log
		    sendbuf = currentEvent;
		    airport_log <<"@" << sendbuf.timestamp <<  "\tTo: " << airport_to_string[sendbuf.destination] << "\tID: " << sendbuf.id << endl;
		}
		else if (currentEvent.destination == rank)
		{
		    // Inbound Flight
		    // Update airport's log
		    airport_log <<"@" << currentEvent.timestamp <<  "\tFrom: " << airport_to_string[currentEvent.source] << "\tID: " << currentEvent.id << endl;
		    // Create a null message
		    sendbuf.timestamp = time;
		    sendbuf.id        = -1;
		    sendbuf.destination = -1;

		}
		else
		{
		    // Null or Irrelevant message
		    // Create a null message only if time has advanced
		    // or if the link, from which the current event comes, is empty
		    if (eventAdvancedTime || links[index].empty())
		    {
			sendbuf.timestamp = time;
			sendbuf.id        = -1;
			sendbuf.destination = -1;
		    }
		    else
			// Should be replaced, this is a temporal patch
			goto select_appropriate_event;
		}
		    
	    } // End of while loop

	} // End of run functino
};



using namespace std;

int main(int argc, char *argv[])
{
    int my_rank;
    Process *airport;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);    
    MPI_Type_contiguous(4, MPI_INT, &MPI_Event);
    MPI_Type_commit(&MPI_Event);

    switch (my_rank) {
    case 0:
    {
	airport = new Process(my_rank);
	airport->run();
	break;
    }
    case 1:
    {
	airport = new Process(my_rank);
	airport->run();
	//airport->run(5);
	break;
    }
    case 2:
    {	
	airport = new Process(my_rank);
	airport->run();
	break;
    }
    default:
    {
	cout << "Something is rotten in the state of Denmark" << endl;
	break;
    }
    }
    
    
    MPI_Finalize();
    return 0;
}










//#include "process.hpp"

//template<int NUM_LINES>
//Process<NUM_LINES>::Proc1111ess(){

//BEGIN_AUTOGENERATED_SECTION:
//	inLines[0].rank = 3;  //Line between iss1 and cache
//	inLines[1].rank = 4;  //Line between iss1 and memory
//	inLines[2].rank = 3;  //Line between iss2 and cache
//	inLines[3].rank = 4;  //Line between iss2 and memory
//	inLines[4].rank = 4;  //Line between cache and memory
//END_AUTOGENERATED_SECTION

//}


//template<int NUM_LINES>
//void Process<NUM_LINES>::eventLoop(){

//}





