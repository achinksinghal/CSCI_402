#include "syscall.h"

int num=0;
int lck1=-1;
int lck2=-1;

void main()
{
	lck1 = CreateLock("lock1");
	lck2 = CreateLock("lock2");
	
	
	Print("\n Client 3: Two locks Created..\n", -1, -1, -1);
	
	Print("\n Client 3: This gets those locks since client 2 is dead\n", -1, -1, -1);
	Yield();
	
	AcquireLock("lock1");
	AcquireLock("lock2");
	Print("Client 3: Got both the locks..\n", -1, -1, -1);
	
	Print("Client 3: This has the lock - start client 4...\n", -1, -1, -1);
	for (num=0; num<15000; num++)
	{
		Print("",-1,-1,-1);
		Yield();
	}
		
	Print("Client 3: Terminate this client using Ctrl+C...\n", -1, -1, -1);

	for (num=0; num<32000; num++)
	{
		Print("",-1,-1,-1);
		Yield();
	}

	
	Exit(0);
	
}
