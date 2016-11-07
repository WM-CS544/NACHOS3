#ifdef CHANGED

#include "synchconsole.h"

//----------------------------------------------------------------------
// 	Need this to be a C routine, because 
//	C++ can't handle pointers to member functions.
//----------------------------------------------------------------------

static void
staticReadAvail(int arg)
{
	SynchConsole *synchConsole = (SynchConsole *)arg;
	synchConsole->ReadAvail();
}

static void
staticWriteDone(int arg)
{
	SynchConsole *synchConsole = (SynchConsole *)arg;
	synchConsole->WriteDone();
}

SynchConsole::SynchConsole(char *in, char *out)
{
	console = new(std::nothrow) Console(in, out, staticReadAvail, staticWriteDone, int(this));
	readAvail = new(std::nothrow) Semaphore("read avail", 0);
	writeDone = new(std::nothrow) Semaphore("write done", 0);
	lock = new(std::nothrow) Lock("console lock");
}

SynchConsole::~SynchConsole()
{
	delete console;
	delete readAvail;
	delete writeDone;
	delete lock;
}

char
SynchConsole::GetChar()
{
	char ch;

	lock->Acquire();			//get lock

	readAvail->P();						//wait for something to read
	ch = console->GetChar();	//read char

	lock->Release();			//release lock

	return ch;
}

int
SynchConsole::Read(char *buffer, int size)
{
	
	for (int i=0; i < size; i++) {
		buffer[i] = GetChar();
	}	

	return size;
}

void
SynchConsole::PutChar(char ch)
{
	lock->Acquire();	//get lock

	console->PutChar(ch);	//write to console
	writeDone->P();				//wait for write to finish

	lock->Release();	//release lock
}

void
SynchConsole::Write(char *buffer, int size)
{
	lock->Acquire();
	
	for (int i=0; i < size; i++) {
		console->PutChar(buffer[i]);
		writeDone->P();
	}

	lock->Release();
}

#endif
