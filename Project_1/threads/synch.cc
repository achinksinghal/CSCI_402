// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//enum LockStatus {FREE, BUSY}; --Moved to synch.h

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
	DEBUG('t',"\nInitializing Semaphore variable..\n");

    name = debugName;
    value = initialValue;
    queue = new List();
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
	DEBUG('t',"\nInside Semaphore::P() --Wait-- ... \n");
	IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts 
	//(OldLevel has state of interrupt just prior to turning them off)
    
    while (value == 0) { 			// semaphore not available
	queue->Append((void *)currentThread);	// so go to sleep
	currentThread->Sleep();
    } 
    value--; 					// semaphore available, 
						// consume its value
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
	DEBUG('t',"\nInside Semaphore::v() --Signal-- ... \n");

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
		scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!

Lock::Lock(char* debugName) // constructor for Lock class
{

    name = debugName;

	DEBUG('t',"\nInitializing new Lock Variable with name as %s\n", name);

    LockState = FREE; // Lock initialized as free
    LockOwner = NULL; // For a new lock, current thread is the new owner -- 
    LockWaitQueue = new List(); // Wait queue for the lock
//    LockWaitQueue = (List *)malloc(sizeof(List)); // Wait queue for the lock
}

Lock::~Lock() // Deconstructor for Lock class
{
	LockState = FREE;
	delete LockWaitQueue;
//	free(LockWaitQueue);
}

bool Lock::isHeldByCurrentThread() {// We'll check if lock is held by current thread
	DEBUG('t',"\nInside Lock::isHeldByCurrentThread(), will now try and see if currentThread has Lock -- %s",name);	
	if (currentThread == LockOwner) 
	{
		DEBUG('t',"-- Yes it did\n");
		return true;
	}
	else 
	{
		DEBUG('t',"-- Nope it didnt\n");
		return false;

	}
}
/*
Thread* Lock::isOwnedByWhatThread()
{
	return LockOwner;	
}
*/
void Lock::Acquire() { // This function will acquire the lock

	IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
	DEBUG('t',"\nInside Lock::Acquire(), will now try and see if we can acquire this Lock -- %s -- or not..\n",name);	

	if (isHeldByCurrentThread()) // Check if I am the owner
		{
			//printf("\nThis thread already acquired the lock.. So, doesnt have it acquire it again\n");
			(void) interrupt->SetLevel(oldLevel);	// The lock was already with currentThread so I simply re-enabled interrupts
			return ; 		
		}

	if (LockState == FREE) // Checking if lock is available
		{
			DEBUG('t',"\nLock was found to be FREE and can be acquired\n");

			LockState = BUSY; // Change state of lock to busy
			LockOwner = currentThread;// Make yourself lock holder
			DEBUG('t',"\nWill now re-enable interrupts and then leave\n");
	
			(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
			return ;	

		}
	else
		{ // Lock wasnt available
			DEBUG('t',"\nLock was not found to be FREE and thread will now be put on LockWaitQueue\n");
		
			LockWaitQueue->Append((void *)currentThread);	//  add to wait queue and fall asleep
//			LockWaitQueue.Append((void *)currentThread);	//  add to wait queue and fall asleep
			currentThread->Sleep();
			DEBUG('t',"\nWill now re-enable interrupts and then leave\n");
	
			(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
			return ;	

		}

}

void Lock::Release() 
{
	IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
	DEBUG('t',"\n\nWill now try and see if we can release this Lock -- %s -- or not..\n",name);
	
	if(!(isHeldByCurrentThread()))// Checking if I do own the lock I am trying to let go
	{
		//printf("PROBLEM: You are trying to release a lock you do not own. You cannot do that.\n");
		(void) interrupt->SetLevel(oldLevel);	// Restoring Interrupts
		return;
	}
	else
	{
	
		if (LockWaitQueue->IsEmpty())			// Atleast 1 thread is waiting for the lock in its queue
		{
			LockState = FREE; // Change state of lock to free
			LockOwner = NULL;// Nobody owns the clock so clear out ownership -- 
			DEBUG('t',"LockWaitQueue was found to be empty. So no one there to release. Simply set LockState as FREE\n");

		}
		else								// No one was waiting for the lock
		{
			DEBUG('t',"Thread -- %s -- released Lock -- %s --\n",currentThread->getName(),getName());
			Thread *thread;
//			thread = (Thread *)LockWaitQueue.Remove(); 		// Remove one thread and give it the lock
			thread = (Thread *)LockWaitQueue->Remove(); 		// Remove one thread and give it the lock
			DEBUG('t',"\n Took out Thread -- %s -- from LockWaitQueue to give it the lock -- %s --\n",thread->getName(),getName());

			LockOwner = thread;// Make yourself lock holder
			LockState = BUSY; // Change state of lock to busy

			scheduler->ReadyToRun(thread); 	// Put that thread in the Ready Queue in the Ready
		}
	(void) interrupt->SetLevel(oldLevel);	// Restoring Interrupts	
	return;
		
	}
}

Condition::Condition(char* debugName) 
{ 
	DEBUG('t',"\nInitializing new Condition Variable\n");
	name=debugName;
	WaitingLock = NULL;
	
    //WaitingLock = new Lock("Condition Wait Lock"); // Start a new lock as condition lock, picked it up from synchlist.cc -- 
    ConditionWaitQueue = new List; // Wait queue for the lock
	if (ConditionWaitQueue->IsEmpty())
		DEBUG('t',"\nIts wait queue is initialized to be NULL\n");
	else
		DEBUG('t',"\nIts wait queue is initialized to be NOT NULL\n");

}

Condition::~Condition() 
{ 
	delete ConditionWaitQueue;
}

void Condition::Wait(Lock *conditionLock) { // conditionLock is the lock you required to enter the monitor and later you give up the lock
	//ASSERT(FALSE); // This was the given code
	DEBUG('t',"\nInside Condition::Wait\n");

	IntStatus oldLevel = interrupt->SetLevel(IntOff);	// Disable interrupts
	if (conditionLock == NULL) // Somebody tries to pass a a bad object
	{
		//printf("\nPROBLEM: You passed a bad object.\n");
		(void) interrupt->SetLevel(oldLevel);	// Restoring Interrupts
		return;
	}
	// Waiting for this lock is the lock waiting people have given up
	// this had been introduced to keep track of which lock are we currently waiting for. Eg - Clerk1 or Clerk2 or whatever

	if (WaitingLock == NULL) 
	{
	// This is first thread calling wait; no one else has given up a lock yet
		DEBUG('t',"\nThis is first thread calling wait; no one else has given up a lock yet.\n");

		WaitingLock = conditionLock; 
		//Everyone gives up the same lock to get a chance to wait; having multiple locks you do not have mutually exclusive access
		// Now we need to Wait
/*		conditionLock -> Release(); // Release my lock before going to sleep; exited the monitor here
		ConditionWaitQueue->Append((void *)currentThread);	//Add myself to lock queue for Condition Variable
		DEBUG('t',"\nAppended an item to list ConditionWaitQueue\n");
		currentThread->Sleep(); // And now time to sleep till somebody wakes me up
		
		conditionLock->Acquire(); // First thing I do after I wake up is acquire the lock again
		(void) interrupt->SetLevel(oldLevel);	// Restoring Interrupts
		return;
*/	}
	
	if (WaitingLock != conditionLock)
	{
		//printf("\nPROBLEM: You have called Wait with the wrong lock\n");
		(void) interrupt->SetLevel(oldLevel);	// Restoring Interrupts
		return;	
	}
		//We reach here if there are already elements in COnditionWaitQueue
		conditionLock -> Release(); // Release my lock before going to sleep; exited the monitor here
		ConditionWaitQueue->Append((void *)currentThread);	//Add myself to lock queue for Condition Variable
		DEBUG('t',"\nAppended an item to list ConditionWaitQueue\n");
		currentThread->Sleep(); // And now time to sleep till somebody wakes me up
		
		conditionLock->Acquire(); // First thing I do after I wake up is acquire the lock again
		(void) interrupt->SetLevel(oldLevel);	// Restoring Interrupts
		return;

}

void Condition::Signal(Lock *conditionLock) 
{ 
	DEBUG('t',"\nInside Condition::Signal\n");

	IntStatus oldLevel = interrupt->SetLevel(IntOff);	// Disable interrupts
	if(ConditionWaitQueue->IsEmpty()) // Nobody is waiting
	{
		DEBUG('t',"\nNobody was found to be in the Condition Wait Queue.\n");

		(void) interrupt->SetLevel(oldLevel);	// Restoring Interrupts		
		return;
	}
	
	if(conditionLock!=WaitingLock)
	{
		//printf("\nPROBLEM: Sorry you passed in the wrong lock to this Signal function. There is nobody waiting for this lock\n");	
		(void) interrupt->SetLevel(oldLevel);	// Restoring Interrupts		
		return;
	}
	else	// Time to wake up one waiter from the ConditionWaitQueue
	{
		DEBUG('t',"\nSearching for threads in the ConditionWaitQueue --\n");//For testing	
		Thread *thread;
		thread = (Thread *)ConditionWaitQueue->Remove(); 		// Remove one thread and give it the lock
		DEBUG('t',"\nA thread was removed from the ConditionWaitQueue --- %s \n",thread->getName());//For testing
		scheduler->ReadyToRun(thread); 	// Put that thread in the Ready Queue in the Ready State
	
		if (ConditionWaitQueue->IsEmpty()) // If only 1 thread was waiting we can set the lock waiting to Null as there is no more left
			{
				DEBUG('t',"\nNobody else is now left in ConditionWaitQueue\n");//For testing
				WaitingLock == NULL;
			}

		(void) interrupt->SetLevel(oldLevel);	// Restoring Interrupts		
		return;
	}
}

void Condition::Broadcast(Lock *conditionLock) 
{
	DEBUG('t',"\nInside Condition::Broadcast\n");
	while(!(ConditionWaitQueue->IsEmpty())) // Atleast 1 is waiting
	{
		Signal(conditionLock);
	}
}
