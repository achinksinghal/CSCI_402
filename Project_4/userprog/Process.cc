#include "system.h"
#include "Process.h"
#include "ThreadControl.h"

//-----------------------------------------------------------------------
// Process table area
//-----------------------------------------------------------------------
// create a processs
Process::Process(AddrSpace *space)
{
	processLock -> Acquire();
	int nextProcID = processBitMap -> Find(); // find the next available process id to associate with the process
	if(ProcessCount > MAX_PROCESS) // the process id shouldn't exceed the maximum allowable process in the system
	{
		printf("\nMaximum available processes exceeded\n");
		//processLock -> Acquire();
		processLock -> Release();
		nextProcID = -1;
	}
	// increment the total number of processes in the system
	ProcessCount++;
	processLock -> Release();
	processID = nextProcID;
	isChildProcess = false;
	childProcessCount = 0;
	// initialize the number of child threads associated with the process
	childThreadCount = 0;
	parentProcessID = -1;
	nextThread = 0;
	
	char *name = new char[50];
	sprintf(name, "Process%dSynchLock", processID);
	processSynchLock = new Lock(name);
	
	name = new char[50];
	sprintf(name,"JoiningSemaphore%d", nextProcID);
	joinSemaphore = new Semaphore(name, 0);
	
	//associate the newly created address space to the process
	processAddressSpace = space;
}

// increment the child process spawned by this process
void Process :: IncrementChildProcessCount(int childProcessID)
{
	childProcess[childProcessCount] = childProcessID;
	childProcessCount++;
}

// increment the child thread count 
void Process :: IncrementChildThreadCount()
{
	childThreadCount++;
	nextThread++;
	//printf("\n\childThreadCount is now %d\n\n",childThreadCount);
}

// find the total number of child threads of the process
int Process :: ChildThreadCount()
{
	//processSynchLock -> Acquire();
	int cnt = childThreadCount;
	//processSynchLock -> Release();
	return cnt;
}

//duplicate function. not used
int Process :: GetChildThreadCount()
{
	return childThreadCount;
}

// decerement the child threads associated with the process
void Process :: DecrementChildThreadCount()
{
	childThreadCount--;
}

//  remove the exiting process if its associated with the process
int Process :: RemoveChildProcess(int childProcessID)
{
	int idx;
	
	for(idx = 0; idx < childProcessCount; idx++)
	{
		// find where is the child process in the parent process references
		if(childProcess[idx] = childProcessID)
		{
			break;
		}
	}
	
	if(idx > childProcessCount)
	{
		printf("\nError in terminating the child process %d. Returning back to the main program\n", childProcessID);
		return -1;
	}
	else
	{
		//invalidate the child process entry in the parent process references 
		childProcess[idx] = -1;
	}
	
	return 0;
}

// set the parent process of a process
void Process :: SetParentProcess(int parentID)
{
	parentProcessID = parentID;
	isChildProcess = true;
}

//  get synch param of the process... not used anywhere
Semaphore* Process :: GetSemaphore()
{
	return joinSemaphore;
}

// find the process id of the process
int Process :: GetProcessID()
{
	return processID;
}

//  create a thread control block for a new thread
ThreadControlBlock :: ThreadControlBlock(int th)
{
	thread = th;
	joinSem = (int)new Semaphore("ThreadJoiningSem", 0);
	StatusLock = (int) new Lock("StatusLock");
	IsExiting = false;
}

// not used
int ThreadControlBlock :: GetSynchParam()
{
	return joinSem;
}

// set the starting virtual page of a thread stack
void ThreadControlBlock :: SetStackStart(int pageNumber)
{
	stackPageStart = pageNumber;
}

// find the stack start of a thread stack
int ThreadControlBlock :: GetStackStart()
{
	return stackPageStart;
}

// not used
void ThreadControlBlock :: SetExitStatus()
{
	Lock *statusLock = (Lock *) StatusLock;
	statusLock -> Acquire();
	IsExiting = true;
	statusLock -> Release();
}

// not used
bool ThreadControlBlock :: GetExitStatus()
{
	Lock *statusLock = (Lock *) StatusLock;
	statusLock -> Acquire();
	bool exit = IsExiting;
	statusLock -> Release();
	
	return exit;
}
