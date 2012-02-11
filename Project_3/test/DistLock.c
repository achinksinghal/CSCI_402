#include "syscall.h"

void main()
{
	AcquireLock(0);
	Print("\nLock acquired by DistLock", -1 ,-1 , -1);
	ReleaseLock(0);
}
