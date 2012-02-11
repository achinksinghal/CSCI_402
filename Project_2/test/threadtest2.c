/*// threadtest.cc 
//    Simple test case for the threads assignment.
//
//    Create two threads, and have them context switch
//    back and forth between themselves by calling Thread::Yield, 
//    to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

//#include "thread.h"*/
#include "syscall.h"

/*//----------------------------------------------------------------------
// SimpleThread
//     Loop 5 times, yielding the CPU to another ready thread 
//    each iteration.
//
//    "which" is simply a number identifying the thread, for debugging
//    purposes.
//----------------------------------------------------------------------
*/
void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
    if(which == 0)
    {
        Write(" ***Thread 0 loops*** ", 22, 1);
		NewLine();
		PrintTab();
		Print(" ***Thread P%d loops*** ",24,0,-1);
		NewLine();
    }
    else
    {    
        Write(" ***Thread 1 loops*** ", 22, 1);
		NewLine();
		PrintTab();
		Print(" ***Thread P%d loops*** ",24,1,-1);
		NewLine();
    }
    
        Yield();
    }
}

/*//----------------------------------------------------------------------
// ThreadTest
//     Set up a ping-pong between two threads, by forking a thread 
//    to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------*/

void
main()
{
    Write(" Entering Main Thread ", 22,1);
	NewLine();
	Print(" P Entering Main Thread ", 24,1,-1);
	NewLine();
	Fork(SimpleThread, 1);
    SimpleThread(0);
    
    Write(" Exiting the main thread ",25,1);
	NewLine();    
	PrintTab();
	Print(" P Exiting the main thread ",27,1,-1);
    NewLine();
}
