#ifdef CHANGED

#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include "synch.h"

class ProcessManager {
	
	public:

		ProcessManager();
		~ProcessManager();

		int NewProcess();

	private:

		Lock *lock;
		int numProcesses;
};

#endif	//PROCESS_MANAGER_H
#endif	//CHANGED
