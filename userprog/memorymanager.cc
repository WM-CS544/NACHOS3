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
	sharedList = new(std::nothrow) sharedEntry[NumSectors];
	for (int j=0; j<NumSectors; j++) {
		sharedList[j].head = NULL;
	}
}

MemoryManager::~MemoryManager()
{
	delete diskMap;
	delete memoryMap;
	delete lock;
	delete [] memInfo;
	delete [] sharedList;
}

//TODO:Check if find returns -1
int
MemoryManager::NewPage(AddrSpace *curSpace)
{
	lock->Acquire();

	int pageNum = diskMap->Find();
	AddSharedAddrspace(curSpace, pageNum);

	lock->Release();

	return pageNum;
}

//TODO: Change this to clear based on pagetable
void
MemoryManager::ClearPage(int page, TranslationEntry *pageTable, AddrSpace *curSpace)
{
	lock->Acquire();

	//page is in physMem
	if (pageTable[page].valid) {
		//ReplacePage(pageTable[page].physicalPage);
	}

	//remove shared addrspace and clear disk if needed
	RemoveSharedAddrspace(curSpace, pageTable[page].physicalPage);

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

void
MemoryManager::AddSharedPage(AddrSpace *curSpace, unsigned int dsn)
{
	lock->Acquire();

	AddSharedAddrspace(curSpace, dsn);

	lock->Release();
}

void
MemoryManager::UnSharePages(unsigned int vpn, unsigned int ppn, AddrSpace *curSpace)
{
	//lock creates situation where requested readonly page is now not in mem should fix and readd lock
	//lock->Acquire();

	addrspaceNode *curNode = sharedList[memInfo[ppn].dsn].head;

	while (curNode != NULL) {
		if (curNode->addrspace != curSpace) {
			int newPage = diskMap->Find();
			AddSharedAddrspace(curNode->addrspace, newPage);
			curNode->addrspace->GetPageTable()[vpn].physicalPage = newPage;
			curNode->addrspace->GetPageTable()[vpn].valid = false;
			synchDisk->WriteSector(newPage, &(machine->mainMemory[ppn*PageSize]));
		} else {
			sharedList[memInfo[ppn].dsn].head = curNode;
		}
		curNode->addrspace->GetPageTable()[vpn].readOnly = false;
		curNode = curNode->next;
	}

	//lock->Release();
}

void
MemoryManager::AddSharedAddrspace(AddrSpace *curSpace, unsigned int dsn)
{
	//increment counter
	sharedList[dsn].numShared ++;

	//add new addspace to list
	addrspaceNode *newNode = new(std::nothrow) addrspaceNode;
	newNode->addrspace = curSpace;
	newNode->next = NULL;

	addrspaceNode *curNode = sharedList[dsn].head;
	addrspaceNode *prevNode = NULL;

	while (curNode != NULL) {
		prevNode = curNode;
		curNode = curNode->next;
	}
	//start of list
	if (prevNode == NULL) {
		sharedList[dsn].head = newNode;
	} else {
		prevNode->next = newNode;
	}
}

void
MemoryManager::RemoveSharedAddrspace(AddrSpace *curSpace, unsigned int dsn)
{
	//if only page using clear from map
	if (sharedList[dsn].numShared == 1) {
		//page should not be empty if clearing it
		ASSERT(diskMap->Test(dsn));
		diskMap->Clear(dsn);
	}

	//decrement counter
	sharedList[dsn].numShared --;

	addrspaceNode *curNode = sharedList[dsn].head;
	addrspaceNode *prevNode = NULL;

	while (curNode != NULL) {
		if (curNode->addrspace == curSpace) {
			//start of list
			if (prevNode == NULL) {
				sharedList[dsn].head = curNode->next;
			} else {
				prevNode->next = curNode->next;
			}
			return;
		}
		prevNode = curNode;
		curNode = curNode->next;	
	}
	//if removing addrspace it should be in list
	//ASSERT(0);
}

void
MemoryManager::SharedPageToMem(unsigned int dsn, int ppn, unsigned int vpn)
{
	sharedList[dsn].ppn = ppn;
	//validate all pagetables
	addrspaceNode *curNode = sharedList[dsn].head;
	while(curNode != NULL) {
		curNode->addrspace->GetPageTable()[vpn].physicalPage = ppn;
		curNode->addrspace->GetPageTable()[vpn].valid = 1;
		curNode = curNode->next;
	}
}

void
MemoryManager::ReplaceSharedPage(unsigned int dsn, int ppn, unsigned int vpn)
{
	sharedList[dsn].ppn = -1;
	//invalidate all pagetables
	addrspaceNode *curNode = sharedList[dsn].head;
	while(curNode != NULL) {
		curNode->addrspace->GetPageTable()[vpn].physicalPage = dsn;
		curNode->addrspace->GetPageTable()[vpn].valid = false;
		curNode = curNode->next;
	}
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

	//update sharedPageList
	SharedPageToMem(disksn, physPage, virtpn);

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

	//invalidate for all shared pages
	ReplaceSharedPage(curEntry.dsn, toBeReplaced, curEntry.vpn);

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
