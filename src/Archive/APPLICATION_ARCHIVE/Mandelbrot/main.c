/* 
 * THE MANDELBROT SET
 *
 * Notes:
 * If you notice lines or blocks that seem to be skipped, do not worry;
 * clients avoid traversing through the whole line buffers, and there might
 * be some lines that are left unprocessed until the very end.  The clients
 * are still busy all the time.  This occurs often when communication is
 * very fast, and the master is able to feed the slaves one line at a time.
 *
 * The design idea:
 * To have a scalable (to hundreds of slaves) implementation,
 * which is tolerant to slow and lagging communication.
 * For example, nothing is assumed of the message order,
 * just that messages eventually will get through in some order.
 *
 * Implementation:
 * Each client has a buffer of rows.  These rows can be one of the following:
 * 1. Free
 * 2. Reserved for receiving (only the amount is reserved, not specific rows)
 * 3. Data for an old view (will get cleaned automatically once encountered)
 * 4. Data for this current view (will be selected to be rendered in an arbitrary order)
 * 5. Data for a future view (they are skipped until they are current)
 * At start, clients report the master of how many 
 * free (ready to accept) rows they have. 
 * In a loop, clients process one row and then check for new messages.  
 * If there are no rows to process, clients will block while waiting for messages.
 * Upon receiving a new view (new dimensions), a view count is incremented
 * and all rows for the current view are considered obsolete 
 * (but purged only upon an encounter).
 * New rows are accepted for current and future views, and each time a pack
 * of rows is received, client reports (async) how many new free lines it has.
 * Clients don't spam the master, and have only one outstanding message
 * at all times.  In regular situations, they report 5-50% of the buffer size as
 * being free, but if the master is underloaded, it might be just 1 row.
 * Note, that very big buffer sizes can be used, because the buffer is not
 * traversed through in each loop.
 *
 * The master thread also avoids spamming the communication system.
 * It sends row packs to clients in a synchronized fashion, forcing other clients
 * to cumulate the free count while processing.  When it is a client's turn, 
 * the master sends it a pack of rows (to fill all the reported free slots, unless
 * a smaller pack size is forced (HEIGHT/client count)), and the client eventually
 * replies with an update to its free slot situation.
 * Because of this, communication stays rather constant, and when there are
 * very many clients, the row pack sizes just get bigger, but the master doesn't
 * choke and clients do not run out of computation.
 *
 * compile: mpicc -mpe=graphics mandelbrot.c -o mandelbrot -lm
 * run: mpicc mpiexec -n 20 ./mandelbrot
 *
 * Click the window to quit.
 *
 * set "#define DEBUG 1" in the code below if you would like to see how the
 * execution goes.  It gets rather messy, though, but has allowed me to confirm
 * that the architecture works as planned.
 */

#include "mpi.h"
#define MPE_GRAPHICS
#include "mpe.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIDTH 1024
#define HEIGHT 768
#define BUFSIZE 25 // How many row requests to keep in each client
#define DEBUG 0 // Debug messages (true/false)

int procc, rank; // Process count and the current rank
static MPE_XGraph graph;
static MPE_Color color[256];
static MPE_Point line[WIDTH];

typedef struct {
    double r, i;
} complex;

typedef struct {
    int rowNumber;
    int view; // In which consecutive view this line belongs to
} rowInfo;

enum { // Communication tags.  Enumeration type casts to integer, so we can use this directly
    tag_free, // Clients inform the server how many free spots they have in their buffer
    tag_dim, // Server informs clients of new dimensions
    tag_rows // Server hands out rows to clients
};


int mpiInit(int argc, char **argv) {
    if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
        fprintf(stderr, "Failure in initializing MPI\n");
        return 100;
    }

    if (MPI_Comm_size(MPI_COMM_WORLD, &procc) != MPI_SUCCESS) {
        fprintf(stderr, "Couldn't get the process count\n");
        MPI_Finalize();
        return 101;
    } else if (procc < 2) {
        fprintf(stderr, "Must have at least 2 processes (have %d)\n", procc);
        MPI_Finalize();
        return 102;
    }

    if (MPI_Comm_rank(MPI_COMM_WORLD, &rank) != MPI_SUCCESS) {
        fprintf(stderr, "Couldn't get rank for a process\n");
        MPI_Finalize();
        return 103;
    }

    MPE_Open_graphics(&graph, MPI_COMM_WORLD, NULL, 0, 0, WIDTH, HEIGHT, 0);

    MPE_Make_color_array(graph, 256, color);

    if (DEBUG && !rank) // Root only
        printf("MPI Initialized\n");
    
    return 0;
} // mpiIinit


// We're using the same naming, and the same function.
// (As this assignment is about parallel programming, 
// I think it's acceptable to just copy this)
int calc_mandel(complex c) {
    int count = 0;
    int max = 255;
    complex z;
    double len2, temp;
    z.r = z.i = 0.0;
    do {
        temp = z.r*z.r - z.i*z.i + c.r;
        z.i = 2.0*z.r*z.i + c.i;
        z.r = temp;
        len2 = z.r*z.r + z.i*z.i;
        if (len2 > 4.0) break;
        count++;
    } while (count < max);
    return count;
} // calc_mandel()


// This is the final step of a row
void processRow(int row, double *dim) {
    int i;
    double rowPosX; // Where in the complex plane this row belongs
    double rowPosY; 
    double advanceX; // How wide is a pixel in the complex plane
    complex c;

    // If we had a framebuffer, we could just memcpy the color index data into it,
    // but now we have to loop through the points (inefficient).
    // (Don't blame me, blame the MPE requirement in the assignment :-)
    rowPosY = dim[1] + dim[3]*((double)row/(double)HEIGHT);
    rowPosX = dim[0];
    advanceX = dim[2]/(double)WIDTH; // This gets to 0.0 when zoomed close enough == horizontal bands
    c.i = rowPosY;
    c.r = rowPosX;

    for (i = 0; i < WIDTH; ++i) {
        line[i].y = row;
        line[i].x = i;
        c.r += advanceX;
        line[i].c = color[calc_mandel(c)];
    }
    MPE_Draw_points(graph, line, WIDTH); // Drawing the line
    MPE_Update(graph);
}


void clientExec() {
    rowInfo rowBuffer[BUFSIZE];
    unsigned char free = BUFSIZE; // The amount of free buffer slots

    // We only need to store one row at a time, as we dispose it immediately.
    // The values are 0..255 so unsigned char suits fine.
    unsigned char tempRowData[WIDTH]; 

    int i;
    int quit = 0;
    int procIndex = 0, storeIndex = 0; // procIndex == processing, storeIndex == receiving
    int viewCount = 0; // This is used to detect old row requests
    double dim[4]; // Dimensions of the view

    MPI_Status tempStatus;
    MPI_Request *req; // Rows, dimension, send
    // reportedRows keeps count of how many there are pending from the server, 
    // and reportRows is the send buffer of the last report message
    int reportedRows = BUFSIZE, reportRows; 
    int freeRows = 0; // How many rows are free, that are not reported to server
    int nothingToProcess = 0;

    // The incoming messages
    double dimMsg[4]; // Dimensions
    // Row message:
    // The first value is view number (related to viewCount),
    // the second is the amount of rows given,
    // and is followed by the list of the rows (caps at BUFSIZE).
    int rowMsg[2+BUFSIZE]; 
    int msgId, commFlag;

    // Whether we have already sent an update of our free situation,
    // but not yet received data.  This is used to avoid sending
    // too much updates about how many free slots we have, when the
    // master is choking.  
    // If, due to master's choking or the delay of the communication,
    // the row processing is N times faster than the round trip of
    // a message, then we automatically settle to sending an update
    // saying (on average) that "we have N rows free". (N < BUFSIZE)
    // If N > BUFSIZE, we request row counts of: BUFSIZE, 1, BUFSIZE, 1, ...
    // So you should make BUFSIZE large enough, but so that BUFSIZE < HEIGHT/processes
    int responsePending;

    // Sending an asynchronous message telling that we have BUFSIZE free spots
    for (i = 0; i < BUFSIZE; ++i)
        rowBuffer[i].view = -1; // Marking the row as being free

    req = (MPI_Request*) malloc(sizeof(MPI_Request) * 3);
    
    reportRows = reportedRows;
    MPI_Isend(&reportRows, 1, MPI_INT, 0, tag_free, MPI_COMM_WORLD, &req[2]);
    responsePending = 1;

    // Starting 2 nonblocking receives, for dimensions and row data
    MPI_Irecv(dimMsg, 4, MPI_DOUBLE, 0, tag_dim, MPI_COMM_WORLD, &req[1]);
    MPI_Irecv(rowMsg, 2+BUFSIZE, MPI_INT, 0, tag_rows, MPI_COMM_WORLD, &req[0]);

    if (DEBUG)
        printf("Client %d: Initialized (buffer size %d), switching to main loop\n", rank, BUFSIZE);

    while (!quit) {
        if (DEBUG)
            printf("Client %d: Begin loop.  %d processable rows\n\tBUFSIZE %d, free rows %d, rows in request %d, procIndex %d, storeIndex %d\n",
                    rank, BUFSIZE-freeRows-reportedRows, BUFSIZE, freeRows, reportedRows, procIndex, storeIndex);

        // If reportedRows + freeRows == BUFSIZE, we have nothing to do until we get
        // a task from the master.  Therefore we start waiting for a message.
        // Also, if we previously looped through the data and found out that all we have
        // is future rows, we need to wait for more data or a dimension update.
        if (reportedRows + freeRows == BUFSIZE
            || nothingToProcess) {
            if (DEBUG)
                printf("Client %d: Empty or only future rows.  Waiting for any message.\n", rank);

            MPI_Waitany(2, req, // Rows and dimension messages were the first 2
                    &msgId, &tempStatus); // No need for status, we know the length of the row message after parsing
        } else {
            // Testing
            MPI_Testany(2, req, &msgId, &commFlag, &tempStatus);
            if (!commFlag) 
                msgId = -1; // No messages
        }

        // Checking out messages
        switch (msgId) {
            case -1:
                if (DEBUG)
                    printf("Child %d: No messages\n", rank);
                break;

            case 0: // Receiving new rows

                // If the view number is 0, we quit.
                if (!rowMsg[0]) {
                    if (DEBUG)
                        printf("Child %d: We got quit signal, aborting\n", rank);
                    quit = 1;
                    break;
                }

                responsePending = 0; // We should send a new request, if we have free spots

                if (DEBUG)
                    printf("Child %d: We got %d rows for view %d (reported buffer spots after this: %d)\n",
                            rank, rowMsg[1], rowMsg[0], reportedRows-rowMsg[1]);
                
                // Since we haven't sent more requests than we can handle,
                // no overflow can happen. (We would be forever in this loop in an overflow)
                reportedRows -= rowMsg[1];
                while (rowMsg[1]) {
                    // Finding a spot from the buffer for the new row.

                    // This is a row with old view (view < viewCount), or free (-1).
                    // Note that this also accepts rows for future views!
                    // (storeIndex is not connected to procIndex, they both traverse
                    // separate from each other.
                    for (; rowBuffer[storeIndex].view >= viewCount;
                            storeIndex = (storeIndex+1)%BUFSIZE);
                    // Now storeIndex points to a buffer spot which is free
                    // (either -1 (== free) or belongs to an old view)
                    
                    if (rowBuffer[storeIndex].view != -1) { // We're freeing it if it was old
                        freeRows++; // Freeing this spot
                        rowBuffer[storeIndex].view = -1;

                        if (DEBUG)
                            printf("Child %d: Spot %d was obsolete (%d < %d)\n",
                                    rank, storeIndex, rowBuffer[storeIndex].view, viewCount);
                    }

                    if (DEBUG)
                        printf("Child %d: Storing request for row %d (view %d) to buffer spot %d.\n",
                                rank, rowMsg[1+rowMsg[1]], rowMsg[0], storeIndex);

                    rowBuffer[storeIndex].view = rowMsg[0];
                    rowBuffer[storeIndex].rowNumber = rowMsg[1+rowMsg[1]];

                    rowMsg[1]--; // One less to go
                }

                if (DEBUG)
                    printf("Child %d: New row processing ended, spawning a new recv\n", rank);

                // And receiving a new row message (at some point)
                MPI_Irecv(rowMsg, 2+BUFSIZE, MPI_INT, 0, tag_rows, MPI_COMM_WORLD, &req[0]);

                break;

            case 1: // New dimensions

                viewCount++;
                memcpy((void*)dim, (void*)dimMsg, sizeof(double)*4);

                if (DEBUG)
                    printf("Child %d: Received new dimensions (x %.3f y %.3f w %.3f h %.3f), increasing viewCount to %d\n",
                           rank, dim[0], dim[1], dim[2], dim[3], viewCount);
    
                MPI_Irecv(dimMsg, 4, MPI_DOUBLE, 0, tag_dim, MPI_COMM_WORLD, &req[1]);

                break; // Unnecessary
        } // switch(msgId)    

        // If we have received a quit signal, this is the next piece of code that follows.
        if (quit)
            break;

        // It might be that we received dimension update after having nothing to process.
        // In that case, we can't go into this loop, because we still have nothing to process.
        if (freeRows + reportedRows == BUFSIZE)
            continue;

        // Searching the next processable row.
        // It might be possible, that we were given a full buffer of row requests
        // for a future view.  This might be, for example, because the preceeding
        // dimension message lagged.  Therefore we must escape if we found nothing
        // to process after BUFSIZE iterations.  If that is the case, we will
        // set nothingToProcess to indicate that we should not go into this loop
        // again before receiving more messages.
        for (i = 0; i < BUFSIZE; ++i, procIndex = (procIndex+1)%BUFSIZE) {
            if (rowBuffer[procIndex].view == -1); // Free, ignore
            else if (rowBuffer[procIndex].view < viewCount) {
                // Obsolete, free
                if (DEBUG)
                    printf("Child %d: Slot %d is obsolete (%d < %d), freeing in proc loop\n",
                            rank, procIndex, rowBuffer[procIndex].view, viewCount);

                rowBuffer[procIndex].view = -1;
                freeRows++;

            } else if (rowBuffer[procIndex].view == viewCount) {
                // This needs processing
                if (DEBUG)
                    printf("Child %d: Processing slot %d (view %d, row %d)\n",
                            rank, procIndex, rowBuffer[procIndex].view, rowBuffer[procIndex].rowNumber);

                processRow(rowBuffer[procIndex].rowNumber, dim);
                rowBuffer[procIndex].view = -1;
                freeRows++;
                // We could loop more, but we try to avoid running out of processing, 
                // so we check the status of communication each loop
                nothingToProcess = 0;
                break;
            } // What remains are the future rows (i.e. rows for a view whose dimensions aren't declared yet)
        }
        if (i == BUFSIZE) // We didn't find anything
            nothingToProcess = 1;

        // If there are free rows, and if the master has replied us since the last report,
        // and if the previous asynchronous send has already finished,
        // we report the master all the new free rows we've had since our last report.
        // (We wait for master's reply so that we don't flood it; if comm. is slow 
        // and there are hundreds of clients, each can't have multiple reports pending at a time.)
        if (freeRows > 0 && !responsePending) {
            MPI_Test(&req[2], &commFlag, &tempStatus);
            //printf("rvalue: %d, comm %d, success? %d\n", rvalue, commFlag, rvalue == MPI_SUCCESS);
            if (commFlag) {
                if (DEBUG)
                    printf("Child %d: Previous send of %d slots went through, sending a new one with %d\n",
                            rank, reportRows, freeRows);

                reportedRows += freeRows;
                reportRows = freeRows;
                freeRows = 0;
                MPI_Isend(&reportRows, 1, MPI_INT, 0, tag_free, MPI_COMM_WORLD, &req[2]);
                responsePending = 1;
            }
        }
    } // while (!quit)

    if (DEBUG)
        printf("Child %d: Telling the master we're done\n", rank);

    // We send a blocking message to the master telling that we're ready to quit
    reportRows = 0;
    MPI_Ssend(&reportRows, 1, MPI_INT, 0, tag_free, MPI_COMM_WORLD);

    if (DEBUG)
        printf("Child %d: Terminating execution\n", rank);
} // childExec()


// We already have open listeners for tag_free,
// so we continue listening them (req).
void terminate(MPI_Request *req, int *freeSlots) {
    int i;
    int msg;
    MPI_Status tempStatus;

    // Quitting, we send each client the quit signal and wait
    for (i = 1; i < procc; ++i) {
        if (DEBUG)
            printf("Master: Sending client %d the termination signal\n", i);

        msg = 0; // We might as well use this as the data
        MPI_Ssend(&msg, 1, MPI_INT, i, tag_rows, MPI_COMM_WORLD);

        if (DEBUG)
            printf("Master: Waiting for acknowledgement...\n");

        do {
            MPI_Wait(&req[i-1], &tempStatus);
            // Starting a listen for the next value
            MPI_Irecv(&freeSlots[i-1], 1, MPI_INT, i, tag_free, MPI_COMM_WORLD, &req[i-1]);

            if (DEBUG)
                printf("Master: Received %d free slots from client %d, expected 0\n", 
                        freeSlots[i-1], i);
        } while (freeSlots[i-1]); // We ignore any remaining "give more rows"-calls

        if (DEBUG)
            printf("Master: Client %d has terminated\n", i);
    }

    printf("All clients have terminated.  Master terminating.\n");
} // terminate()


// Returns true on exit, and stores new dimensions over the old ones
int getDrag(double *dim) { // Returns 1 on exit
    int dragX1, dragX2,
        dragY1, dragY2;
    int dragArea;
    static int dragAreaThreshold = 10;
    
    // Reading the new dimensions from the user
    MPE_Get_drag_region(graph, 1, 1, &dragX1, &dragY1, &dragX2, &dragY2);
    if (DEBUG)
        printf("Dragged %d %d %d %d\n", dragX1, dragY1, dragX2, dragY2);

    dragArea = (dragX2-dragX1)*(dragY2-dragY1); // It seems that always dragX2 > dragX1, dragY2 > dragY1

    // We simply compute the new dimensions.
    dim[0] += dim[2]*((double)dragX1/(double)WIDTH);
    dim[1] += dim[3]*((double)dragY1/(double)HEIGHT);
    dim[2] *= ((double)(dragX2-dragX1)/(double)WIDTH);
    dim[3] *= ((double)(dragY2-dragY1)/(double)HEIGHT);

    if (DEBUG)
        printf("Drag area %d, threshold %d\n", dragArea, dragAreaThreshold);

    // If the area that was dragged was < threshold, we exit.
    if (dragArea < dragAreaThreshold)
        return 1;
    else 
        return 0;
} // getDrag()


void serverExec() {
    // Initial values
    double dim[] = { // X, Y, W, H
        -2.0, -2.0, 4.0, 4.0
    };

    int quit = 0;
    int i, j;
    int rowsLeft;
    int viewCount = 0;

    double startTime, tempTime;

    int clients = procc - 1;
    MPI_Request req_rows[clients];
    MPI_Request req_dim[clients];
    MPI_Status sta[clients];
    int completeCount;
    // There should be at most procc-1 requests, one per client,
    // but we allow 2 just in case. 
    int completed[clients*2];
    int freeSlots[clients]; // Total free slots
    int newFreeSlots[clients]; // Received new free slots
    int tempRowCount;
    int tempRowData[2+HEIGHT]; // Send buffer, this is for 1 client and BUFSIZE >= HEIGHT

    // Initially we fork a listener for each client
    for (i = 0; i < clients; ++i) {
        freeSlots[i] = 0; // Resetting free slots
        MPI_Irecv(&newFreeSlots[i], 1, MPI_INT, i+1, tag_free, MPI_COMM_WORLD, &req_rows[i]);
    }

    if (DEBUG)
        printf("Master: Listeners for %d clients forked\n", clients);

    do {
        startTime = MPI_Wtime();
        viewCount++;
        // Announce dimensions.  The optimal solution would be to use broadcast,
        // but we rely on tags, so we can't use it.  Fortunately this phase
        // isn't a big time-consumer.
        if (DEBUG)
            printf("Master: Announcing dimensions (%.3f, %.3f, %.3f, %.3f)\n",
                    dim[0], dim[1], dim[2], dim[3]);
        
        for (i = 0; i < clients; ++i)
            // We can fork and forget, because clients can receive row data
            // before they know the dimensions.
            MPI_Isend(dim, 4, MPI_DOUBLE, i+1, tag_dim, MPI_COMM_WORLD, &req_dim[i]);

        rowsLeft = HEIGHT;
        // While we have rows to give out, we block and wait for any client announcements
        while (rowsLeft) {
            // We need to get responses from clients fairly, and
            // Waitany() might favor some clients more than others,
            // so we use Waitsome, and process incoming requests from all clients.
            if (MPI_Waitsome(clients, req_rows, &completeCount, completed, sta) != MPI_SUCCESS) 
                fprintf(stderr, "Master: Error in reading children status\n");

            if (DEBUG)
                printf("Master: Got %d announcements\n", completeCount);

            // For each client, we read the results and refork the receiver.
            for (i = 0; i < completeCount; ++i) {
                if (DEBUG)
                    printf("Master: Client %d reported %d free slots\n",
                            completed[i], newFreeSlots[completed[i]]);

                // Adding the new spots to what we previously knew
                freeSlots[completed[i]] += newFreeSlots[completed[i]];

                // Restarting the listener
                MPI_Irecv(&newFreeSlots[completed[i]], 1, MPI_INT,
                        completed[i]+1, tag_free, MPI_COMM_WORLD, &req_rows[completed[i]]);

                // We hand out N rows to the client.
                // N is max(rows the client can accept, HEIGHT/clients, rowsLeft).
                // We cap N to HEIGHT/clients so that we have something for every client.
                // We could also do any type of scheduling here, for example give more
                // rows in the beginning of the calculation, etc.
                tempRowCount = (HEIGHT/clients < freeSlots[completed[i]]) ? 
                    HEIGHT/clients : freeSlots[completed[i]]; // Rounding errors in int/int division don't matter
                if (tempRowCount > rowsLeft)
                    tempRowCount = rowsLeft;

                // Example:
                // If we start at HEIGHT = 10, and this client eats up 5 requests,
                // we give it rows 9, 8, 7, 6 and 5.  The next client gets
                // 4, 3, 2, 1 and 0.  After that, while(rowsLeft) exits.
                tempRowData[0] = viewCount;
                tempRowData[1] = tempRowCount;
                for (j = 0; j < tempRowCount; ++j)
                    tempRowData[2+j] = --rowsLeft;

                // Counterintuitively, we are doing a synchronized send here.
                // We can actually sleep in the master loop quite long before clients
                // deplete themselves of processing.  And it is really beneficial not to
                // give out large amounts of small messages, with large pending message queues.
                // Each client accepts new rows after processing one line, therefore
                // BUFSIZE for a client needs to be at least 1*client_count for no stalling to occur.
                // This also keeps the communication at the minimum.
                // NOTE: clients < BUFSIZE is not a serious limitation unless we want to use
                // thousands of clients for small fractal resolution.  But if we really feel that
                // it is constricting, we can spawn several asynchronous sends for different clients,
                // but this increases communication.
                if (DEBUG)
                    printf("Master: Sending %d new rows to client %d, leaving %d free\n", 
                            tempRowCount, completed[i]+1, freeSlots[completed[i]]-tempRowCount);
                MPI_Ssend(tempRowData, tempRowCount+2, MPI_INT, completed[i]+1, tag_rows, MPI_COMM_WORLD);

                // Removing the sent rows from the available pool
                freeSlots[completed[i]] -= tempRowCount;
            } // for
        } // while(rowsLeft)

        // We should wait here for all the clients to finish, but that would have an
        // impact on the user experience: We designed clients so that they can be
        // interrupted before they finish, and they discard old rows automatically.
        // Therefore user can make another zoom as soon as he likes (well, the master
        // has to have got rid of the rowsLeft first)
        // This has an impact on the timing: the time tells how fast the master thread
        // is ready to take another drag event.  There might still be up to
        // BUFSIZE*clients rows left to process.
        tempTime = MPI_Wtime();
        printf("Master: View %d took %.3f seconds\n", 
                viewCount, tempTime - startTime);
        // PS.  If we wanted to do this properly, we would need an extra thread to catch
        // "I'm finished" calls from clients while we sit in the MPE_Get_drag_region function,
        // and then report the time...
        
        if (getDrag(dim))
            quit = 1;
    } while (!quit);

    if (DEBUG)
        printf("Master: Exited the main loop\n");

    terminate(req_rows, newFreeSlots);
} // serverExec


int main(int argc, char **argv) {
    if (mpiInit(argc, argv)) {
        fprintf(stderr, "Couldn't init MPI\n");
        return 10;
    }

    if (!rank && procc-1 > BUFSIZE)
        printf("WARNING: You have %d clients and %d BUFSIZE.\nTo keep all clients busy, BUFSIZE is recommended to be larger than client count.\n", procc-1, BUFSIZE);

    if (rank)
        clientExec();
    else
        serverExec();

    MPI_Finalize();
    return 0;
} // main()
