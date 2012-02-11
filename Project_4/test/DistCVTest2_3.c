#include "syscall.h"

int cv=-1;
int lck=-1;
int num=0;

void main()
{
	
	lck = CreateLock("lock");

	cv = CreateCV("cv");
	
	AcquireLock("lock");

	Print("Will now wake up all sleepers..\n",-1,-1,-1);
	
	BroadcastCV("lock","cv");

	ReleaseLock("lock");

	Exit(0);
	
}
