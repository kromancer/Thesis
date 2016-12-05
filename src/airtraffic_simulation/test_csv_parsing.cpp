#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <queue>
using namespace std;

enum Airport {SKG, ARL, TXL};
Airport airports[3] = {SKG, ARL, TXL};
std::string airport_to_string[3] = { "SKG", "ARL", "TXL"};

struct Event
{
    int timestamp;
    int id;
    int destination;
    int source;
};

struct Link
{
    queue<Event> fifo;
    int timestamp;
};




int main(int argc, char *argv[])
{
    ofstream test_flight_schedule_copy;
    ifstream flight_schedule_file;
    Link flight_schedule;
    flight_schedule_file.open("flight_schedule_"+ airport_to_string[0] + ".csv");
    test_flight_schedule_copy.open("test_flight_schedule_copy.csv");

    string line;
    while( getline(flight_schedule_file,line) )
    {
	stringstream lineStream(line);
	string number;
	Event temp;
	temp.source = 0;
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
	flight_schedule.fifo.push(temp);
    }
    flight_schedule.timestamp = flight_schedule.fifo.front().timestamp;

    int size = flight_schedule.fifo.size();
    for( int i=0; i < size ; i++)
    {
	Event temp = flight_schedule.fifo.front();

	test_flight_schedule_copy << temp.timestamp << "," << temp.id << "," << temp.destination << endl;
	
	flight_schedule.fifo.pop();
    }


    
    return 0;
}
