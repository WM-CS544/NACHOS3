#ifndef ELEVATOR_H
#define ELEVATOR_H

#include "copyright.h"
#include "synch.h"

#define NUM_FLOORS 5

class Elevator {
	public:
		Elevator();
		~Elevator();

		void Start();											//start the elevator
		void ArrivingGoingFromTo(int atFloor, int toFloor);		//called by passengers arriving

	private:
		//variables
		Lock *lock;												//lock used to implement "monitor"
		int curFloor;											//current floor the elevator is on
		int curDirection;										//direction: 1 = up, -1 = down
		Condition *elevatorWaiting;								//wait when elevator not moving

		Condition *waitingToGoUp[NUM_FLOORS];					//wait if passenger wants to go up
		int numWaitingToGoUp[NUM_FLOORS];						//number of passengers waiting to go up per floor
		int totalWaitingToGoUp;									//total number of passengers waiting to go up

		Condition *waitingToGoDown[NUM_FLOORS];					//wait if passenger wants to go down
		int numWaitingToGoDown[NUM_FLOORS];						//number of passengers waiting to go down per floor
		int totalWaitingToGoDown;								//total number of passengers waiting to go down

		Condition *waitingToLeave[NUM_FLOORS];					//wait if passenger is on the elevator
		int numWaitingToLeave[NUM_FLOORS];						//number of passengers waiting to get off per floor
		int totalWaitingToLeave;								//total number of passengers on the elevator

		//methods
		int CheckInFront();										//check if elevator needs to continue in direction: 1 = yes, 0 = no
		void ChangeDirection();									
		void Move();								
		void EmptyElevator();									//let everyone off at current floor
		void FillElevator();									//let everyone on at current floor
};

#endif
