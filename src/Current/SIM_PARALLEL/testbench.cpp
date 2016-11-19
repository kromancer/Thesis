#include "topology.hpp"
#include "process.hpp"
#include <random>
#include <functional>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>

using namespace std;

vector<Event> random_schedule, result_parallel, result_sequential;
int n; // The size of the random schedule
ofstream csv[3];
ifstream csv_in[3];



void print_schedules()
{
    cout << "*********************************" << endl;
    for (auto it=random_schedule.begin(); it!=random_schedule.end(); it++)
	cout << "@" << it->timestamp << "\tID: " << it->id << endl;

    cout << "*********************************" << endl;
    for (auto it=result_parallel.begin(); it != result_parallel.end(); it++ )
	cout << "@" << it->timestamp << "\tID: " << it->id << "\tFrom: " << it->destination << "\tTo: " << it->source << endl;

    cout << "*********************************" << endl;
    for (auto it=result_sequential.begin(); it != result_sequential.end(); it++ )
	cout << "@" << it->timestamp << "\tID: " << it->id << "\tFrom: " << it->destination << "\tTo: " << it->source << endl;

}


void create_random_flight_schedule()
{
    default_random_engine generator;
    uniform_int_distribution<int> distribution_num_of_flights(6,6);
    uniform_int_distribution<int> distribution_airport(0,2);
    uniform_int_distribution<int> distribution_timestamp(0,10);

    auto random_airport   = bind( distribution_airport, generator);
    auto random_timestamp = bind( distribution_timestamp, generator);
    auto random_number_of_flights   = bind( distribution_num_of_flights, generator);

    n = random_number_of_flights();
    random_schedule.reserve(n);

    // Total size of logged events = 2 * scheduled departures ( one event for the departure and one for the arrival)
    result_parallel.reserve(2*n);
    result_sequential.reserve(2*n);

    for( int i=0; i<n; i++)
    {
	Event e;
	e.timestamp   = random_timestamp();
	e.destination = random_airport();
	e.source      = random_airport();
	while( e.destination == e.source )
	    e.source = random_airport();
	string temp = to_string(e.timestamp) + to_string(e.source) + to_string(e.destination);
	e.id = stoi(temp);
	random_schedule.push_back(e);
    }
}

void segregate_flight_schedule()
{
    
    for(int i=0; i<3; i++)
	csv[i].open("flight_schedule_"+ airport_to_string[i] + ".csv");

    for(int i=0; i<n; i++)
    {
	Event e = random_schedule[i];
	csv[e.source] << e.timestamp << "," << e.id << "," << e.destination << endl;
    }

}


inline void sort_random_flight_schedule()
{
    std::sort(random_schedule.begin(), random_schedule.end());
}

inline void sort_simulation_log()
{
    std::sort(result_parallel.begin(), result_parallel.end());
}


void consolidate_airport_logs()
{
    for(int i=0; i<3; i++)
    {
	csv_in[i].open("airport_log_"+ airport_to_string[i] + ".csv");
	string line;
	while( getline(csv_in[i], line) )
	{
	    stringstream lineStream(line);
	    string number;
	    Event temp;
	    int k=0;
	    while( getline(lineStream, number, ',') )
	    {
		switch (k) {
		case 0: {
		    temp.timestamp = stoi(number);
		    k++;
		    break;
		}
		case 1: {
		    temp.id = stoi(number);
		    k++;
		    break;
		}
		case 2: {
		    temp.source = stoi(number);
		    k++;
		    break;
		}
		default: {
		    temp.destination = stoi(number);
		    break;
		}
		}
	    }
	    result_parallel.push_back(temp);
	}
    }// Parsing of log files ends here
}







void simulate_sequentially_and_compare()
{
    if (result_parallel.size() != 2*n)
    {
	cout << "--------------------------------------------------------------------------" << endl;
	cout << "FATAL SIMULATION ERROR: Inconsistent number of non-NULL events generated" << endl;
	cout << "Will now abort, check airport's log file for further details" << endl;
	MPI_Abort(MPI_COMM_WORLD, 0);
    }

    // For every event in the random schedule
    // Create 2 log entries
    // 1. The departure event
    // 2. The arrival event 
    //    The timestamp of the arrival event is the timestamp of the departure + flight time (lookahead)
    for(int i=0; i<n; i++)
    {
	Event e = random_schedule[i];
	result_sequential.push_back(e);
	e.timestamp += lookahead[e.source][e.destination];
	result_sequential.push_back(e);
    }

    sort(result_sequential.begin(), result_sequential.end());

    if (result_sequential != result_parallel)
    {
	cout << "--------------------------------------------------------------------------" << endl;
	cout << "FATAL SIMULATION ERROR: Sequential and Parallel simulation did not produce same events" << endl;
	cout << "Will now abort, check airport's log file for further details" << endl;
	MPI_Abort(MPI_COMM_WORLD, 0);
    }
}



MPI_Datatype MPI_Event;

int main(int argc, char *argv[])
{
    int my_rank;
    Process *airport;
    
    //*******************
    // MPI INITIALIZATION
    //*******************    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);    
    MPI_Type_contiguous(4, MPI_INT, &MPI_Event);
    MPI_Type_commit(&MPI_Event);

    //********************
    // Simulation Preample
    //********************
    if (my_rank==0)
    {
	create_random_flight_schedule();
	sort_random_flight_schedule();
	segregate_flight_schedule();
    }
    MPI_Barrier(MPI_COMM_WORLD);

    //********************
    // Parallel Simulation
    //********************
    airport = new Process(my_rank);
    airport->run();

    //*************************
    // Post Simulation Actions
    //*************************
    airport->check_for_causality_violation();
    airport->~Process();

    if( my_rank == 0)
    {
	consolidate_airport_logs();
	sort_simulation_log();
	simulate_sequentially_and_compare();


	cout << "--------------------------------------------------------------------------" << endl;
	cout << "SIMULATION SUCCESS:" << endl;
	cout << "1. Causality errors were not detected" << endl;
	cout << "2. Sequential and Parallel simulation match:" << endl;
	cout << "\ta. Generated the same number of events" << endl;
	cout << "\tb. Generated the same events" << endl;
	cout << "--------------------------------------------------------------------------" << endl;
    }
    
    MPI_Finalize();    
    return 0;
}




















/* GRAVEYARD OF POTENTIALLY USEFUL CODE

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
*/

