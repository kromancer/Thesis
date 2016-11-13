// Compile:      mpicxx -g --std=c++11 process.cpp
// Run:          mpirun -n 3 a.out
// Debug:        mpirun -np 3 xterm -e gdb a.out

#include "process.hpp"
#include "topology.hpp"
#include <sstream>
#include <climits>
using namespace std;



Process::Process(int rank)
    : rank(rank),
      time(0),
      num_neighbors(indegree[rank]),
      no_inbound_flights_in_links(true)
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
    airport_log_file.open("airport_log_"+ airport_to_string[rank] + ".csv");

    // Initialization of simulation termination related variables
    links = new Link[num_neighbors];
    for( int i=0; i<num_neighbors; i++)
    {
	links[i].num_of_inbound_flights = 0;
	links[i].ready_to_terminate = false;
    }
    

    MPI_Info info;
    MPI_Dist_graph_create_adjacent( MPI_COMM_WORLD, num_neighbors, sources[rank], sourceweights[rank], num_neighbors, sources[rank], sourceweights[rank], info, 1, &my_neighbors);
    MPI_Type_contiguous(4, MPI_INT, &MPI_Event);
    MPI_Type_commit(&MPI_Event);
}


Process::~Process()
{
    delete[] links;
    airport_log_file.close();
    flight_schedule_file.close();
}

void Process::run()
{
    Event recvbuf[num_neighbors];
    Event sendbuf;
    Event currentEvent;

    // The airport-process is ready to terminate when all of the following conditions apply:
    // 1. It has exhausted its outbound flight schedule
    // 2. It does not contain inbound flights in its incoming links
    // ------------------------------------------------------------
    // When 1 and 2 become true, the id of Null messages changes from -1 to -5
    //-------------------------------------------------------------
    // 3. Conditions 1 and 2 also apply for its inbound neighbors


    bool ready_to_terminate=false;
    bool have_sent_term_event=false;
    
    // Initialize sendbuf with a null message
    // This is essential for kick-starting the simulation
    sendbuf.timestamp = 0;
    sendbuf.id = -1;

    while (!ready_to_terminate)
    {
	// ***********
	// Communicate
	// ***********
	communicate(&sendbuf, recvbuf);
	

	
	int index;
	bool outbound_flight_created;
	do {
		
	    // ***************************************************************************
	    // SELECT appropriate event to process (currentEvent)
	    // It belongs to the link with the minimum timestamp
	    // Apart from incoming links, take into consideration the flight_schedule link
	    // ***************************************************************************
	    int min   = schedule.front().timestamp;
	    index = -1;
	    outbound_flight_created = false;
	    for( int i=0; i < num_neighbors; i++)
	    {
		if (min>links[i].fifo.front().timestamp)
		{
		    min = links[i].fifo.front().timestamp;
		    index = i;
		}
	    }
	    if (index > -1)
	    {
		// Event came from incoming links (some neighbor)
		currentEvent = links[index].fifo.front();
		links[index].fifo.pop();
	    }
	    else
	    {
		// Event came from flight_schedulre
		currentEvent = schedule.front();
		schedule.pop();
		outbound_flight_created = true;
	    }



	    // The essence of a DE simulation
	    time = currentEvent.timestamp;


	    //*********
	    // COMPUTE 
	    //*********
	    // Is this neighboring process ready to terminate?
	    if ( currentEvent.id == -5 )
		links[index].ready_to_terminate = true;
	    sendbuf = compute(currentEvent, index);


	    
	}while( !outbound_flight_created && !links[index].fifo.empty());



        //*********************************
	// Check for TERMINATION CONDITION
	//*********************************
	bool neighbors_ready_to_terminate=true;
	for( int i=0; i<num_neighbors; i++)
	    neighbors_ready_to_terminate &= links[i].ready_to_terminate;

	
	if ( no_inbound_flights_in_links && schedule.front().timestamp==INT_MAX && neighbors_ready_to_terminate && have_sent_term_event)
	    ready_to_terminate = true;

	
	if( no_inbound_flights_in_links && schedule.front().timestamp==INT_MAX && sendbuf.id==-1)
	{
	    sendbuf.id=-5;
	    have_sent_term_event = true;
	}
    } // End of while loop
} // End of run function


void Process::check_for_causality_violation()
{
    for( auto it = airport_log.begin(); it != airport_log.end()-1; it++)
	if( it->timestamp > (it+1)->timestamp )
	{
	    cout << "--------------------------------------------------------------------------" << endl;
	    cout << "FATAL SIMULATION ERROR: Causality Violation was detected on Airport: " << airport_to_string[rank] << endl;
	    cout << "Will now abort, check airport's log file for further details" << endl;
		    
	    MPI_Abort(MPI_COMM_WORLD, 0);
	}
}





void Process::communicate(Event *sendbuf, Event *recvbuf)
{
    MPI_Neighbor_allgather(sendbuf, 1, MPI_Event, recvbuf, 1, MPI_Event, my_neighbors);
    for( int i=0; i < num_neighbors; i++)
    {
	recvbuf[i].timestamp += lookahead[sources[rank][i]][rank];
	links[i].fifo.push( recvbuf[i] );
	// Is this a relevant inbound flight?
	// Knowledge needed for establishing proper simulation termination condition
	if (recvbuf[i].id > 0 && recvbuf[i].destination == rank)
	    links[i].num_of_inbound_flights++;
    }
}
















Event Process::compute(Event e, int index)
{
    // Outbound flight
    if (e.source == rank)

    {
	airport_log.push_back(e);
	airport_log_file << e.timestamp <<  "," << e.id << "," << e.source << "," << e.destination << endl;
	return e;
    }
    // This branch always respond with a null message
    else
    {
	// Inbound flight
	if( e.id > 0 && e.destination==rank)
	{
	    airport_log.push_back(e);
	    airport_log_file << e.timestamp <<  "," << e.id << "," << e.source << "," << e.destination << endl;
	    
	    // Meta-Computation needed for proper simulation termination 
	    links[index].num_of_inbound_flights--;
	    int k=0;
	    for( int i=0; i<num_neighbors; i++)
		if( links[index].num_of_inbound_flights == 0 )
		    k++;
	    if (k==num_neighbors)
		no_inbound_flights_in_links=true;

	}
	e.timestamp = time;
	e.id = -1;
	e.source = rank;
	e.destination = -1;
    }
    return e;
};














/* Graveyard of potentially useful code
airport_log_file <<"@" << currentEvent.timestamp <<  "\tFrom: " << airport_to_string[currentEvent.source] << "\t" << "ID: " << currentEvent.id << endl;
*/
