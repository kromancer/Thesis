#include <fstream>
#include <vector>
#include <map>
#include <iostream>
#include "TinyXml/tinyxml2.h"

using namespace std;
using namespace tinyxml2;


/******************************************
 * CLASS: SYSTEM
 *****************************************/
class System: public XMLVisitor{

private:
    struct parsedLine{
	string type;
	string from;
	string to;
	void print(){
	    cout << "Line: " << type << "\t" << from << "\t" << to << endl;
	}
    };

    struct parsedProc{
	int rank;
	vector<parsedLine*> lines;
    };
    
public:
    XMLDocument sysCfg;
    string      simDur;
    int         numProcesses;
    map<string, parsedProc*>    processes;

    System(char *xmlFile)
	:sysCfg(true, COLLAPSE_WHITESPACE),
	 numProcesses(0)
	{
	    sysCfg.LoadFile(xmlFile);
	    sysCfg.Accept(this);
	}

    bool VisitEnter(const XMLElement& t, const XMLAttribute *a){

	if( !strcmp(t.Value(),"system"))
	{
	    simDur = t.Attribute("simulationEnd");
	    cout << "system" << endl;
	}
	
	// FOUND PROCESS TAG
	else if( !strcmp(t.Value(),"process") )
	{
	    numProcesses++;
	    string name = t.FirstChildElement("name")->GetText();
	    parsedProc *temp   = new parsedProc();
	    temp->rank         = numProcesses;
	    auto temp2         = make_pair(name, temp);
	    processes.insert(temp2);
	    cout << "FOUND PROCESS" << endl;
	}
	
	// FOUND LINE TAG+
	else if( !strcmp(t.Value(),"link") )
	{
	    auto parentProcess = t.Parent()->ToElement();
	    parsedLine* l = new parsedLine();
	    l->type = parentProcess->Attribute("role");
	    l->from = parentProcess->FirstChildElement("name")->GetText();
	    l->to   = t.GetText();
	    l->print();
	    processes.at(l->from)->lines.push_back(l);
	    cout << "FOUND LINE" << endl;
	}
	return true;
    }


    string generateCode(int id){
	string pre ("\n//BEGIN_AUTOGENERATED_SECTION:\n");
	string post("//END_AUTOGENERATED_SECTION\n");
	string temp;
	
	switch (id)
	{
	case 1: // process.hpp: Define simulation duration
	    return pre + "#define SIM_DUR " + simDur + "\n" + post;
	    
	case 51: // top.cpp: Initialize lines array
	    for( auto i=processes.begin(); i!=processes.end(); i++)
	    {
		int    numLines = i->second->lines.size();
		if (numLines)
		{
		    int    rank     = i->second->rank;
		    temp +=  "\tcase " + to_string(rank) + ":\n\t{\n";
		    temp += "\t\tarray<Line," + to_string(numLines) + "> lines;\n";
		    for( int j=0; j<numLines; j++)
		    {
			auto line      = i->second->lines[j];
			int  destRank  = processes.at(line->to)->rank;
			temp += "\t\tlines[" + to_string(j) + "].rank = " +  to_string(destRank) + ";";
			temp += "  //Line between " + i->first + " and "  +  line->to +"\n";
		    }
		    temp += "\t\tProcess<" + to_string(numLines) + "> process" + to_string(rank) + "(lines); \n";
		    temp += "\t}\n";
		}
	    }
	    return pre + temp + post;	
	}

	return "";
    }
};





int main(int argc, char *argv[])
{
    System sys(argv[1]);
    string line;    

    /* Do some verbatim copying of files in the SKELETON DIRECTORY 
     * 1. utils.hpp
     * 2. wrapper_if.hpp
     */
    ifstream utils  ("SKELETONS/utils.hpp");
    ofstream utils_ ("SIM_PARALLEL/utils.hpp");
    utils_ << utils.rdbuf();

    ifstream wrapper  ("SKELETONS/wrapper_if.hpp");
    ofstream wrapper_ ("SIM_PARALLEL/wrapper_if.hpp");
    utils_ << utils.rdbuf();
    
    /* process.hpp
     * 1. Define Simulation Duration
     * 2. How many process.cpp ?
     */
    ifstream procHpp   ("SKELETONS/process.hpp");    
    ofstream procHpp_  ("SIM_PARALLEL/process.hpp");
    while ( getline(procHpp, line) )
    {
	if ( line.find("//1:") != string::npos )
	    // Define simulation duration
	    procHpp_ << sys.generateCode(1) << endl;
	else
	    procHpp_ << line << endl;
    }
    
    /* top.cpp
     * 1. Initialize lines array
     */
    ifstream topCpp   ("SKELETONS/top.cpp");    
    ofstream topCpp_  ("SIM_PARALLEL/top.cpp");
    while ( getline(topCpp, line) )
    {
	if ( line.find("//51:") != string::npos )
	    // Initialize lines array
	    topCpp_ << sys.generateCode(51) << endl;
	else
	    topCpp_ << line << endl;
    }



    
    return 0;
}


