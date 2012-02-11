#include "syscall.h"

int lock=0;
int cv1=0;


int main()
{

	lock = CreateLock("lock1");
	
	cv1 = CreateCV("cv 1");

	AcquireLock(lock);


	Print("Client 1 is going to wait...",-1,-1,-1);
	
	WaitCV(lock,cv1);

	Print("Client 1 is now out of wait...",-1,-1,-1);

	ReleaseLock(lock);

	
}
