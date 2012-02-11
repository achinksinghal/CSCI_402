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

int Lock::Release() 
{
	IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
	DEBUG('t',"\n\nWill now try and see if we can release this Lock -- %s -- or not..\n",name);
	
	if(!(isHeldByCurrentThread()))// Checking if I do own the lock I am trying to let go
	{
		//printf("PROBLEM: You are trying to release a lock you do not own. You cannot do that.\n");
		(void) interrupt->SetLevel(oldLevel);	// Restoring Interrupts
		return 0;
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
	return 1;
		
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

int Condition::Signal(Lock *conditionLock) 
{ 
	DEBUG('t',"\nInside Condition::Signal\n");

	IntStatus oldLevel = interrupt->SetLevel(IntOff);	// Disable interrupts
	if(ConditionWaitQueue->IsEmpty()) // Nobody is waiting
	{
		DEBUG('t',"\nNobody was found to be in the Condition Wait Queue.\n");

		(void) interrupt->SetLevel(oldLevel);	// Restoring Interrupts		
		return 0;
	}
	
	if(conditionLock!=WaitingLock)
	{
		//printf("\nPROBLEM: Sorry you passed in the wrong lock to this Signal function. There is nobody waiting for this lock\n");	
		(void) interrupt->SetLevel(oldLevel);	// Restoring Interrupts		
		return 0;
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
		return 1;
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

//Server Response code

//-----------------------------------------------------------------------------
//	Response:: Response
//	creates a response message to be saved in the server queue
//-----------------------------------------------------------------------------
Response::Response(int netID, int mBoxID, bool isError, char *msg)
{
	networkID = netID;
	mailboxID = mBoxID;
	isError = isError;
	response = msg;
}

//Area for the server lock
//------------------------------------------------------------------------------
// ServerLock::ServerLock
// Constructor for ServerLock class. Initialize the wait queue and the 
// owner of the lock to null and the state to be FREE
//------------------------------------------------------------------------------
ServerLock::ServerLock(char *name)
{
	lockState = FREE;
	LockWaitQueue = new List();
	this -> name = name;
	ownerNetworkID = -1;
	ownerMailboxID = -1;
	refCount =0;
	deleteCalled = false;
}


//--------------------------------------------------------------------------------
// ServerLock::~ServerLock
// Deallocate the memory allocated to the server lock
//
//--------------------------------------------------------------------------------
ServerLock::~ ServerLock()
{
	delete LockWaitQueue;
	deleteCalled = false;
	refCount = 0;
}

//--------------------------------------------------------------------------------
// ServerLock :: Acquire
// Tries to acquire the lock for the client requesting the lock. The address of 
// the client is passed to the function and gets set as the owner of the lock
//--------------------------------------------------------------------------------
void ServerLock :: Acquire(int networkID, int mailboxID)
{
	if(lockState == FREE)
	{	// lock is free assign the lock to the client 
		lockState = BUSY;
		// set the owner ship of the lock
		ownerNetworkID = networkID;
		ownerMailboxID = mailboxID;
		
	}
	else
	{
		// queue the response that will be needed to send to the client 
		Response *response = new Response(networkID, mailboxID, false, "");
		LockWaitQueue -> Append((void *)response);
	}
	
	// increment reference count
	refCount++;
}


//----------------------------------------------------------------------------------
// ServerLock :: Release
// Release the lock on behalf of the client requesting the release. The response is
// sent bacl to the server so that it can honor the next acquire request
//----------------------------------------------------------------------------------
Response* ServerLock :: Release(int networkID, int mailboxID)
{
	// check if the lock is being release by a valid owner
	if((ownerNetworkID == networkID) && (ownerMailboxID == mailboxID))
	{
		// check if the wait queue is not empty
		
		if(LockWaitQueue -> IsEmpty()) // this was the last client who held the lock
		{
			lockState = FREE;
			Response *response = new Response(-1,-1,false,"");
			printf("\nThe lock is free\n");
			return response;
		}
		else // there are pending requests which need to be fulfilled
		{
			lockState = BUSY;
			Response *response = (Response *) LockWaitQueue -> Remove();
			// set the new owner of the lock
			ownerNetworkID = response -> networkID;
			ownerMailboxID = response -> mailboxID;
			
			// decrement the reference count
			refCount--;
			
			// send the response message to the client
			return response;
		}
	}
	else
	{
		//client is trying to release a lock not held by it
		Response *response = new Response(networkID, mailboxID, true, "Invalid Release");
		return response;
	}
}

//--------------------------------------------------------------------
// ServerLock :: GetLockState
//	Returns the status of the lock
//--------------------------------------------------------------------
LockStatus ServerLock :: GetLockState()
{
	return lockState;
}

//---------------------------------------------------------------------
// ServerCondition :: ServerCondition
//	creates a ServerCondition object and initializes the data for it
//
//---------------------------------------------------------------------
ServerCondition::ServerCondition(char *nm)
{
	name = nm;
	ConditionWaitQueue = new List();
	WaitingLock = -1;
	refCount =0;
	deleteCalled = false;
}

//----------------------------------------------------------------------
// ServerCondition :: ~ ServerCondition
// Performs the cleanup when a ServerConditon Object goes out of scope
//
//----------------------------------------------------------------------
ServerCondition::~ServerCondition()
{
	delete ConditionWaitQueue;
	deleteCalled = false;
	refCount = 0;
}

//---------------------------------------------------------------------
// ServerCondition :: Wait
// Makes a client wait on a condition variable
//
//---------------------------------------------------------------------
Response* ServerCondition :: Wait(int waitLock, int networkID, int mailboxID)
{
	if(waitLock == -1) // the lock passed here hasn't been created yet
	{
		Response *response = new Response(networkID, mailboxID, true, "Null Lock");
		return response;
	}
	
	if(WaitingLock == -1) // This is the first client to invoke this condition variable
	{
		WaitingLock = waitLock;
	}
	
	if(waitLock != WaitingLock) // this is not the same lock as everyone else is waiting on
	{
		Response *response = new Response(networkID, mailboxID, true, "Bad Lock");
		return response;
	}
	
	
	// find the actual server lock to which the id belongs
	ServerLock *servLock = ServerLocks[waitLock];
	Response *response = servLock -> Release(networkID, mailboxID);
	
	// append the response to thw wait queue
	Response *rsp = new Response(networkID, mailboxID, false,"");
	ConditionWaitQueue -> Append((void *)rsp);
	
	//increase the reference count 
	refCount++;
	
	//return the response to the server to be sent back to the client
	return response;
}

//---------------------------------------------------------------------
// ServerCondition :: Signal
// Signals a blocked client
//
//---------------------------------------------------------------------
bool ServerCondition :: Signal(int waitLock)
{
	if(WaitingLock != waitLock) // the locks don't match
	{
		return false;
	}
	else
	{
		Response *response = (Response *)ConditionWaitQueue -> Remove();
		
		//we need to acquire the lock for the client we are going to start
		
		ServerLocks[waitLock] -> Acquire(response -> networkID, response -> mailboxID);
		
		// add the response to the lock wait queue
		//List* lockList = ServerLocks[waitLock] -> getLockList();
		//lockList -> Append((void *) response);
		
		// a new copy of the response will get created in the Acquire call so we can delete this response
		//delete response;
		
		if(ConditionWaitQueue -> IsEmpty())
		{
			WaitingLock = -1;
		}
		
		refCount--;
		return true;
	}
}

//---------------------------------------------------------------------
// ServerCondition :: Broadcast
// Wakes up all the clients blocked on the condition variables
//
//---------------------------------------------------------------------
bool ServerCondition :: Broadcast(int waitLock)
{
	while(!ConditionWaitQueue -> IsEmpty())
	{
		bool result = Signal(waitLock);
		
		if(!result)
		{
			return result;
		}
	}
	
	return true;
}


//---------------------------------------------------------------------
// MonitorVariable :: MonitorVariable
// Initializes the monitor variables
//
//---------------------------------------------------------------------
MonitorVariable :: MonitorVariable(char *nm)
{
	name = nm;
}

//---------------------------------------------------------------------
// MonitorVariable :: ~MonitorVariable
// Performs clean up while de-alllocating MonitorVariable
//
//---------------------------------------------------------------------
MonitorVariable :: ~MonitorVariable()
{
	value = -1;
	delete name;
}

//---------------------------------------------------------------------
// MonitorVariable :: setValue
// sets the value of the MonitorVariable 
//
//---------------------------------------------------------------------
int MonitorVariable :: setValue(int val)
{
	value = val;
	return value;
}