#include "process.hpp"
#define SIM_DUR 10




template<int NUM_LINES>
Process<NUM_LINES>::Process(array<Line, NUM_LINES>& lines_)
    : lines(lines_)
{
}


template<int NUM_LINES>
void Process<NUM_LINES>::eventLoop()
{
    while(myTime < SIM_DUR)
    {
	/* Receive messages:
         *    - Block until each line contains at least one message.
         *    - How many of bytes are you receiving? let us say sizeIn.
         *    - REMEMBER: Buffering happens on the sender side.
         *    - REMEMBER: I just need to check whether there is a value from a previous recv.
         *    - REMEMBER: After receiving the messages you need to update the timestamp_in for every input line.
         */
	int numRecvPending=0;
	for(int i=0; i<NUM_LINES; i++)
	{
	    if (lines.at(i).empty)
	    {
		void *buf   = lines.at(i).bufIn;
		int   rank  = lines.at(i).rank;
		int   sizeIn= lines.at(i).sizeIn;
		int   tag   = lines.at(i).tagIn;
		MPI_Request *req = &(lines.at(i).reqIn);
		MPI_Irecv( buf, sizeIn, MPI_CHAR, rank, tag, MPI_COMM_WORLD, req);
		numRecvPending++;
	    }
	}
	MPI_Request *array_of_requests = new MPI_Request[numRecvPending];
	MPI_Status  *array_of_statuses = new MPI_Status[numRecvPending];
	for(int i=0; i<NUM_LINES; i++)
	{
	    if (lines.at(i).status)
		array_of_requests[i] = lines.at(i).reqIn;
	}
	MPI_Waitall(numRecvPending, array_of_requests, array_of_statuses);
	// Update line timestampIn, clear status field, set empty field
	for (int i=0; i<NUM_LINES; i++)
	{
	    if (lines.at(i).status)
	    {
		lines.at(i).status = 0;
		lines.at(i).empty  = 0;
		lines.at(i).timestamp_in = lines.at(i).bufIn[100];
	    }
	}
	

	
	/* Computation:
         *    - Select line with minimum timestamp
         *    - If the message received is a NULL one, discard it and proceed
         *    - Make a payload and call the recv function
         *    - UPDATE LOCAL TIME
         *
         * Do not forget:
         *    - Think about storing a trace!
         */
	int min   = -1;
	int min_i =  0;
	for(int i=0; i<NUM_LINES; i++)
	{
	    if ( lines[i].status )
		if ( min > lines[i].timestamp_in || min == -1)
		{
		    min   = lines[i].timestamp_in;
		    min_i = i;
		}
	}
	// Update local time, and flip the signal for this line; now it is empty
	myTime = lines[min_i].timestamp_in;
	lines[min_i].empty = 1;
	// Only if the message is a not-null do any computation
	if (lines[min_i].notNull)
	{
	    sc_core::sc_time delay;
	    module->recv( bufToPayload(lines.at(min_i).bufIn), delay);
	    myTime += delay.to_seconds();
	}


	

	/* Send Messages:
         *    - With my timestamp
         *    - To every line
         *
         * Do not forget:
         *    - Check whether you can use output buffers
         *    - You need a payloadToBuf function
         */  
	for (int i=0; i<NUM_LINES; i++)
	{
	    void *buf    = lines.at(i).bufIn;
	    int   rank   = lines.at(i).rank;
	    int   sizeOut= lines.at(i).sizeOut;
	    int   tag    = lines.at(i).tagIn;
	    MPI_Request *req = &(lines.at(i).reqOut);

	    if (i == min_i)
	    {
		
	    }
	    else
	    {
	    // Null messages to these
	    }
	    MPI_Ibsend(buf, sizeOut, MPI_CHAR, rank, tag, MPI_COMM_WORLD, req);
	}
    }
}






