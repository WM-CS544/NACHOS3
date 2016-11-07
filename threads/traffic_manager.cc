#include "traffic_manager.h"

/* 
* Constructor method:
*  Initialize private members and instantiate the Lock and Condition variables
*/
TrafficManager::TrafficManager()
{
	number_of_cars_on_bridge = 0;
	current_direction = 1; // 1 was arbitrarily chosen as the starting direction
	   // This could change depending on the direction of the first car to arrive at the bridge
	lock = new(std::nothrow) Lock("TrafficManager Lock"); 
	condition = new(std::nothrow) Condition("TrafficManager Condition");
}

// Destructor method: simply free previously allocated memory
TrafficManager::~TrafficManager()
{
	delete lock;
	delete condition;
}

// Must not return until it is safe for the car to cross the bridge in the given direction. 
// Must guarantee that there will be no head-on collisions or bridge collapses.  
void TrafficManager::ArriveBridge(int direc, int pid)
{
	lock->Acquire(); // Before anything can happen the process must aquire the lock. 
	while (!((number_of_cars_on_bridge == 0) || ((number_of_cars_on_bridge < 3) && (current_direction == direc))))
		//while (!((current_direction == direc) && (number_of_cars_on_bridge < 3)))
	{
		// A car can't begin crossing the bridge if:
		// 1) the current direction of traffic is different from that car's direction
		// 2) there are 3 or more cars already on the bridge.  
		// Therefore, that specific car has to wait on the lock. 
		printf("Car %d waiting on bridge in the %d direction. \n", pid, direc);
		condition->Wait(lock);
	}
	// If the program has reached this point, the car has began crossing the bridge, so
	// we increment the number of cars on the bridge by 1. 
	number_of_cars_on_bridge++;
	current_direction = direc; // Current direction is also set to this specific car's direction.  
	   // This will only actually change the value for the first car crossing
	   // after the direction has last been switched, but it doesn't hurt to 
	   // reassign it just in anyway.  
	lock->Release(); // And finally we release the lock for another process to grab. 
}

// Called to indicate that the caller has finished crossing the bridge.  
// Should take steps to let additional cars cross the bridge. 
void TrafficManager::ExitBridge(int direc, int pid)
{
	lock->Acquire(); // Must acquire the lock before anything can happen.  
	printf("Car %d exiting bridge in the %d direction. \n", pid, direc);
	number_of_cars_on_bridge--; // Decrement the number of cars on the bridge by 1. 
	condition->Broadcast(lock); // We use broadcast to let all other cars have a shot at crossing next. 
	lock->Release(); // Always release the lock so another car can do something.  
}

// Only needs to print out a debug message
void TrafficManager::CrossBridge(int direc, int pid)
{
	printf("Car %d crossing bridge in %d direction. \n", pid, direc);
	ASSERT(number_of_cars_on_bridge <= 3); // Is this fails it means the bridge has collapsed. 
}
