#ifndef PROD_CON_H
#define PROD_CON_H

#include "copyright.h"
#include "synch.h"

#define BUFF_SIZE 20

class ProdCon {
	public:
		ProdCon();
		~ProdCon();

		void Produce();				//put an item in the buffer

		void Consume();				//remove an item from the buffer and print it

	private:
		Lock *lock;					//lock used to implement a "monitor"
		int prodIndex;				//producer's index in the buffer
		int prodStringIndex;		//index in the string used by all producers
		Condition *isFull;			//sleep if  buffer is full
		int conIndex;				//consumer's index in the buffer
		Condition *isEmpty;			//sleep if buffer is emepty
		int numInBuff;				//current number of items in the buffer
		char buff[BUFF_SIZE];		//the buffer
};

#endif

