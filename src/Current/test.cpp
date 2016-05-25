#include <set>
#include <array>
#include <map>
#include <iostream>
#include <thread>
#include <chrono>
#include <list>
#include "mpi.h"




using namespace std;
using tuple = pair<int,int>;

static int Z=100;

struct Line{
    int type;
    int rank;
    int timestamp;
    int message;
    MPI_Request request;
    int *buf;
};

struct Compare{
    bool operator()( const pair<Line, Line*>& a, const pair<Line, Line*>& b){
	int value_A = a.first.rank;
	int value_B = 1000 + b.first.rank;
	if (value_A < value_B)
	    return true;
	else
	    return false;
    }
};



template<int INPUT_LINES, int OUTPUT_LINES>
class Physical_Process {
public:
    int lookahead(int j, list<int> inform) {return 5;}
};



template<int INPUT_LINES, int OUTPUT_LINES>
class Logical_Process {
public:
    Logical_Process(){
	for (int i = 0; i < OUTPUT_LINES; i++) {
	    output[i].type = 1;
	    output[i].rank = 5;
	    output[i].timestamp = 0;
	    output[i].message = -1;
	    output[i].buf = new int[2];
	}

	for (int i = 0; i < INPUT_LINES; i++) {
	    input[i].type = 0;
	    input[i].rank = 5;
	    input[i].timestamp = 0;
	    input[i].message = -1;
	    input[i].buf = new int[2];
	}

	make_histories();
	
	T = 0;
	TIN = 0;

	
    }


    void Simulation_Loop(){
	while( T < Z){
	    selection();
	    computation();
	    IO_operation();
	}
	
    }

    void selection(){
	for (int i=0; i < INPUT_LINES; i++) {
	    if (input[i].timestamp == T) {
		next.insert(make_pair(input[i], &input[i]));
	    }
	}
	
	for (int i=0; i < OUTPUT_LINES; i++){
	    if (output[i].timestamp == T && output[i].timestamp < TOUT) {
		next.insert(make_pair(output[i], &output[i]));
	    }
	}
    }


    void computation(){
	for(auto it = next.begin(); it!=next.end(); it++){

	    std::this_thread::sleep_for(std::chrono::seconds(1));
	    // Compute h_ij(T_i, T_i + L_ij(T_i)) from A7
	    // We next compute (NEWT_ij, NEWM_ij) the next tuple to output on line (i,j) from the previous calculation

	    
	    if( it->first.type == 1){
		auto it1 = h[it->first.rank].lower_bound(TOUT);
		if (it1->first != TOUT)
		    it1--;
    
		auto it2 = h[it->first.rank].find(TIN);
		int count=0;
		for( auto it3 = it1; it3 != it2; it3-- ){
		    cout << it3->first << endl;
		    count++;
		}


		
		if (count==0){
		    it->second->timestamp = TOUT;
		    it->second->message   = -1;
		}
		else{
		    it->second->timestamp = it2->first;
		    it->second->message   = it2->second;
		}

		// Send final message
		if(it->second->timestamp > Z)
		{
		    it->second->timestamp = Z;
		    it->second->message   = -1;
		}
	    }
	}
    };

    

    void IO_operation(){
	int count=0;
	MPI_Request *requests = new MPI_Request[next.size()];
	MPI_Status  *statuses = new MPI_Status[next.size()];
	
	for( auto it=next.begin(); it!=next.end(); it++){

	    // OUTPUT REQUESTS
	    if (it->first->type == 0){
		it->second->buf[0] = it->second->timestamp;
		it->second->buf[1] = it->second->message;
		MPI_Issend(it->second->buf, 2, MPI_INT, it->first->rank, MPI_ANY_TAG, MPI_COMM_WORLD, &it->second->mpi_request);
		requests[count] = it->second->mpi_request;
	    }
	    // INPUT REQUESTS
	    else{
		MPI_Irecv(it->second->buf, 2, MPI_INT, it->first->rank, MPI_ANY_TAG, MPI_COMM_WORLD, &it->second->mpi_request);
		requests[count] = it->second->mpi_request;
	    }
	    count++;
	}
	
	// Parallel I/O operation
	MPI_Waitall(count, requests, statuses);

	for( auto it=next.begin(); it!=next.end(); it++){
	    // INPUT
	    if (it->type == 0){
		it->timestamp = it->buf[0];
		it->message = it->buf[1];
	    }
	}

	// Compute T, TIN
	int TIN = 0;
	for( auto it=input.begin(); it!=input.end(); it++){
	    if (it->timpestamp < TIN){
		TIN = it->timestamp;
	    }
	}

	int min_output = 0;
	for( auto it=output.begin(); it!=output.end(); it++){
	    if (it->timpestamp < min_output){
		min_output = it->timestamp;
	    }
	}

	if (TIN < min_output)
	    T = TIN;
	else
	    T = min_output;

	TOUT = TIN + L;


	// Obtain h_ki(T_i, NEWT_i)
	// Compute STATE_i (NEWT_i) from A8
	// Set T_i = NEWT_i

	
	delete[] requests;
	delete[] statuses;
	next.clear();
    };








    
private:
    int T, TIN, TOUT;

    array<Line, INPUT_LINES> input;
    array<Line, OUTPUT_LINES> output;

    set< pair<Line, Line*>, Compare> next;

    Physical_Process<INPUT_LINES, OUTPUT_LINES> physical;
    
    // META SITUATION. WE ALREADY KNOW THE BEHAVIOR OF THE PHYSICAL SYSTEM
    map< int, map<int,int> >  h;

    
    void make_histories(){
	for (int i=0; i < OUTPUT_LINES; i++) {
	    map<int, int> temp;
	    for (int j=0; j < 10; j++) {
		temp.insert(std::pair<int,int>(j*10, j));
	    }
	    h.insert(std::pair<int, map<int,int> >(i,temp));
	}
    }

	
    
};




int main(int argc, char *argv[])
{
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0){
	Logical_Process<0, 1> source;
	source.Simulation_Loop();
    }
    else if(rank == 1){
	Logical_Process<1, 1> process1;
	process1.Simulation_Loop();
    }
    else{
	Logical_Process<1, 0> sink;
	sink.Simulation_Loop();
    }


    cout << "shit" << endl;
    return 0;
}
