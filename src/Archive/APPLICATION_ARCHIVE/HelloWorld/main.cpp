#include <iostream>
#include <omp.h>


using namespace std;
int main(int argc, char **argv)
{

    #pragma omp parallel
    {
        #pragma omp critical
	cout << "Hello from thread: " << omp_get_thread_num() << endl;
    }
    cout << "The openmp runtime has reported " << omp_get_max_threads() << " threads" << endl;
    return 0;
}
