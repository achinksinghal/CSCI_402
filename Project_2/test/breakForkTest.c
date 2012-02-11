#include "syscall.h"

int loop = -1;

int test()
{
	Exit(0);
}

int main()
{

	for (loop =0 ; loop<500; loop++)
		Fork(test,"break Fork");
	
	
}
