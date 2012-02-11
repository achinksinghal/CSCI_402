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

	DestroyCV(cv);
	
	AcquireLock(lock);
	
	WaitCV(lock,cv);
	Print("This prints as CV has been destroyed so no wait occurs..",-1,-1,-1);
	ReleaseLock(lock);


}
