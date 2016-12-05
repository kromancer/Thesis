#include "mpi.h"
#include <iostream>
#include <climits>
#include <unistd.h> 

int main(int argc, char *argv[])
{
    int myId, workerLeft, workerRight, numWorkers;
    bool isFirst(false), isLast(false);
    int completedTask(INT_MIN), nextTask(INT_MIN), taskAtHand(INT_MIN);

    // Register with the factory's administration
    MPI_Init(&argc, &argv);
    // What is my id
    MPI_Comm_rank(MPI_COMM_WORLD, &myId);
    // How many workers in the factory
    MPI_Comm_size(MPI_COMM_WORLD, &numWorkers);

    // Who is on my left
    if( myId != 0)
	workerLeft = myId - 1;
    else
    {
	// The first worker has access to the raw materials
	isFirst = true;
	taskAtHand = 0;
	nextTask = 0;
    }

    // Who is on my right
    if( myId != numWorkers-1)
	workerRight = myId + 1;
    else
    {
	isLast = true;
	workerRight = myId;	
    }


    // Life at the factory
    while(1)
    {
	MPI_Request signals[2];
	MPI_Status  statuses[2];
	// Communicate but don't block
	if (!isLast)
	    MPI_Isend(&completedTask, 1, MPI_INT, workerRight, 1, MPI_COMM_WORLD, &signals[0]);
	if (!isFirst)
	    MPI_Irecv(&nextTask, 1, MPI_INT, workerLeft, 1, MPI_COMM_WORLD, &signals[1]);

	// Simulate Work
	taskAtHand++;
	sleep(1);

	// Work is done, prepare for communication
	completedTask = taskAtHand;
	taskAtHand    = nextTask;

	// This is meaningful only to the last worker
	if( completedTask == numWorkers)
	    std::cout << "Car is ready!" << std::endl;

	if (isFirst)
	    MPI_Wait(&signals[0], &statuses[0]);
	else if (isLast)
	    MPI_Wait(&signals[1], &statuses[1]);
	else
	    MPI_Waitall(2, signals, statuses);
    }

    MPI_Finalize();
    return 0;
}

