#ifndef CHANGED 
// threadtest.cc 
//      Simple test case for the threads assignment. 
// 
//      Create two threads, and have them context switch 
//      back and forth between themselves by calling Thread::Yield, 
//      to illustratethe inner workings of the thread system. 
// 
// Copyright (c) 1992-1993 The Regents of the University of California. 
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions. 

#include "copyright.h" 
#include "system.h" 

//---------------------------------------------------------------------- 
// SimpleThread 
//      Loop 5 times, yielding the CPU to another ready thread 
//      each iteration. 
// 
//      "which" is simply a number identifying the thread, for debugging 
//      purposes. 
//---------------------------------------------------------------------- 

void 
SimpleThread(int which) 
{ 
    int num; 

    for (num = 0; num < 5; num++) { 
        printf("*** thread %d looped %d times\n", which, num); 
        currentThread->Yield(); 
    } 
} 

//---------------------------------------------------------------------- 
// ThreadTest 
//      Set up a ping-pong between two threads, by forking a thread 
//      to call SimpleThread, and then calling SimpleThread ourselves. 
//---------------------------------------------------------------------- 

void 
ThreadTest() 
{ 
    DEBUG('t', "Entering SimpleTest"); 

    Thread *t = new Thread("forked thread"); 

    t->Fork(SimpleThread, 1); 
    SimpleThread(0); 
} 
#else 

#include "copyright.h" 
#include "system.h" 
#include "synch.h" 
#include <stdio.h> 
#include <stdlib.h> 
#include "resmanager.h" 
#include "prodcon.h"
#include "elevator.h"
#include "traffic_manager.h"
#include <new>

char pname[20][5]; 
Thread *t[20];

//Resource Manager Variables
ResManager *RM; 
int used=0;         // Keep track of units in use so we can check 
                    // validity of the system. 

//Prodcon Variables
ProdCon *PC;

//Elevator Variables
Elevator *E;

//Bridge Variables
TrafficManager *TM;


void 
RMProcess(int arg) 
{ 
	int i, delay, hold, numunits; 

	// Let's roll some random numbers for a process 

	delay = 1+(int) (100000.0*rand()/(RAND_MAX+1.0));  // Initial delay before 
                                                     // requesting units. 
	hold = 1+(int) (100000.0*rand()/(RAND_MAX+1.0));   // Time the units are held. 
	numunits = 1+(int) ((float)NUMUNITS*rand()/(RAND_MAX+1.0));   // Number of units. 

	// An initial, pre-request delay. 
	for (i=0; i<delay; i++){ 
		interrupt->SetLevel(IntOff); 
		interrupt->SetLevel(IntOn); 
	} 

	printf("p%d asking for %d units\n",arg, numunits); 
	RM->Request(numunits); 
	printf("p%d GOT %d units\n",arg, numunits); 
	used += numunits; 

	ASSERT(used <= NUMUNITS);                          // Sanity check! 

	// Process holds the resource units for a while. 
	for (i=0; i<hold; i++){ 
		interrupt->SetLevel(IntOff); 
		interrupt->SetLevel(IntOn); 
	} 
	printf("p%d returning %d units\n",arg, numunits); 
	used -= numunits; 
	RM->Release(numunits); 
} 

void
PCProcess(int arg) 
{
	int delay = 1+(int) (100000.0*rand()/(RAND_MAX+1.0));

	for (int i=0; i<delay; i++){ 
		interrupt->SetLevel(IntOff); 
		interrupt->SetLevel(IntOn); 
	} 

	if ((arg % 2)) {
		while (1) {
			PC->Produce();
			for (int i=0; i<delay; i++){ 
				interrupt->SetLevel(IntOff); 
				interrupt->SetLevel(IntOn); 
			} 
		}
	} else {
		while (1) {
			PC->Consume();
			for (int i=0; i<delay; i++){ 
				interrupt->SetLevel(IntOff); 
				interrupt->SetLevel(IntOn); 
			} 
		}	   
	}
}

void
EProcess(int arg)
{
	if (!arg) {
		E->Start();
	} else {
		int delay = 1+(int) (100000.0*rand()/(RAND_MAX+1.0));

		for (int i=0; i<delay; i++){ 
			interrupt->SetLevel(IntOff); 
			interrupt->SetLevel(IntOn); 
		} 
		
		int toFloor = rand() % 5;
		int atFloor = rand() % 5;

		if (toFloor == atFloor) {
			if (toFloor == 1) {
				toFloor = NUM_FLOORS-1;
			} else {
				toFloor = 1;
			}
		}
		printf("+++++ new passenger: at floor: %d, to floor: %d\n", atFloor, toFloor);
		E->ArrivingGoingFromTo(atFloor, toFloor);
		printf("----- thread %d arrived at floor\n", arg);
	}
}

void
OneVehicle(int pid)
{
	int i, arrival_delay, crossing_time, direction;

	arrival_delay = 1+(int) (100000.0*rand()/(RAND_MAX+1.0));
	crossing_time = 1+(int) (100000.0*rand()/(RAND_MAX+1.0));
	direction = rand() % 2; // Generate zero or one "randomly" 

	//printf("Car %d going in the %d direction. \n", pid, direction); 

	for (i = 0; i < arrival_delay; i++)
	{
		interrupt->SetLevel(IntOff);
		interrupt->SetLevel(IntOn);
	}

	TM->ArriveBridge(direction, pid);
	TM->CrossBridge(direction, pid);

	for (i = 0; i < crossing_time; i++)
	{
		interrupt->SetLevel(IntOff);
		interrupt->SetLevel(IntOn);
	}
	TM->ExitBridge(direction, pid);
}

void 
ThreadTest(long problem) 
{ 
	int i; 

	if (problem == 1) {				//resource manager
		DEBUG('t', "Starting the RESOURCE MANAGER System"); 
		// Instantiate the resource manager. 
		RM = new(std::nothrow) ResManager; 
		// Instantiate the 20 threads. 
		for (i=0; i<20; i++) { 
			sprintf(pname[i], "p%d", i); 
			t[i] = new(std::nothrow) Thread(pname[i]); 
			t[i]->Fork(RMProcess,i); 
		}
	} else if (problem == 2) {		//producer consumer
		DEBUG('t', "Starting the PRODUCER CONSUMER System");
		PC = new(std::nothrow) ProdCon;
		for (i=0; i<2; i++) {
			sprintf(pname[i], "p%d", i); 
			t[i] = new(std::nothrow) Thread(pname[i]);
			t[i]->Fork(PCProcess, i);
		}
	} else if (problem == 3) {		//elevator
		DEBUG('t', "Starting the ELEVATOR System");
		E = new(std::nothrow) Elevator;
		for (i=0; i<20; i++) {
			sprintf(pname[i], "p%d", i); 
			t[i] = new(std::nothrow) Thread(pname[i]);
			t[i]->Fork(EProcess, i);
		}
	} else {						//bridge
		DEBUG('t', "Starting the TRAFFIC MANAGER System");
		TM = new(std::nothrow) TrafficManager;
		for (i=0; i<20; i++) {
			sprintf(pname[i], "p%d", i); 
			t[i] = new(std::nothrow) Thread(pname[i]);
			t[i]->Fork(OneVehicle, i);
		}
	}
	// Having spawned 20 threads, the initial thread terminates. 
} 

#endif 

