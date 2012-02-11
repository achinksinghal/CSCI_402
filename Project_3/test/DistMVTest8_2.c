#include "syscall.h"

int mv=-1,mv2=-1;
int mvEdit=0;
int ret=-1;
int loop=0;
int lck=-1;
int lck2=-1;
int cv=-1;

int main()
{
	mv = CreateMV("mv1");
	mv2 = CreateMV("mv2");
	
	lck = CreateLock("lock1");
	lck2 = CreateLock("lock2");
	
	cv = CreateCV("cv1");

	for (loop =0; loop<500; loop++) 
	{
		AcquireLock(lck);
		SignalCV(lck,cv);

		AcquireLock(lck2);
		ret = GetMV(mv2);
		ReleaseLock(lck2);
		
		if (ret != 21)/*This will wait only if the other one isnt finished*/
			WaitCV(lck,cv);

		mvEdit = GetMV(mv);
		Print("Client 2: Monitor variable was read to be %d..",mvEdit,-1,-1);

		mvEdit+=3;
	
		
		ret=SetMV(mv,mvEdit);
		mvEdit = GetMV(lck);
	
		Print("Client 2: Monitor variable was set to be %d..",mvEdit,-1,-1);
		
		ReleaseLock(lck);	

		Print("Client 2: %d Iterations done..",loop,-1,-1);
	}
	
	AcquireLock(lck2);
	ret = SetMV(mv2,21);
	
	/*this monitor variable is set to 21 here to signify 
	*one of the processes is over and the other will not
	*wait for it to signal it
	*/
	ReleaseLock(lck2);
	
	AcquireLock(lck);
	SignalCV(lck,cv);
	ReleaseLock(lck);

}
