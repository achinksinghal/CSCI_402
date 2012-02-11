#include "syscall.h"

int cv=-1;
int lock=-1;
int ret=-1;


int main()
{

	cv = CreateCV("cv_1");
	lock = CreateLock("lock1");


	AcquireLock("lock1");
	SignalCV("lock1","cv_1");
	ReleaseLock("lock1");

	DestroyCV("cv_1");
	
	AcquireLock("lock1");
	
	WaitCV("lock1","cv_1");
	Print("This prints as CV has been destroyed so no wait occurs..",-1,-1,-1);
	ReleaseLock("lock1");

	Exit(0);
}
