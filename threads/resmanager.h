// resmanager.h 
//      From Problem Set 3, problem 1. 

#ifndef RMANAGER_H 
#define RMANAGER_H 

#include "copyright.h" 
#include "synch.h" 

#define NUMUNITS 10 

class ResManager { 
  public: 
    ResManager(); 
    ~ResManager(); 

    void Request(int num);      // Process asks for num units. 

    void Release(int num);      // Return num units. 

  private: 
    int inventory;              // Number of free units. 
    Lock *lock; 
    Condition *gotsome; 
}; 

#endif // RMANAGER_H 

