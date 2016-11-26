#ifdef CHANGED

#include "memorymanager.h"
#include <new>
#include <math.h>

MemoryManager::MemoryManager(int numPages)
{
	memoryMap = new(std::nothrow) BitMap(numPages);	
	diskMap = new(std::nothrow) BitMap(NumSectors);	
	memInfo = new(std::nothrow) memEntry[numPages];
	lock = new(std::nothrow) Lock("memory manager lock");
	for (int i=0; i<numPages; i++) {
		lru[i] = i;
	}
}

MemoryManager::~MemoryManager()
{
	delete diskMap;
	delete memoryMap;
	delete lock;
	delete [] memInfo;
}

//TODO:Check if find returns -1
int
MemoryManager::NewPage()
{
	lock->Acquire();

	int pageNum = diskMap->Find();

	lock->Release();

	return pageNum;
}

//TODO: Change this to clear based on pagetable
void
MemoryManager::ClearPage(int page, TranslationEntry *pageTable)
{
	lock->Acquire();

	//page is in physMem
	if (pageTable[page].valid) {
		ReplacePage(pageTable[page].physicalPage);
	}

	//page should not be empty if clearing it
	ASSERT(diskMap->Test(pageTable[page].physicalPage));
	diskMap->Clear(page);

	lock->Release();
}

int
MemoryManager::NumPagesFree()
{
	lock->Acquire();

	int numClear = diskMap->NumClear();

	lock->Release();

	return numClear;
}

int
MemoryManager::MoveToMem(AddrSpace *curSpace, unsigned int virtpn)
{
	lock->Acquire();

	int replacedPage = GetPageFromDisk(curSpace, virtpn);

	lock->Release();

	return replacedPage;
}

int
MemoryManager::KernelPage(AddrSpace *curSpace, unsigned int virtpn)
{
	lock->Acquire();
	
	int lockPage;
	int retval = 0;
	//page not already in memory
	if (!curSpace->GetPageTable()[virtpn].valid) {
			lockPage = GetPageFromDisk(curSpace, virtpn);
			retval = 1;	//return one if faulted on page
	} else {
			lockPage = curSpace->GetPageTable()[virtpn].physicalPage;
#ifndef RANDOM
			AdjustLRU(lockPage);
#endif
	}

	//lock the page
	memInfo[lockPage].lockbit = 1;

	lock->Release();

	return retval;
}

int
MemoryManager::UnlockPage(AddrSpace *curSpace, unsigned int virtpn)
{
	lock->Acquire();

	//should be valid page if we are unlocking
	ASSERT(curSpace->GetPageTable()[virtpn].valid);

	memInfo[curSpace->GetPageTable()[virtpn].physicalPage].lockbit = 0;

	lock->Release();

	return 1;
}

int
MemoryManager::GetPageFromDisk(AddrSpace *curSpace, unsigned int virtpn)
{
	char *fromDisk = new(std::nothrow) char[PageSize];
#ifndef RANDOM
	int replace = 0;
#endif

	if (memoryMap->NumClear() <= 0) {
		ReplacePage(FindPageToReplace());	
#ifndef RANDOM
		replace = 1;
#endif
	} 

	int physPage = memoryMap->Find();
#ifndef RANDOM
	if (!replace) {
		AdjustLRU(physPage);
	}
#endif
	//if not valid then ppn = dsn
	unsigned int disksn = curSpace->GetPageTable()[virtpn].physicalPage;
	//read from disk and copy to physical memory
	synchDisk->ReadSector(disksn, fromDisk);
	memcpy(&(machine->mainMemory[physPage*PageSize]), fromDisk, PageSize);
	//change entry in page table to be valid and replace with real physical page
	curSpace->GetPageTable()[virtpn].physicalPage = physPage;
	curSpace->GetPageTable()[virtpn].valid = true;
	//Fill in page info
	memInfo[physPage].addrspace = curSpace;
	memInfo[physPage].vpn = virtpn;
	memInfo[physPage].dsn = disksn;
	memInfo[physPage].lockbit = 0;

	delete fromDisk;

	return physPage;
}

int
MemoryManager::ReplacePage(int toBeReplaced)
{
	memEntry curEntry = memInfo[toBeReplaced];
	AddrSpace *curSpace = memInfo[toBeReplaced].addrspace;
	TranslationEntry *curPageTable = curSpace->GetPageTable();

	//if not valid then ppn = dsn
	curPageTable[curEntry.vpn].physicalPage = curEntry.dsn;
	curPageTable[curEntry.vpn].valid = false;

	synchDisk->WriteSector(curEntry.dsn, &(machine->mainMemory[toBeReplaced*PageSize]));
	char *fromDisk = new(std::nothrow) char[PageSize];
	synchDisk->ReadSector(curEntry.dsn, fromDisk);
	
	//free from memoryMap
	//page should not be empty if clearing it
	ASSERT(memoryMap->Test(toBeReplaced));
	memoryMap->Clear(toBeReplaced);

	return 1;
}

unsigned int
MemoryManager::FindPageToReplace()
{
#ifdef RANDOM
	int page = rand()%NumPhysPages;
	while (memInfo[page].lockbit) {
		page = rand()%NumPhysPages;
	}
	return page;
#else
	int page = lru[0];
	while (memInfo[page].lockbit) {
		AdjustLRU(page);
		page = lru[0];
	}
	AdjustLRU(page);
	return page;
#endif
}

void
MemoryManager::AdjustLRU(int page)
{
	int index = FindIndexLRU(page);
	int tmp = lru[index];

	for (int i=index; i<(NumPhysPages-1); i++) {
		lru[i] = lru[i+1];
	}
	lru[NumPhysPages-1] = tmp;
}

int
MemoryManager::FindIndexLRU(int page)
{
	for (int i=0; i<NumPhysPages; i++) {
		if (lru[i] == page) {
			return i;
		}
	}
	//page not in lru, shouldn't happen
	ASSERT(0);
	return -1;
}
#endif
