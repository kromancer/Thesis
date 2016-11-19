#include <vector>
#include <algorithm>
#include <iostream>

using namespace std;


struct Event
{
    int timestamp;
    int id;
    int destination;
    int source;

    bool operator<(const Event& right) const { return timestamp < right.timestamp;}
};



int main(int argc, char *argv[])
{

    Event a,b;

    a.timestamp = 0;
    b.timestamp = 10;
    
    vector<Event> test;
    test.push_back(a);
    test.push_back(b);

    sort(test.begin(), test.end());


    for(auto it=test.begin(); it!=test.end(); it++)
	cout << it->timestamp << endl;

    
    return 0;
}
