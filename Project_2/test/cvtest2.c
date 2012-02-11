#include "syscall.h"

int i=-1;
int j=-1;
int k=-1;
int loop = 0;

void t1_t3()
{

	/*SignalCV(i ,k);*/
	BroadcastCV(i,k);
	DestroyCV(k);
	Exit(0);
}

void t1_t2()
{
	AcquireLock(i);
	WaitCV(i, k);
	Print(" t1_t2 is now awake..\n ",-1,-1,-1);
	ReleaseLock(i);
	Exit(0);
}

void t1_t1()
{
	AcquireLock(i);
	WaitCV(i, k);
	Print(" t1_t1 is now awake.. \n",-1,-1,-1);
	ReleaseLock(i);
	Exit(0);

}

int main()
{
	i=CreateLock("Locktest1");
	j=CreateLock("Locktest2");

	for (j=0;j<9;j++)
	{
		k=CreateCV("CVtest");
		NewLine();
		PrintTab();
		Print("CV_%d was created\n",k,-1,-1);
		Fork(t1_t1,"t1_t1");
		for (loop=0; loop<100; loop++)
			Yield(); 
		
		Fork(t1_t2,"t1_t2");
		for (loop=0; loop<100; loop++)
			Yield(); 
		
		Fork(t1_t3,"t1_t3");
		for (loop=0; loop<1000; loop++)
			Yield(); 
			/*This shows that broadcast is not kept in memory*/
		/*Locks destroyed by now so this wait isnt serviced*/
		WaitCV(i, k);
		
		Print(" This prints.. \n",-1,-1,-1);
	}		
	
	
}

