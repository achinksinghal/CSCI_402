#include "syscall.h"

int cv=-1;
int lock=-1;
int ret=-1;

void main()
{

	cv = CreateCV("cv_1");
	lock = CreateLock("lock1");
	

	AcquireLock("lock1");

	WaitCV("lock1","cv_1");
	Print("\nClient 1 is out of wait..\n",-1,-1,-1);
	ReleaseLock("lock1");

	Exit(0);
}
