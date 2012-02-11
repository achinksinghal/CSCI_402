/* matmult.c 
 *    Test program to do matrix multiplication on large arrays.
 *
 *    Intended to stress virtual memory system.
 *
 *    Ideally, we could read the matrices off of the file system,
 *	and store the result back to the file system!
 */

#include "syscall.h"

#define Dim 	20	/* sum total of the arrays doesn't fit in 
			 * physical memory 
			 */

int A[Dim][Dim];
int B[Dim][Dim];
int C[Dim][Dim];
int DA[Dim][Dim];
int DB[Dim][Dim];
int DC[Dim][Dim];


void
matmultA()
{
    int i, j, k;

    for (i = 0; i < Dim; i++)		/* first initialize the matrices */
	for (j = 0; j < Dim; j++) {
	     A[i][j] = i;
	     B[i][j] = j;
	     C[i][j] = 0;
	}

    for (i = 0; i < Dim; i++)		/* then multiply them together */
	for (j = 0; j < Dim; j++)
            for (k = 0; k < Dim; k++)
		 C[i][j] += A[i][k] * B[k][j];

	for (i = 0; i < Dim; i++)	 
		{	
			for (j = 0; j < Dim; j++)
				{
					/*Print(" C[%d][%d] is %d ",i,j,C[i][j]);*/
				}
					/*NewLine();*/
		}
	Print("\n  Forked 1: The last value is %d \n",C[19][19],-1,-1);
    /*Exit(C[Dim-1][Dim-1]);		 and then we're done */
	Exit(0);
}

void
matmultD()
{
    int i, j, k;

    for (i = 0; i < Dim; i++)		/* first initialize the matrices */
	for (j = 0; j < Dim; j++) {
	     DA[i][j] = i;
	     DB[i][j] = j;
	     DC[i][j] = 0;
	}

    for (i = 0; i < Dim; i++)		/* then multiply them together */
	for (j = 0; j < Dim; j++)
            for (k = 0; k < Dim; k++)
		 DC[i][j] += DA[i][k] * DB[k][j];

	for (i = 0; i < Dim; i++)	 
		{	
			for (j = 0; j < Dim; j++)
				{
					/*Print(" DC[%d][%d] is %d ",i,j,DC[i][j]);*/
				}
					/*NewLine();*/
		}
	Print("\n Forked 2: The last value is %d \n",DC[19][19],-1,-1);
    /*Exit(C[Dim-1][Dim-1]);		 and then we're done */
	Exit(0);
}

int main()
{
	Fork(matmultA, "matmultA");
	Fork(matmultD, "matmultD");
}
