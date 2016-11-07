#ifdef CHANGED

#include "fdset.h"

FDSet::FDSet(FDSet *copySet)
{

	//lock = new(std::nothrow) Lock("fdset lock");

	//set up fdArray
	if (copySet == NULL) {	//new set

		FDSetEntry *consoleIn = new(std::nothrow) FDSetEntry;
		consoleIn->numOpen = 1;
		consoleIn->file = (OpenFile *)1;
		FDSetEntry *consoleOut = new(std::nothrow) FDSetEntry;
		consoleOut->numOpen = 1;
		consoleOut->file = (OpenFile *)2;

		for (std::size_t i=0; i < (sizeof(fdArray)/sizeof(fdArray[0])); i++) {
			if (i == 0) {
				fdArray[i] = consoleIn;
			} else if (i == 1) {
				fdArray[i] = consoleOut;
			} else {
				fdArray[i] = NULL;
			}
		}

	} else {	//inherited fdArray
		FDSetEntry **copyArray = copySet->GetArray();
		for (std::size_t i=0; i < (sizeof(fdArray)/sizeof(fdArray[0])); i++) {
			if (copyArray[i] != NULL) {
				copyArray[i]->numOpen++;
			}
			fdArray[i] = copyArray[i];
		}
	}
}

FDSet::~FDSet()
{
	for (std::size_t i=0; i < (sizeof(fdArray)/sizeof(fdArray[0])); i++) {
		if (fdArray[i] != NULL) {
			if (fdArray[i]->numOpen == 1) { //only delete if only one open
				delete fdArray[i]->file;
				delete fdArray[i];
			} else {
				fdArray[i]->numOpen--;	//decrement number open
			}
		}
	}	
}

FDSetEntry**
FDSet::GetArray()
{
	return fdArray;
}

int
FDSet::AddFD(OpenFile *file)
{
	fdSetLock->Acquire();

	int fd = -1;
	for (int i=0; i < (int)(sizeof(fdArray)/sizeof(fdArray[0])); i++) {
		if (fdArray[i] == NULL) {
			FDSetEntry *newEntry = new(std::nothrow) FDSetEntry;
			newEntry->numOpen = 1;
			newEntry->file = file;
			fdArray[i] = newEntry;
			fd = i;
			break;
		}
	}

	fdSetLock->Release();

	return fd;
}

OpenFile*
FDSet::GetFile(int fd)
{
	fdSetLock->Acquire();

	OpenFile *file = NULL;
	if (fd < (int)(sizeof(fdArray)/sizeof(fdArray[0])) && fd >= 0) {
		if (fdArray[fd] != NULL) {
			file = fdArray[fd]->file;
		}
	}

	fdSetLock->Release();

	return file;
}

int
FDSet::NumOpen(int fd)
{
	
	fdSetLock->Acquire();

	if (fd < (int)(sizeof(fdArray)/sizeof(fdArray[0])) && fd >= 0) {
		if (fdArray[fd] != NULL) {
			fdSetLock->Release();
			return fdArray[fd]->numOpen;
		}
	}
	fdSetLock->Release();
	return -1;
}

/*
 * returns:
 *	-1 can't delete fd
 *	0 just decrement numOpen
 *	1 closed file
 */
int
FDSet::DeleteFD(int fd)
{
	fdSetLock->Acquire();

	int retval = 0;
	if (fd < (int)(sizeof(fdArray)/sizeof(fdArray[0])) && fd >= 0) {
			if (fdArray[fd] != NULL) {
				if (fdArray[fd]->numOpen == 1) { //delete file
					//can't delete console openFile
					if ((int)fdArray[fd]->file != 1 && (int)fdArray[fd] != 2) {
						delete fdArray[fd]->file;
						retval = 1;
					}
					delete fdArray[fd];
					fdArray[fd] = NULL;
				} else { //don't delete, decrement numOpen
					ASSERT(fdArray[fd] > 0);
					fdArray[fd]->numOpen--;
					fdArray[fd] = NULL;
				}
				fdSetLock->Release();
				return retval;
			}
	}
	fdSetLock->Release();
	return -1;
}

int
FDSet::Dup(int fd)
{
	fdSetLock->Acquire();

	if (fd < (int)(sizeof(fdArray)/sizeof(fdArray[0])) && fd >= 0) {
		if (fdArray[fd] != NULL) {
			for (unsigned int i=0; i < sizeof(fdArray)/sizeof(fdArray[0]); i++) {
				if (fdArray[i] == NULL) {
					fdArray[fd]->numOpen++; //increase number open
					fdArray[i] = fdArray[fd];	//duplicate
					fdSetLock->Release();
					return i;
				}
			}
		}	
	}

	fdSetLock->Release();
	return -1; //error 
}

#endif
