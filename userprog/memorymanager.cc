#ifdef CHANGED

#include "memorymanager.h"
#include <new>

MemoryManager::MemoryManager(int numPages)
{
	memoryMap = new(std::nothrow) BitMap(numPages);	
	diskMap = new(std::nothrow) BitMap(NumSectors);	
	memInfo = new(std::nothrow) memEntry[numPages];
	lock = new(std::nothrow) Lock("memory manager lock");
}

MemoryManager::~MemoryManager()
{
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

	int numClear = diskMap->NumClear();

	lock->Release();

	return numClear;
}

int
MemoryManager::MoveToMem(AddrSpace *curSpace, unsigned int virtpn)
{
	char *fromDisk = new(std::nothrow) char[PageSize];

	lock->Acquire();

	if (memoryMap->NumClear() <= 0) {
		ReplacePage();	
	}

	int physPage = memoryMap->Find();
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

	lock->Release();

	delete fromDisk;

	return 1;
}

int
MemoryManager::ReplacePage()
{

	unsigned int toBeReplaced = FindPageToReplace();
	memEntry curEntry = memInfo[toBeReplaced];
	AddrSpace *curSpace = memInfo[toBeReplaced].addrspace;
	TranslationEntry *curPageTable = curSpace->GetPageTable();

	//if not valid then vpn = dsn
	curPageTable[memInfo[toBeReplaced].vpn].virtualPage = curEntry.dsn;
	curPageTable[memInfo[toBeReplaced].vpn].valid = false;

	//TODO:write to disk
	
	//free from memoryMap
	memoryMap->Clear(toBeReplaced);

	return 1;
}

unsigned int
MemoryManager::FindPageToReplace()
{
	return 1;
}
#endif
