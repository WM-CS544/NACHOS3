#ifdef CHANGED
#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include "copyright.h"
#include "synch.h"
#include "bitmap.h"

class  MemoryManager {
	public:

		MemoryManager(int numPages);
		~MemoryManager();

		int NewPage();
		void ClearPage(int page);
		int NumPagesFree();

	private:

		BitMap *memoryMap;
		Lock *lock;
};

#endif //MEMORY_MANAGER_H
#endif //CHANGED
