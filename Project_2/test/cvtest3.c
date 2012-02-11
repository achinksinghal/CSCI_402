#include "syscall.h"

int i=-1;
int j=-1;
int j2=-1;
int loop=-1;
int loop2=-1;

void test()
{
	for (loop2=0; loop2<100; loop2++) 
		Yield();/*This is to ensure that signal is sent only after Wait request */
			
	SignalCV(i,j2+1);/*This prints error statement*/
	SignalCV(i,j);
	
	Exit(0);
}

int main()
{
	i=CreateLock("Lock 0");
	Print("Lock_%d created...\n",i,-1,-1);
	
	for(loop = 0; loop < 1000 ;loop++)
		{
			j=CreateCV("CV j");
			j2=CreateCV("CV j2");

			AcquireLock(i);
			WaitCV(i,j2+1);
			Print("This doesnt print after we get errors for going out of threads..\n",-1,-1,-1);
			Fork(test,"Fork 1");
			
			WaitCV(i,j);
			Print("main is now awake\n",-1,-1,-1);
			DestroyCV(j);/*Only one of the locks is always deleted*/
		}

}