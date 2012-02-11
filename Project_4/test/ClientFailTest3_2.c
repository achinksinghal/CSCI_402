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
	
	Yield();
	Print("\n Client 2: Three locks created..\n", lck1, -1, -1);
	
	Print("\n Client 2: This gets them all  when Client 1 is killed\n", -1, -1, -1);
	AcquireLock("lock1");
	AcquireLock("lock2");
	AcquireLock("lock3");

	Yield();
	
	Print("Client 2: This now exits - Please start client 3...\n", -1, -1, -1);

	Exit(0);


}
