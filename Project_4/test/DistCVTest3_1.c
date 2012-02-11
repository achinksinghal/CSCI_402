#include "syscall.h"

int mv=-1,mv1=1;
int lck=-1;
int ret=-1;

void main()
{

	mv = CreateMV("mv1");
	lck = CreateLock("lock1");
	

	AcquireLock("lock1");
	ret=SetMV("mv1",9);
	Print("ret = %d..",ret,-1,-1);
	
	ReleaseLock("lock1");

	
	AcquireLock("lock1");
	ret = GetMV("mv1");
	
	Print("Client 1: Monitor variable was initialized to be %d..",ret,-1,-1);
	ReleaseLock("lock1");
	Exit(0);
	

}
