#include "syscall.h"

int cv=-1;
int lock=-1;
int ret=-1;

void main()
{

	cv = CreateCV("cv 1");
	lock = CreateLock("lock1");
	

	AcquireLock(lock);

	WaitCV(0,0);
	Print("\nClient 1 is out of wait..\n",-1,-1,-1);
	ReleaseLock(0);

}
