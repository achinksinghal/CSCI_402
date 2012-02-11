#include "syscall.h"

int num = 0;
int lck=-1;


void main()
{
	lck = CreateLock("lock1");
	Yield();
	Print("\n Client 1: Created Lock is %d\n", lck, -1, -1);
	
	AcquireLock("lock1");
	Yield();
	
	Print("Client 1: This will now exit.. Please start client 2...\n", -1, -1, -1);

	Exit(0);


}
