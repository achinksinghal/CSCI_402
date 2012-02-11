#include "syscall.h"

int cv=-1;
int lck=-1;
int num=0;

void main()
{
	
	lck = CreateLock("lock");

	cv = CreateCV("cv");

	AcquireLock(lck);

	Print("Client 1 to go on wait...\n",-1,-1,-1);
	
	WaitCV(lck,cv);
	
	Print("Client 1 out of wait...\n",-1,-1,-1);


	ReleaseLock(lck);


	
}

