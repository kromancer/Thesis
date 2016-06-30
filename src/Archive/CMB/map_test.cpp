#include <map>
#include <iostream>
#include <array>

using namespace std;


int main(int argc, char *argv[])
{

    array< map<int, int>, 2 >  mymap;

    for(int i=0; i < 2; i++){

	for (int j=0; j < 10; j++) {
	    mymap[i].insert ( pair<int,int>(j*10, j*10)  );
	}

    }


//***************************

    int TOUT=55;
    int TIN=50;
    
    auto it1 = mymap[0].lower_bound(TOUT);

    if (it1->first != TOUT)
	it1--;
    
    auto it2 = mymap[0].find(TIN);

    int count=0;
    for( auto it = it1; it != it2; it-- ){
	cout << it->first << endl;
	count++;
    }

    cout << count << endl;
//****************************
    
    return 0;
}
