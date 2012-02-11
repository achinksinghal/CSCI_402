#include "syscall.h"

int lock;

void main()
{

	lock = CreateLock("lock");

	AcquireLock("lock");
	DestroyLock("lock");
	
	Print("Client 3 has the lock ..\n",-1,-1,-1);
	ReleaseLock("lock");

	Print("Client 3 now attempts to acquire a deleted lock..\n",-1,-1,-1);

	AcquireLock("lock");
	
	Print("Client 3: An exception for bad lock was generated ..",-1,-1,-1);
	ReleaseLock("lock");

	Exit(0);
}
