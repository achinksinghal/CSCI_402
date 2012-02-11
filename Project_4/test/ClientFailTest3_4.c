#include "syscall.h"

int lck1=-1;
int lck2=-1;
int cv=-1;
int num=-1;

void main()
{
	lck1 = CreateLock("lock1");
	cv = CreateCV("cv1");
	Yield();
	Print("\n Client 4: Created Lock and cv\n", -1, -1, -1);
	
	Print("\n Client 4: This gets lock when Client 3 is killed\n", -1, -1, -1);
	AcquireLock("lock1");
	Yield();
	
	Print("Client 4: This will go on wait on a cv and 1 recovered lock..\n", -1, -1, -1);
	Print("Client 4: Start client 5 to awaken this..\n", -1, -1, -1);

	WaitCV("lock1","cv1");
	
	for (num=0; num<9;num++)
	{
		Print("\n",-1,-1,-1);
	}
	
	Print("Client 4: This is now out of wait..\n", -1, -1, -1);

	ReleaseLock("lock1");

	Exit(0);

	
}
