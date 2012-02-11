#include "syscall.h"

int cv=-1;
int lock=-1;
int ret=-1;


int main()
{

	cv = CreateCV("cv 1");
	lock = CreateLock("lock1");

	AcquireLock(lock);
	
	SignalCV(lock,cv);
	ReleaseLock(lock);

	AcquireLock(lock);

	WaitCV(lock,cv);
	Print("\nClient 2 is out of wait..\n",-1,-1,-1);
	ReleaseLock(lock);

}
