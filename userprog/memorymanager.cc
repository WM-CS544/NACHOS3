#ifdef CHANGED

#include "memorymanager.h"
#include <new>

MemoryManager::MemoryManager(int numPages)
{
	memoryMap = new(std::nothrow) BitMap(numPages);	
	lock = new(std::nothrow) Lock("memory manager lock");
}

MemoryManager::~MemoryManager()
{
	delete memoryMap;
	delete lock;
}

//TODO:Check if find returns -1
int
MemoryManager::NewPage()
{
	lock->Acquire();

	int pageNum = memoryMap->Find();

	lock->Release();

	return pageNum;
}

void
MemoryManager::ClearPage(int page)
{
	lock->Acquire();

	//page should not be empty if clearing it
	ASSERT(memoryMap->Test(page));

	memoryMap->Clear(page);

	lock->Release();
}

int
MemoryManager::NumPagesFree()
{
	lock->Acquire();

	int numClear = memoryMap->NumClear();

	lock->Release();

	return numClear;
}
#endif
