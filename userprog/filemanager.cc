#ifdef CHANGED

#include "filemanager.h"

FileManager::FileManager()
{

	fileListHead = NULL;
	lock = new(std::nothrow) Lock("fileManager lock");

}

FileManager::~FileManager()
{

	FileNode *prevNode= NULL;
	FileNode *curNode = fileListHead;

	while (curNode != NULL) {
		prevNode = curNode;
		curNode = curNode->next;

		delete prevNode->lock;
		delete prevNode->name;
		delete prevNode;	
	}

}

void
FileManager::OpenFile(char *fileName)
{
	lock->Acquire();

	FileNode *prevNode = NULL;
	FileNode *curNode = fileListHead;

	while (curNode != NULL) {
		//if file already opened
		if (strcmp(curNode->name, fileName) == 0) {
			curNode->numOpen++;
			lock->Release();
			return;
		}
		prevNode = curNode;
		curNode = curNode->next;
	}

	//File not already open
	Lock *newLock = new(std::nothrow) Lock("file lock");
	char *newName = new(std::nothrow) char[strlen(fileName)+1];
	strncpy(newName, fileName, strlen(fileName));
	newName[strlen(fileName)] = '\0';

	FileNode *newNode = new(std::nothrow) FileNode;
	newNode->lock = newLock;
	newNode->name = newName;
	newNode->numOpen = 1;
	newNode->next = NULL;

	//head is NULL
	if (prevNode == NULL) {
		fileListHead = newNode;	
	} else {
		prevNode->next = newNode;
	}

	lock->Release();
}

void
FileManager::CloseFile(char *fileName)
{
	lock->Acquire();

	FileNode *prevNode = NULL;
	FileNode *curNode = fileListHead;

	while (curNode != NULL) {
		if (strcmp(curNode->name, fileName) == 0) {
			//file opened by others as well
			if (curNode->numOpen > 1) {
				curNode->numOpen--;
			} else {
				//head of list
				if (prevNode == NULL) {
						fileListHead = curNode->next;
				} else {
					prevNode->next = curNode->next;
				}
				delete curNode->lock;
				delete curNode->name;
				delete curNode;
			}
			lock->Release();
			return;
		}
		prevNode = curNode;
		curNode = curNode->next;
	}

	//trying to close file that isn't in list
	ASSERT(0);

	lock->Release();
}

Lock*
FileManager::GetLock(char *fileName)
{
	lock->Acquire();

	FileNode *curNode = fileListHead;

	while (curNode != NULL) {
		if (strcmp(curNode->name, fileName) == 0) {
			lock->Release();
			return curNode->lock;
		}
		curNode = curNode->next;
	}

	lock->Release();
	return NULL;
}


#endif
