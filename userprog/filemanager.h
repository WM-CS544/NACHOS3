#ifdef CHANGED
#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "synch.h"

struct FileNode;

class FileManager {

	public:

		FileManager();
		~FileManager();

		void OpenFile(char *fileName);
		void CloseFile(char *fileName);
		Lock *GetLock(char *fileName);

	private:
		
		FileNode *fileListHead;
		Lock *lock;

};

struct FileNode {
	int numOpen;
	char *name;
	Lock *lock;
	FileNode *next;
};


#endif	//FILE_MANAGER_H
#endif	//CHANGED
