#include "syscall.h"

int lock=0;

void main()
{

	lock = CreateLock("lock2");
	
	AcquireLock(lock);
	
	Print("Client 2 has the lock..",-1,-1,-1);
	ReleaseLock(lock);
	

}
