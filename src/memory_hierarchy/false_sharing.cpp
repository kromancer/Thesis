#include <array>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/thread/barrier.hpp>
#include "annotation.h"

#define N 10
	
struct Danger
{
    int x[4];
};

Danger hazard[N];

boost::barrier rendezvous(4);
boost::mutex display;




void thrash_cache()
{
    {
	boost::lock_guard<boost::mutex> lock(display);
	std::cout << boost::this_thread::get_id() << std::endl;
    }

    atomic_trace::start_roi();
    for(int i=0; i<N; i++)
	hazard[i].x[2] = rand();
    atomic_trace::end_roi();
}



void run()
{
    // Give some time for thread relocation to take effect
    boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
    rendezvous.wait();
    thrash_cache();
}



void initialize( boost::thread& t){
    static int index=0;
    
    t = boost::thread( run );

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(index, &cpuset);
    pthread_setaffinity_np(t.native_handle(), sizeof(cpu_set_t), &cpuset);

    index+=2;
};


void finalize( boost::thread& t)
{
    t.join();
}



int main(int argc, char *argv[])
{
    std::array<boost::thread,4> t;

    std::for_each(t.begin(), t.end(), initialize);
    std::for_each(t.begin(), t.end(), finalize);
    
    return 0;
}
