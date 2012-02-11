#include "syscall.h"

int i=-1;

int test()
{
	int int2=0;
	for (int2=0;int2<550;int2++)
		{	
			i=CreateLock("locksTesting");
			AcquireLock(i);
			ReleaseLock(i);
			Print("New loop is %d\n",int2,-1,-1);
			if (int2 % 10 == 0) 
				DestroyLock(i);
		}
	Exit(0);		
}

int main()
{
	int int1=0;
	/*for (int1=0; int1<10; int1++)	*/
		Fork(test,"Fork 1");

}
