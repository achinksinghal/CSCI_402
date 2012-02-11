#include "syscall.h"

int cv=-1;
int lck=-1;
int num=0;

void main()
{
	
	lck = CreateLock("lock");

	cv = CreateCV("cv");
	
	AcquireLock(lck);

	Print("Will now wake up all sleepers..\n",-1,-1,-1);
	
	BroadcastCV(lck,cv);
	
	ReleaseLock(lck);
	
}
