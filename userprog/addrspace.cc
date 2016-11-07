// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"
#include <new>

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical 
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executable)
{
    NoffHeader noffH;
    unsigned int size;
#ifndef USE_TLB
    unsigned int i;
#endif

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && 
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
#ifndef CHANGED
    ASSERT(noffH.noffMagic == NOFFMAGIC);
#else
	//check if script or executable or neither
	char *script = new(std::nothrow) char[7];
	char const *shell= "shell";
	OpenFile *tmp;
	executable->ReadAt(script, 7, 0);
	if (strcmp(script, "#SCRIPT") == 0) {
		tmp = executable;
		executable = fileSystem->Open((char *)shell);
    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && 
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
		//can't open shell
		ASSERT(executable != NULL);
	} else if (noffH.noffMagic != NOFFMAGIC) {
		//not script or noff executable
		ASSERT(0);
	}
#endif

// how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size 
			+ UserStackSize;	// we need to increase the size
						// to leave room for the stack
    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;

#ifndef CHANGED
    ASSERT(numPages <= NumPhysPages);		// check we're not trying
						// to run anything too big --
						// at least until we have
						// virtual memory
#else
		ASSERT(numPages <= (unsigned int) memoryManager->NumPagesFree());
#endif

    DEBUG('a', "Initializing address space, num pages %d, size %d\n", 
					numPages, size);
#ifndef USE_TLB
// first, set up the translation 
    pageTable = new(std::nothrow) TranslationEntry[numPages];
    for (i = 0; i < numPages; i++) {
			pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
#ifndef CHANGED
			pageTable[i].physicalPage = i;
#else
			pageTable[i].physicalPage = memoryManager->NewPage();
			ASSERT(pageTable[i].physicalPage != -1);
#endif
			pageTable[i].valid = true;
			pageTable[i].use = false;
			pageTable[i].dirty = false;
			pageTable[i].readOnly = false;  // if the code segment was entirely on 
							// a separate page, we could set its 
							// pages to be read-only
    }
#endif    

// zero out the entire address space, to zero the unitialized data segment 
// and the stack segment
#ifndef CHANGED
    bzero(machine->mainMemory, size);
#else
		for (i = 0; i < numPages; i++) {
			bzero(&(machine->mainMemory[GetPhysPageNum(i)*PageSize]), PageSize);
		}
#endif

// then, copy in the code and data segments into memory
    if (noffH.code.size > 0) {
        DEBUG('a', "Initializing code segment, at 0x%x, size %d\n", 
			noffH.code.virtualAddr, noffH.code.size);
#ifndef CHANGED
        executable->ReadAt(&(machine->mainMemory[noffH.code.virtualAddr]),
			noffH.code.size, noffH.code.inFileAddr);
#else
			//TODO: Maybe clean this up a little
			for (i=(noffH.code.virtualAddr % PageSize); i < (unsigned int)(noffH.code.size + (noffH.code.virtualAddr % PageSize)); i+=(PageSize - (i % PageSize))) {
				int physAddress = GetPhysAddress((noffH.code.virtualAddr + i) - (noffH.code.virtualAddr % PageSize));
				unsigned int writtenSoFar = i - (noffH.code.virtualAddr % PageSize);
				int location = noffH.code.inFileAddr + writtenSoFar;

				//not at beginning of page
				if (i % PageSize != 0) { //more data than can fit on page
					if ((noffH.code.size - writtenSoFar) > (PageSize - (i % PageSize))) {
						executable->ReadAt(&(machine->mainMemory[physAddress]),
								(PageSize - (i % PageSize)), location);
					} else {	//all data can fit on current page
						executable->ReadAt(&(machine->mainMemory[physAddress]),
								(noffH.code.size - writtenSoFar), location);
					}

				} else { //starting at beginning of page
					if ((noffH.code.size - writtenSoFar) > PageSize) {	//more data than can fit on page
						executable->ReadAt(&(machine->mainMemory[physAddress]),
								PageSize, (noffH.code.inFileAddr + writtenSoFar));
					} else {	//all data can fit on current page
						executable->ReadAt(&(machine->mainMemory[physAddress]),
								(noffH.code.size - writtenSoFar), location);
					}
				}
			}
#endif
    }
    if (noffH.initData.size > 0) {
        DEBUG('a', "Initializing data segment, at 0x%x, size %d\n", 
			noffH.initData.virtualAddr, noffH.initData.size);
#ifndef CHANGED
        executable->ReadAt(&(machine->mainMemory[noffH.initData.virtualAddr]),
			noffH.initData.size, noffH.initData.inFileAddr);
#else
			//TODO: Maybe clean this up a little
			for (i=(noffH.initData.virtualAddr % PageSize); i < (unsigned int)(noffH.initData.size + (noffH.initData.virtualAddr % PageSize)); i+=(PageSize - (i % PageSize))) {
				int physAddress = GetPhysAddress((noffH.initData.virtualAddr + i) - (noffH.initData.virtualAddr % PageSize));
				unsigned int writtenSoFar = i - (noffH.initData.virtualAddr % PageSize);
				int location = noffH.initData.inFileAddr + writtenSoFar;

				//not at beginning of page
				if (i % PageSize != 0) { //more data than can fit on page
					if ((noffH.initData.size - writtenSoFar) > (PageSize - (i % PageSize))) {
						executable->ReadAt(&(machine->mainMemory[physAddress]),
								(PageSize - (i % PageSize)), location);
					} else {	//all data can fit on current page
						executable->ReadAt(&(machine->mainMemory[physAddress]),
								(noffH.initData.size - writtenSoFar), location);
					}

				} else { //starting at beginning of page
					if ((noffH.initData.size - writtenSoFar) > PageSize) {	//more data than can fit on page
						executable->ReadAt(&(machine->mainMemory[physAddress]),
								PageSize, location);
					} else {	//all data can fit on current page
						executable->ReadAt(&(machine->mainMemory[physAddress]),
								(noffH.initData.size - writtenSoFar), location);
					}
				}
			}
#endif
    }

#ifdef CHANGED
	processControlBlock = new(std::nothrow) ProcessControlBlock(NULL, NULL, 1);
	if (strcmp(script, "#SCRIPT") == 0) {
		processControlBlock->GetFDSet()->DeleteFD(0);
		OpenFile *scriptFile = fileSystem->Open(tmp->GetName());
		processControlBlock->GetFDSet()->AddFD(scriptFile);
		fileManager->OpenFile(tmp->GetName());
	}
#endif

}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
#ifndef USE_TLB
#ifdef CHANGED
	//free memory before deleting
	for (unsigned int i=0;  i < numPages; i++) {
		memoryManager->ClearPage(pageTable[i].physicalPage);
	}
#endif
   delete pageTable;
#endif
#ifdef CHANGED
	delete processControlBlock;
#endif
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table,
//      IF address translation is done with a page table instead
//      of a hardware TLB.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
#ifndef USE_TLB
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
#endif
}

#ifdef CHANGED

AddrSpace::AddrSpace(AddrSpace *parentSpace, int pid)
{
	numPages = parentSpace->GetNumPages();

	ASSERT(numPages <= (unsigned int)memoryManager->NumPagesFree());		// check we're not trying
					// to run anything too big --
					// at least until we have
					// virtual memory
#ifndef USE_TLB
// first, set up the translation 
  pageTable = new(std::nothrow) TranslationEntry[numPages];
  for (unsigned int i = 0; i < numPages; i++) {
		pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
		pageTable[i].physicalPage = memoryManager->NewPage();
		pageTable[i].valid = true;
		pageTable[i].use = false;
		pageTable[i].dirty = false;
		pageTable[i].readOnly = false;  // if the code segment was entirely on 
																		// a separate page, we could set its 
																		// pages to be read-only
		ASSERT(pageTable[i].physicalPage != -1);
    }
#endif    

	//copy pages to new addrspace
	for (unsigned int i=0; i < numPages; i++) {
		for (unsigned int offset=0; offset < PageSize; offset++) {
			machine->mainMemory[(pageTable[i].physicalPage * PageSize) + offset] = machine->mainMemory[(parentSpace->GetPhysPageNum(i) * PageSize) + offset];
		}
	}

	processControlBlock = new(std::nothrow) ProcessControlBlock(parentSpace->GetProcessControlBlock(), parentSpace->GetProcessControlBlock()->GetFDSet(), pid);
}

void
AddrSpace::Exec(OpenFile *executable) {
	
	//free memory
	for (unsigned int i = 0; i < numPages; i++) { 
		memoryManager->ClearPage(pageTable[i].physicalPage);
	}
	delete pageTable;

	//reload addrspace
	NoffHeader noffH;
	unsigned int size;
#ifndef USE_TLB
	unsigned int i;
#endif

	executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
	if ((noffH.noffMagic != NOFFMAGIC) && 
	(WordToHost(noffH.noffMagic) == NOFFMAGIC))
		SwapHeader(&noffH);
	ASSERT(noffH.noffMagic == NOFFMAGIC);

// how big is address space?
	size = noffH.code.size + noffH.initData.size + noffH.uninitData.size 
		+ UserStackSize;	// we need to increase the size
					// to leave room for the stack
	numPages = divRoundUp(size, PageSize);
	size = numPages * PageSize;

	//Make sure not more than we have room for
	ASSERT(numPages <= (unsigned int) memoryManager->NumPagesFree());

	DEBUG('a', "Initializing address space, num pages %d, size %d\n", 
				numPages, size);
#ifndef USE_TLB
// first, set up the translation 
	pageTable = new(std::nothrow) TranslationEntry[numPages];
	for (i = 0; i < numPages; i++) {
		pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
		pageTable[i].physicalPage = memoryManager->NewPage();
		pageTable[i].valid = true;
		pageTable[i].use = false;
		pageTable[i].dirty = false;
		pageTable[i].readOnly = false;  // if the code segment was entirely on 
						// a separate page, we could set its 
						// pages to be read-only
		ASSERT(pageTable[i].physicalPage != -1);
	}
#endif    

// zero out the entire address space, to zero the unitialized data segment 
// and the stack segment
	for (i = 0; i < numPages; i++) {
		bzero(&(machine->mainMemory[GetPhysPageNum(i)*PageSize]), PageSize);
	}

// then, copy in the code and data segments into memory
	if (noffH.code.size > 0) {
			DEBUG('a', "Initializing code segment, at 0x%x, size %d\n", 
		noffH.code.virtualAddr, noffH.code.size);

		//TODO: Maybe clean this up a little
		for (i=(noffH.code.virtualAddr % PageSize); i < (unsigned int)(noffH.code.size + (noffH.code.virtualAddr % PageSize)); i+=(PageSize - (i % PageSize))) {
			int physAddress = GetPhysAddress((noffH.code.virtualAddr + i) - (noffH.code.virtualAddr % PageSize));
			unsigned int writtenSoFar = i - (noffH.code.virtualAddr % PageSize);
			int location = noffH.code.inFileAddr + writtenSoFar;

			//not at beginning of page
			if (i % PageSize != 0) { //more data than can fit on page
				if ((noffH.code.size - writtenSoFar) > (PageSize - (i % PageSize))) {
					executable->ReadAt(&(machine->mainMemory[physAddress]),
							(PageSize - (i % PageSize)), location);
				} else {	//all data can fit on current page
					executable->ReadAt(&(machine->mainMemory[physAddress]),
							(noffH.code.size - writtenSoFar), location);
				}

			} else { //starting at beginning of page
				if ((noffH.code.size - writtenSoFar) > PageSize) {	//more data than can fit on page
					executable->ReadAt(&(machine->mainMemory[physAddress]),
							PageSize, (noffH.code.inFileAddr + writtenSoFar));
				} else {	//all data can fit on current page
					executable->ReadAt(&(machine->mainMemory[physAddress]),
							(noffH.code.size - writtenSoFar), location);
				}
			}
		}
	}
	if (noffH.initData.size > 0) {
			DEBUG('a', "Initializing data segment, at 0x%x, size %d\n", 
		noffH.initData.virtualAddr, noffH.initData.size);

		//TODO: Maybe clean this up a little
		for (i=(noffH.initData.virtualAddr % PageSize); i < (unsigned int)(noffH.initData.size + (noffH.initData.virtualAddr % PageSize)); i+=(PageSize - (i % PageSize))) {
			int physAddress = GetPhysAddress((noffH.initData.virtualAddr + i) - (noffH.initData.virtualAddr % PageSize));
			unsigned int writtenSoFar = i - (noffH.initData.virtualAddr % PageSize);
			int location = noffH.initData.inFileAddr + writtenSoFar;

			//not at beginning of page
			if (i % PageSize != 0) { //more data than can fit on page
				if ((noffH.initData.size - writtenSoFar) > (PageSize - (i % PageSize))) {
					executable->ReadAt(&(machine->mainMemory[physAddress]),
							(PageSize - (i % PageSize)), location);
				} else {	//all data can fit on current page
					executable->ReadAt(&(machine->mainMemory[physAddress]),
							(noffH.initData.size - writtenSoFar), location);
				}

			} else { //starting at beginning of page
				if ((noffH.initData.size - writtenSoFar) > PageSize) {	//more data than can fit on page
					executable->ReadAt(&(machine->mainMemory[physAddress]),
							PageSize, location);
				} else {	//all data can fit on current page
					executable->ReadAt(&(machine->mainMemory[physAddress]),
							(noffH.initData.size - writtenSoFar), location);
				}
			}
		}
	}

}

int
AddrSpace::ReadByte(unsigned int va, char *ch)
{
	int pa = GetPhysAddress(va);
	if (pa != -1) {
		*ch = machine->mainMemory[pa];
		return 1;
	} else {
		return 0;
	}
}

int
AddrSpace::WriteByte(unsigned int va, char byte)
{
	int pa = GetPhysAddress(va);
	if (pa != -1) {
		machine->mainMemory[pa] = byte;
		return 1;
	} else {
		return 0;
	}
}

ProcessControlBlock*
AddrSpace::GetProcessControlBlock()
{
	return processControlBlock;
}

int
AddrSpace::GetPhysPageNum(unsigned int virtPageNum)
{
	if (virtPageNum < numPages) {
		return pageTable[virtPageNum].physicalPage;
	} else {
		//error if page out of range
		return -1;
	}
}

int AddrSpace::GetPhysAddress(unsigned int va)
{
	int virtPageNum = va / PageSize;
	int offset = va % PageSize;
	int physPageNum = GetPhysPageNum(virtPageNum);

	//Not a valid page
	if (physPageNum == -1)  {
		return -1;
	}

	return (physPageNum * PageSize) + offset;
}
#endif
