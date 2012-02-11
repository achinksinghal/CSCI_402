#include "syscall.h"
#define Num 1024
int A[Num],B[Num];	/* size of physical memory; with code, we'll run out of space!*/

int i = -1;

void sortA()
{
   int i, j, tmp;
	Print("Forked Sort started...\n",-1,-1,-1);

    /* first initialize the array, in reverse sorted order */
    for (i = 0; i < 1024; i++)
        A[i] = 1024 - i;

    /* then sort! */
    for (i = 0; i < 1023; i++)
        for (j = i; j < (1023 - i); j++)
           if (A[j] > A[j + 1]) {       /* out of order -> need to swap ! */
              tmp = A[j];
              A[j] = A[j + 1];
              A[j + 1] = tmp;
           }
	Print("\nForked: The value after sort is %d\n",A[0],-1,-1);
    Exit(0);         
}


int main()
{

	Fork(sortA, "sortA");

	return 1;
}
