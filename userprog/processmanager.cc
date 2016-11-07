#ifdef CHANGED

#include "processmanager.h"

ProcessManager::ProcessManager()
{
	lock = new(std::nothrow) Lock("process manager lock");
	numProcesses = 1;
}

ProcessManager::~ProcessManager()
{
	delete lock;
}

int
ProcessManager::NewProcess()
{
	lock->Acquire();

	int num = ++numProcesses;
	lock->Release();
	return num;
}

#endif
