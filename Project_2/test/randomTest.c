#include "syscall.h"

int loop=-1;

void t4()
{
	for (loop=0; loop<4; loop++)
		Print("Random number %d generated in thread 4 is %d\n",loop, Random(),-1);
	
	Exit(0);
}	

void t3()
{
	for (loop=0; loop<4; loop++)
		Print("Random number %d generated in thread 3 is %d\n",loop, Random(),-1);

	Exit(0);
	
}	

void t2()
{
	for (loop=0; loop<4; loop++)
		Print("Random number %d generated in thread 2 is %d\n",loop, Random(),-1);
	
	Exit(0);
}

void t1()
{
	for (loop=0; loop<4; loop++)
		Print("Random number %d generated in thread 1 is %d\n",loop, Random(),-1);

	Exit(0);
	
}


int main()
{
	Fork(t1,"Thread 1");
	Fork(t2,"Thread 2");
	Fork(t3,"Thread 3");
	Fork(t4,"Thread 4");
}