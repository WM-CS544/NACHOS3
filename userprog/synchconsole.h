#ifdef CHANGED

#ifndef SYNCH_CONSOLE_H
#define SYNCH_CONSOLE_H

#include "copyright.h"
#include "console.h"
#include "synch.h"

class SynchConsole {
	
	public:
		SynchConsole(char *in, char *out);
		~SynchConsole();

		void PutChar(char ch);
		void Write(char *buffer, int size);
		char GetChar();
		int Read(char *buffer, int size);
		inline void ReadAvail(){ readAvail->V(); };
		inline void WriteDone(){ writeDone->V(); };

	private:
		Console *console;
		Semaphore *readAvail;
		Semaphore *writeDone;
		Lock *lock;
};

#endif
#endif
