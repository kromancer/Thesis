#include "mpi.h"
#include <string>




enum Airport {SKG, ARL, TXL};

const int lookahead[3][3] = { {0,3,2}, {3,0,1}, {2,1,0} };

Airport airports[3] = {SKG, ARL, TXL};
std::string airport_to_string[3] = { "SKG", "ARL", "TXL"};

int indegree[3] = {2,2,2};

const int sources[3][2] = { {1,2}, {0,2},  {0,1} };

const int sourceweights[3][2] = { {1,1}, {1,1}, {1,1}};


