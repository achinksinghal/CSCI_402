#include "syscall.h"
#define Num 1024
int A[Num],B[Num];	/* size of physical memory; with code, we'll run out of space!*/

int i = -1;

void sortA()
{
   int i, j, tmp;
	Print("\nForked Sort 1 started...\n",-1,-1,-1);

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
	Print("\nForked 1: The value after sort is %d\n",A[0],-1,-1);
    Exit(0);         
}

void sortB()
{
   int i, j, tmp;
	Print("\nForked Sort 2 started...\n",-1,-1,-1);

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
	Print("\nForked 2: The value after sort is %d\n \n",B[0],-1,-1);
    Exit(0);         
}


int main()
{

	Fork(sortA, "sortA");
	Fork(sortB, "sortB");

	return 1;
}
