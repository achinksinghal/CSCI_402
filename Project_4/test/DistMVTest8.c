#include "syscall.h"

int mv=-1;
int lck=-1;
int lck2=-1;
int cv=-1;
int ret=-1;
int mv2=-1;

void main()
{

	mv = CreateMV("mv1");
	mv2 = CreateMV("mv2");
	
	lck = CreateLock("lock1");
	lck2 = CreateLock("lock2");
	
	cv = CreateCV("cv1");

	AcquireLock(lck);
	ret=SetMV(mv,500);
	Print("ret = %d..",ret,-1,-1);
	
	ReleaseLock(lck);
	
	AcquireLock(lck2);
	ret=SetMV(mv2,11);
	
	ReleaseLock(lck2);
	
	AcquireLock(lck);
	ret = GetMV(mv);
	
	Print("Client 1: Monitor variable was initialized to be %d..",ret,-1,-1);
	ReleaseLock(lck);
	
	AcquireLock(lck2);
	ret = GetMV(mv2);
	
	/*this monitor variable is set to 11 here to signify 
	*none of the processes is over 
	*/
	ReleaseLock(lck2);
	
	
}
