// addrspace.h 
//	Data structures to keep track of executing user programs 
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#ifdef CHANGED
#include "processcontrolblock.h"
#include "translate.h"
#endif

#define UserStackSize		1024 	// increase this as necessary!

class AddrSpace {
  public:
    AddrSpace(OpenFile *executable);	// Create an address space,
					// initializing it with the program
					// stored in the file "executable"
    ~AddrSpace();			// De-allocate an address space

    void InitRegisters();		// Initialize user-level CPU registers,
					// before jumping to user code

    void SaveState();			// Save/restore address space-specific
    void RestoreState();		// info on a context switch 
#ifdef CHANGED
		AddrSpace(AddrSpace *parentSpace, int pid);
		void Exec(OpenFile *executable);
		void ClearPageTable();
		void Checkpoint(OpenFile *file);

		unsigned int GetNumPages() {return numPages;};
		ProcessControlBlock *GetProcessControlBlock();
		TranslationEntry *GetPageTable() {return pageTable;};

		int ReadByte(unsigned int va, char *ch);
		int WriteByte(unsigned int va, char byte);
		int GetPhysPageNum(unsigned int virtPageNum);
		int GetPhysAddress(unsigned int va);

		int *diskPages;
#endif

  private:
#ifndef USE_TLB
    TranslationEntry *pageTable;	// Assume linear page table translation
#endif					// for now!
    unsigned int numPages;		// Number of pages in the virtual 
					// address space
#ifdef CHANGED
		ProcessControlBlock *processControlBlock;
#endif
};

#endif // ADDRSPACE_H
