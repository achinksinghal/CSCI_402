#include "syscall.h"

int mv=-1,mv2=-1;
int mvEdit=0;
int ret=-1;
int loop=0;
int lck=-1;
int lck2=-1;
int cv=-1;
int i=0;
int main()
{
	mv = CreateMV("mv");
	mv2 = CreateMV("mv2");
	
	lck = CreateLock("lock1");
	lck2 = CreateLock("lock2");
	
	cv = CreateCV("cv1");

	for (loop =0; loop<500; loop++) 
	{
		AcquireLock("lock1");
		SignalCV("lock1","cv1");

		AcquireLock("lock2");
		ret = GetMV("mv2");
		ReleaseLock("lock2");
		
		Print("Client 2: Trying for access to MV..\n",-1,-1,-1);

		if (ret != 21)/*This will wait only if the other one isnt finished*/
			WaitCV("lock1","cv1");

			
		for (i=0; i<9; i++)
			Print("\n",-1,-1,-1);

		mvEdit = GetMV("mv");
		Print("Client 2: Monitor variable was read to be %d..",mvEdit,-1,-1);

		mvEdit+=3;
	
		
		ret=SetMV("mv",mvEdit);
		mvEdit = GetMV("mv");
	
		Print("Client 2: Monitor variable was set to be %d..",mvEdit,-1,-1);
		
		ReleaseLock("lock1");	

		Print("\n\n\n\n\n\nClient 2: %d Iterations done..",(loop+1),-1,-1);
	}
	
	AcquireLock("lock2");
	ret = SetMV("mv2",21);
	
	/*this monitor variable is set to 21 here to signify 
	*one of the processes is over and the other will not
	*wait for it to signal it
	*/
	ReleaseLock("lock2");
	
	AcquireLock("lock1");
	SignalCV("lock1","cv1");
	ReleaseLock("lock1");

	Exit(0);

}
