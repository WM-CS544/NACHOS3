// resmanager.cc 
// 
//      Implemented in "monitor"-style -- surround each procedure with a 
//      lock acquire and release pair, using condition signal and wait for 
//      synchronization. 
// 

#include "resmanager.h" 
#include <new>

ResManager::ResManager() 
{ 
    lock = new(std::nothrow) Lock("resmanager lock"); 
    gotsome = new(std::nothrow) Condition("gotsome"); 
    inventory = NUMUNITS; 
} 

ResManager::~ResManager() 
{ 
    delete lock; 
    delete gotsome; 
} 

void 
ResManager::Request(int num) 
{ 
    lock->Acquire();            // entering the "monitor" ... get mutex 
    while (num > inventory) gotsome->Wait(lock);   // Keep trying until 
                                                   // sufficient free units. 
    inventory -= num; 
    lock->Release(); 
} 

void 
ResManager::Release(int num) 
{ 
    lock->Acquire(); 
    inventory += num; 
    gotsome->Broadcast(lock);   // Let ALL sleepers retry. 
    lock->Release(); 
} 

