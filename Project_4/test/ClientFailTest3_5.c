#include "syscall.h"

int lck=-1;
int cv=-1;

void main()
{
	lck = CreateLock("lock1");
	cv = CreateCV("cv1");
	Yield();
	Print("\n Client 5: Created Lock is %d and cv is %d\n", lck, cv, -1);
	
	AcquireLock("lock1");
	
	Print("\n Client 5: This will now signal and awake the Client 4\n", -1, -1, -1);
	SignalCV("lock1","cv1");
	
	Print("Client 5: This will now exit..\n", -1, -1, -1);

	ReleaseLock("lock1");

	Exit(0);

	
}
