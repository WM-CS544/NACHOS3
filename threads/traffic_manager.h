#ifndef TRAFFIC_MANAGER_H
#define TRAFFIC_MANAGER_H

#include "copyright.h" 
#include "system.h" 
#include "synch.h" 
#include <stdio.h> 
#include <stdlib.h> 
#include "resmanager.h" 
#include <new>

class TrafficManager {

	public:
		TrafficManager();
		~TrafficManager();
		void ArriveBridge(int direc, int pid);
		void ExitBridge(int direc, int pid);
		void CrossBridge(int direc, int pid);

	private:
		Lock *lock;
		Condition *condition;
		int number_of_cars_on_bridge;
		int current_direction;

};

#endif
