#include "syscall.h"

int mv=-1;
int lck=-1;
int lck2=-1;
int cv=-1;
int ret=-1;
int mv2=-1;

void main()
{

	mv = CreateMV("mv");
	mv2 = CreateMV("mv2");
	
	lck = CreateLock("lock1");
	lck2 = CreateLock("lock2");
	
	cv = CreateCV("cv1");

	AcquireLock("lock1");
	ret=SetMV("mv",500);
	/*Print("ret = %d..",ret,-1,-1);*/
	
	ReleaseLock("lock1");
	
	AcquireLock("lock2");
	ret=SetMV("mv2",11);
	
	ReleaseLock("lock2");
	
	AcquireLock("lock1");
	ret = GetMV("mv");
	
	Print("Client 1: Monitor variable was initialized to be %d.. \n Start client 2",ret,-1,-1);
	ReleaseLock("lock1");
	
	AcquireLock("lock2");
	ret = GetMV("mv2");
	
	/*this monitor variable is set to 11 here to signify 
	*none of the processes is over 
	*/
	ReleaseLock("lock2");
	
	Exit(0);
}
