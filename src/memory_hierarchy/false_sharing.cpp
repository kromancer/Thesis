#include <array>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/thread/barrier.hpp>
#include "annotation.h"

#define N 100

	
struct Danger
{
    int x[4];
};

Danger hazard[N];
boost::barrier rendezvous(4);
boost::mutex display;
boost::mutex id_counter;
int64_t id=0;	



void run()
{
    {
	boost::lock_guard<boost::mutex> lock(display);
	std::cout << boost::this_thread::get_id() << " is up and running!"<< std::endl;
    }

    int64_t my_id;
    {
	// Register thread with the memory trace environment
	// output will contain memory accesses only for registered threads
	boost::lock_guard<boost::mutex> lock(id_counter);
	atomic_trace::register_thread(id);
	my_id = id;
	id++;
    }

    boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
    rendezvous.wait();


    // We are only interested in tracking memory accesses
    // in this specific area of the code.
    // This is were false sharing occurs.
    atomic_trace::start_roi();
    for(int i=0; i<N; i++)
	hazard[i].x[my_id] = my_id;
    atomic_trace::end_roi();    


}




void initialize( boost::thread& t){
    static int index=0;
    t = boost::thread( run );

    // One thread per physical core
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
