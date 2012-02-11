#include "syscall.h"

int i=-1;
int j=-1;


void test()
{
	AcquireLock(i);
	Print("Signalling CV_%d on lock_%d\n",j,i,-1);
	SignalCV(i,j);
	ReleaseLock(i);
	Exit(0);	
}

void test2()
{
	AcquireLock(i);
	WaitCV(i,j);/*This shows signals are not stored in memory*/
	Print("This never prints as no one ever signals test2 ...\n",-1,-1,-1);
	Exit(0);
}

int main()
{
	
	i=CreateLock("Lock 0");
	j=CreateCV("Lock 1");
	
	AcquireLock(i);
	
	Fork(test,"Fork 1");
	
	WaitCV(i,j);
	
	Print("This is last statement that prints ..\n",-1,-1,-1);

	Fork(test2,"Fork 2");
}
