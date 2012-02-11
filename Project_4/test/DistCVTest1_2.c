#include "syscall.h"

int cv=-1;
int lck=-1;
int i=0;
void main()
{
	lck = CreateLock("lock1");
	cv = CreateCV("cv1");
	Print("Client 2: Created Lock is %d and CV is %d\n", lck, cv, -1);
	
	AcquireLock("lock1");
	
	
	Print("Client 2: About to signal Client 1\n", -1, -1, -1);

	SignalCV("lock1","cv1");
	
	Print("Client 2: About to go on wait...\n", -1, -1, -1);

	
	WaitCV("lock1","cv1");
	for (i=0; i<10; i++)
		Print("\n",-1,-1,-1);
		
	Print("Client 2: Now out of wait...\n", -1, -1, -1);
	ReleaseLock("lock1");
	
	Exit(0);
}
