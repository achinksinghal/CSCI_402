/* sort.c 
 *    Test program to sort a large number of integers.
 *
 *    Intention is to stress virtual memory system.
 *
 *    Ideally, we could read the unsorted array off of the file system,
 *	and store the result back to the file system!
 */

#include "syscall.h"
#define Num 24
int A[Num];	/* size of physical memory; with code, we'll run out of space!*/

int
main()
{
    int i, j, tmp;

    /* first initialize the array, in reverse sorted order */
    for (i = 0; i < Num; i++)
	{
		A[i] = Num - i;
		Print2("A[%d] is %d",i,A[i],0);
		
	}

    /* then sort! */
    for (i = 0; i < (Num-1); i++)
        for (j = i; j < ((Num-1) - i); j++)
	   if (A[j] > A[j + 1]) {	/* out of order -> need to swap ! */
	      tmp = A[j];
	      A[j] = A[j + 1];
	      A[j + 1] = tmp;
    	   }
		   
	Print2("%d",A[0],0,0);
    Exit(0);		/* and then we're done -- should be 0! */
}
