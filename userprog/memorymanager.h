#ifdef CHANGED
#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include "copyright.h"
#include "synch.h"
#include "bitmap.h"
#include "addrspace.h"
#include "synchdisk.h"
#include "machine.h"
#include "system.h"

struct memEntry {
	AddrSpace *addrspace;
	unsigned int vpn;	//virtual page number
	unsigned int dsn;	//disk sector number
	unsigned int lockbit;
};

class  MemoryManager {
	public:

		MemoryManager(int numPages);
		~MemoryManager();

		int NewPage();
		void ClearPage(int page, TranslationEntry *pageTable);
		int NumPagesFree();
		int MoveToMem(AddrSpace *curSpace, unsigned int virtpn);
		int KernelPage(AddrSpace *curSpace, unsigned int virtpn);
		int UnlockPage(AddrSpace *curSpace, unsigned int virtpn);

	private:

		int GetPageFromDisk(AddrSpace *curSpace, unsigned int virtpn);
		int ReplacePage(int toBeReplaced);
		unsigned int FindPageToReplace();
		BitMap *memoryMap;
		BitMap *diskMap;
		memEntry *memInfo;
		Lock *lock;
};

#endif //MEMORY_MANAGER_H
#endif //CHANGED
