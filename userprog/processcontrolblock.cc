#ifdef CHANGED

#include "processcontrolblock.h"
#include <new>

ProcessControlBlock::ProcessControlBlock(ProcessControlBlock *parent, FDSet *parentSet, int newPID)
{
	pid = newPID;
	parentBlock = parent;
	childListHead = NULL;
	lock = new(std::nothrow) Lock("pcb lock");
	fdSet = new(std::nothrow) FDSet(parentSet);
}

ProcessControlBlock::~ProcessControlBlock()
{
	delete fdSet;
	delete lock;
	
	ChildNode *curNode = childListHead;
	ChildNode *prevNode = NULL;

	while(curNode != NULL) {
		curNode->child->SetParent(NULL); //tell child we left them
		prevNode = curNode;
		curNode = curNode->next;
		//delete prevNode->child;	//should already be deleted
		delete prevNode->childDone;
		delete prevNode;
	}
}

FDSet*
ProcessControlBlock::GetFDSet()
{
	return fdSet;
}

int
ProcessControlBlock::GetPID()
{
	return pid;
}

void
ProcessControlBlock::SetFDSet(FDSet *parentSet)
{
	//lock->Acquire();

	delete fdSet;
	fdSet = parentSet;

	//lock->Release();
} 

void
ProcessControlBlock::SetParent(ProcessControlBlock *newParent) 
{
	//lock->Acquire();

	parentBlock = newParent;

	//lock->Release();
}

void
ProcessControlBlock::AddChild(ProcessControlBlock *childBlock, int childPID, Semaphore *childSem)
{
	//lock->Acquire();

	ChildNode *newNode = new(std::nothrow) ChildNode;
	newNode->retval = -1;
	newNode->pid = childPID;
	newNode->childDone = childSem;
	newNode->child = childBlock;
	newNode->next = NULL;

	ChildNode *curNode = childListHead;
	ChildNode *prevNode = NULL;

	while (curNode != NULL) {
		prevNode = curNode;
		curNode = curNode->next;
	}
	
	if (prevNode == NULL) { //empty list
		childListHead = newNode;
	} else {
		prevNode->next = newNode;
	}

	//lock->Release();
}

void
ProcessControlBlock::DeleteChild(int childPID)
{
	//lock->Acquire();

	ChildNode *curNode = childListHead;
	ChildNode *prevNode = NULL;

	while (curNode != NULL) {
		if (curNode->pid == childPID) {
			if (prevNode == NULL) {	//head of list
				childListHead = curNode->next;
			} else {
				prevNode->next = curNode->next;
			}
			//delete curNode->child;	//should already be deleted
			delete curNode->childDone;
			delete curNode;
			break;
		}
		prevNode = curNode;
		curNode = curNode->next;
	}

	//lock->Release();
}

ChildNode*
ProcessControlBlock::GetChild(int childPID)
{
	//lock->Acquire();

	ChildNode *curNode = childListHead;

	while (curNode != NULL) {
		if (curNode->pid == childPID) {
			//lock->Release();
			return curNode;
		}
		curNode = curNode->next;
	}

	//lock->Release();

	return NULL;
}

#endif
