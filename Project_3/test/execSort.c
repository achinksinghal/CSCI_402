#include "syscall.h"
#define Num 1024

int i = -1;


int main()
{

	for (i=0; i<2; i++) 	
		Exec("../test/sort",13);

	return 1;
}
