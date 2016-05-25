#include "process_graph.hpp"
#include <iostream>
#include <array>
#include <algorithm>
#include <thread>
#include <chrono>
#include "mpi.h"

using namespace std;
using timestamp = int;

enum {MESSAGE};

struct Line{
    int type;
    int rank;
    int timestamp;
    int message;
    MPI_Request *request;
    int *buf;

    bool operator<(const Line& other){
	return timestamp < other.timestamp;
    }
};


constexpr int buf_size(int a, int b){
    return 2*(a+b);
}


template<int N_INPUT, int N_OUTPUT, int Lookahead>
class Logical_process{
private:
    int                         rank, n_processes;
    timestamp                   T;
    array<Line, N_INPUT>        input_lines;
    array<Line, N_INPUT>        output_lines;
    MPI_Request                 input_requests[N_INPUT];
    MPI_Request                 output_requests[N_OUTPUT];
    int                         buf[buf_size(N_INPUT, N_OUTPUT)];


    
public:
    Logical_process(int rank_, int n_processes_) :
	T(0),
	rank(rank_),
	n_processes(n_processes_)
	{
	    int *p = buf;
	    int  j = 0;
	    int  k = 0;
	    for(int i=0; i < n_processes; i++){

		if (g[rank*n_processes + i]){
		    output_lines[j].rank      = i;
		    output_lines[j].timestamp = 0;
		    output_lines[j].buf       = p;
		    output_lines[j].request   = &output_requests[j];
		    p += 2;
		    j++;
		}

		if (g[rank + i*n_processes]){
		    input_lines[k].rank      = i;
		    input_lines[k].timestamp = 0;
		    input_lines[k].buf       = p;
		    input_lines[k].request   = &input_requests[k];
		    p += 2;
		    k++;
		}
	    }
	}
    
    void event_loop(timestamp Z){


	
	/* Initialization:
           - Avoid deadlock in the first receive phase */
	for(auto&& it : output_lines){
	    it.timestamp = T + Lookahead;
	    it.buf[0]    = it.timestamp;
	    it.buf[1]    = it.timestamp;
	    MPI_Issend(it.buf, 2, MPI_INT, it.rank, MESSAGE, MPI_COMM_WORLD, it.request);
	}


		
	while (T < Z){
	    /* Receive communication:
	       - Block until each FIFO contains at least one message */
	    for(auto&& it : input_lines){
	       MPI_Irecv( it.buf, 2, MPI_INT, it.rank, MPI_ANY_TAG, MPI_COMM_WORLD, it.request);
	    }
	    MPI_Waitall(N_INPUT, input_requests, MPI_STATUS_IGNORE);
	    for(auto&& it : input_lines){
		it.timestamp = it.buf[0];
		it.message   = it.buf[1];
	    }

	    
	    /* Process M:
               - M is the message with the smallest timestamp
	       Update T:
               - T becomes M's timestamp */
	    auto it = min_element(input_lines.begin(), input_lines.end());
	    T = it->timestamp;
	    std::this_thread::sleep_for(std::chrono::seconds(1));

	    
	    /*  Outgoing communication:
                Send null message to neighboring LPs:
		- Time Stamp equal to lower bound on time stamp of future messages (T + Lookahead) */
	    for(auto&& it : output_lines){
		it.timestamp = T + Lookahead;
		it.buf[0]    = it.timestamp;
		it.buf[1]    = it.timestamp;
		MPI_Issend(it.buf, 2, MPI_INT, it.rank, MESSAGE, MPI_COMM_WORLD, it.request);
	    }

	}
    }
};



int main(int argc, char *argv[])
{
    int rank, n_processes;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &n_processes);

    make_graph();

    timestamp Z = 10;


    switch (rank) {
    case 0: {
	Logical_process<2, 2, 1> p(rank, n_processes);
	p.event_loop(Z);
	break;
    }
    case 1: {
	Logical_process<2, 2, 1> p(rank, n_processes);
	p.event_loop(Z);
	break;
    }
    case 2: {
	Logical_process<2, 2, 1> p(rank, n_processes);
	p.event_loop(Z);
	break;
    }
    default:
	cout << "Something went wrong in the initialization of the MPI environment";
	break;
    }



    
    MPI_Finalize();
    return 0;
}
