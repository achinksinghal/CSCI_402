#include "syscall.h"
#define Num 1024
int B[Num];	/* size of physical memory; with code, we'll run out of space!*/

int i = -1;

void sort()
{
   int i, j, tmp;
	Print("\n Forked sort started..\n",-1,-1,-1);

    /* first initialize the array, in reverse sorted order */
    for (i = 0; i < 1024; i++)
        B[i] = 1024 - i;

    /* then sort! */
    for (i = 0; i < 1023; i++)
        for (j = i; j < (1023 - i); j++)
           if (B[j] > B[j + 1]) {       /* out of order -> need to swap ! */
              tmp = B[j];
              B[j] = B[j + 1];
              B[j + 1] = tmp;
           }
	Print("\nForked: The value after sort is %d\n",B[0],-1,-1);
    Exit(0);         
}


int main()
{	
	Exec("../test/sort",13);

	Fork(sort, "sortFork");

	return 1;
}
