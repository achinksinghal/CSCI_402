/*// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

//#include "thread.h"*/
#include "syscall.h"

/*//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------
*/
void
SimpleThread()
{
    int num;
    
    for (num = 0; num < 5; num++) {
			Print("\n***Thread 1 loops***", -1, -1, -1);
			
	
    }
	Exit(0);
}

void Test2()
{
	Fork(SimpleThread, "1234");
	Print("\nTest 2 forked", -1, -1, -1);
	Exit(0);
}
void test()
{
	Fork(Test2, "4567");
	Exit(0);
}

/*//----------------------------------------------------------------------
// ThreadTest
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------*/

void
main()
{
	Print("\nEntering Main Thread\n", -1, -1, -1);
	
	Fork(test, "5328");
	Fork(SimpleThread, "34e23");
	Fork(SimpleThread, "qe32");
	Fork(SimpleThread, "3214");
	
	Print("\nExiting Proc", -1, -1, -1);
	Exit(0);
	
}

