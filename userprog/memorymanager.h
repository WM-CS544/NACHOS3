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

//#define RANDOM

struct memEntry {
	AddrSpace *addrspace;
	unsigned int vpn;	//virtual page number
	unsigned int dsn;	//disk sector number
	unsigned int lockbit;
};

struct addrspaceNode {
	AddrSpace *addrspace;
	addrspaceNode *next;
};

struct sharedEntry {
	unsigned int ppn;
	addrspaceNode *head;
	int numShared;
};

class  MemoryManager {
	public:

		MemoryManager(int numPages);
		~MemoryManager();

		int NewPage(AddrSpace *curSpace);
		void ClearPage(int page, TranslationEntry *pageTable, AddrSpace *curSpace);
		int NumPagesFree();
		int MoveToMem(AddrSpace *curSpace, unsigned int virtpn);
		int KernelPage(AddrSpace *curSpace, unsigned int virtpn);
		int UnlockPage(AddrSpace *curSpace, unsigned int virtpn);
		void AddSharedPage(AddrSpace *curSpace, unsigned int dsn);
		void UnSharePages(unsigned int vpn, unsigned int ppn, AddrSpace *curSpace);

	private:

		void AddSharedAddrspace(AddrSpace *curSpace, unsigned int dsn);
		void RemoveSharedAddrspace(AddrSpace *curSpace, unsigned int dsn);
		void SharedPageToMem(unsigned int dsn, int ppn, unsigned int vpn);
		void ReplaceSharedPage(unsigned int dsn, int ppn, unsigned int vpn);
		int GetPageFromDisk(AddrSpace *curSpace, unsigned int virtpn);
		int ReplacePage(int toBeReplaced);
		unsigned int FindPageToReplace();
		void AdjustLRU(int page);
		int FindIndexLRU(int page);
		BitMap *memoryMap;
		BitMap *diskMap;
		memEntry *memInfo;
		Lock *lock;
		int lru[NumPhysPages];
		sharedEntry *sharedList;
};

#endif //MEMORY_MANAGER_H
#endif //CHANGED
