#include "syscall.h"

int lock=0;

void main()
{

	lock = CreateLock("lock");
	

	AcquireLock("lock");
	
	Print("Client 1 has the lock..",-1,-1,-1);
	ReleaseLock("lock");
	
	Exit(0);
}
