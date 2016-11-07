#include <stdio.h> 
#include "prodcon.h"
#include <new>

const char *PROD_STRING = "Hello World\n";

ProdCon::ProdCon()
{
	lock = new(std::nothrow) Lock("prod_con lock");
	isFull = new(std::nothrow) Condition("prod_con buff full condition");
	isEmpty = new(std::nothrow) Condition("prod_con buff empty condition");
	prodIndex = 0;
	prodStringIndex = 0;
	conIndex = 0;
	numInBuff = 0;
}

ProdCon::~ProdCon()
{
	delete lock;
	delete isFull;
	delete isEmpty;
}

void
ProdCon::Produce()
{
	lock->Acquire();

	while(numInBuff == BUFF_SIZE) {		//wait while buffer is full
		isFull->Wait(lock);
	}

	buff[prodIndex] = PROD_STRING[prodStringIndex];					//add char to buff
	prodIndex  = (prodIndex + 1) % BUFF_SIZE;						//move indexes and count
	prodStringIndex = (prodStringIndex + 1) % strlen(PROD_STRING);
	numInBuff++;												
	ASSERT(numInBuff <= BUFF_SIZE);									//check for overflow
	isEmpty->Signal(lock);											//wake up consumer waiting

	lock->Release();
}

void
ProdCon::Consume()
{
	lock->Acquire();

	while(!numInBuff) {					//wait while buffer is empty
		isEmpty->Wait(lock);
	}
	char consumed = buff[conIndex];									//get data from buff
	printf("%c", consumed);											//print
	conIndex = (conIndex + 1) % BUFF_SIZE;							//move indexes and count
	numInBuff--;	
	ASSERT(numInBuff >= 0);											//check for overconsumption
	isFull->Signal(lock);											//wake up producer waiting

	lock->Release();
}

