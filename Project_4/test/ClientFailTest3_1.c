#include "syscall.h"

int num = 0;
int lck1=-1;
int lck2=-1;
int lck3=-1;

void main()
{
	lck1 = CreateLock("lock1");
	lck2 = CreateLock("lock2");
	lck3 = CreateLock("lock3");

	Print("\n Client 1: Three Locks created\n", lck1, -1, -1);
	
	AcquireLock("lock1");
	AcquireLock("lock2");
	AcquireLock("lock3");
	
	Print("Client 1: This has acquired them all - start client 2...\n", -1, -1, -1);
	for (num=0; num<15000; num++)
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
