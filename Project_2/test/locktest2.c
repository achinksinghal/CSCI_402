#include "syscall.h"

int i=-1;
int j=-1;
int k=-1;

void t1_t2()
{
	for (k=0; k<999 ; k++)
		Yield(); /*so many yields ensure that thread t1_t1 is executed first and t1_t2 never gets lock_j*/
	
	AcquireLock(i);
	NewLine();
	Print(" t1_t2 acquired lock_%d \n",i,-1,-1);
	AcquireLock(j);
	NewLine();
	Print(" t1_t2 acquired lock_%d \n",j, -1,-1);

	ReleaseLock(j);
	NewLine();
	Print(" t1_t2 released lock_%d \n",j,-1,-1);
	NewLine();	
	DestroyLock(i);
	
	ReleaseLock(i);
	NewLine();
	Print(" t1_t2 released lock_%d \n",i,-1,-1);


	Exit(0);
}

void t1_t1()
{
	
	AcquireLock(i);
	NewLine();
	Print(" t1_t1 acquired lock_%d \n",i,-1,-1);
	AcquireLock(j);
	NewLine();
	Print(" t1_t1 acquired lock_%d \n",j,-1,-1);

	ReleaseLock(i);
	NewLine();
	DestroyLock(j);
	ReleaseLock(j);   
	NewLine();
	Print(" t1_t1 released lock_%d \n",j,-1,-1);
	Exit(0);
}

int main()
{
	i=CreateLock("Locktest1");
	j=CreateLock("Locktest2");

	

	Fork(t1_t1,"t1_t1");
	Fork(t1_t2,"t1_t2");
	
	
}

