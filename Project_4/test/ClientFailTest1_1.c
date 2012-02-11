#include "syscall.h"

int num = 0;
int lck=-1;


void main()
{
	lck = CreateLock("lock1");

	Print("\n Client 1: Created Lock is %d\n", lck, -1, -1);
	
	AcquireLock("lock1");
	
	Print("Client 1: This has the lock - start client 2...\n", -1, -1, -1);
	for (num=0; num<12000; num++)
	{
		Print("",-1,-1,-1);
		Yield();
	}
		
	Print("Client 1: Terminate this client using Ctrl+C...\n", -1, -1, -1);

	for (num=0; num<32000; num++)
	{
		Print("",-1,-1,-1);
		Yield();
	}

	Exit(0);
	
}
