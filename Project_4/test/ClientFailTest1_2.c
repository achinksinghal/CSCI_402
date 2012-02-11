#include "syscall.h"

int lck=-1;

void main()
{
	lck = CreateLock("lock1");

	Print("\n Client 2: Created lock_%d\n", lck, -1, -1);
	
	Print("\n Client 2: Will now wait for lock_%d\n", lck, -1, -1);
	Yield();
	
	AcquireLock("lock1");
	Print("Client 2: Got the lock..\n", -1, -1, -1);
	
	ReleaseLock("lock1");
	Print("Client 2: Lock released, client 2 exits..\n", -1, -1, -1);
	
	Exit(0);

	
}
