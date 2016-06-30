#include "process.hpp"
#include "utils.hpp"
#include <array>
#include "mpi.h"

using namespace std;






int main(int argc, char *argv[])
{
    int n, myRank, numprocs, i;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

    switch (myRank)
    {
    //51: Initialize line array
    default:
	break;
    }

    MPI_Finalize();
    return 0;
}
