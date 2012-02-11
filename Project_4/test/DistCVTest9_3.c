#include "syscall.h"

int cv[100];
int lck[100];

int loop=0;

int main()
{
	for (loop=0; loop<100; loop++)
	{
		cv[loop] = CreateCV("cv_1");
	}

}
