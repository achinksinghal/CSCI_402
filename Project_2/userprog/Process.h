#ifndef PROCESS_H
#define PROCESS_H

#include "addrspace.h"
#include "synch.h"
#include "thread.h"

#define MAX_PROCESS 10 // restricting the number of process that can be loaded in nachos to 10
#define MAX_THREAD 250 // max threads in each process
#define MAX_FORK_AND_EXEC 150 //max forks and execs allowed

class Lock;
class Process
{
	private:
		int processID;    // unique process ID
		AddrSpace* processAddressSpace; // address space of the process
		bool isChildProcess;  // indicates if the current process is the child process
		int childThreadCount; // total number of threads in this process
		int childProcessCount; // total number of child processes
		int childProcess[MAX_PROCESS - 1]; // process IDs of child processes
		int childThreads[MAX_THREAD]; // maximum number of threads inside the process
		//int nextThread;
		int parentProcessID;
		Semaphore* joinSemaphore;
		Lock *processSynchLock;
		Thread *mainThread;
		
		
	public:
	
		int nextThread;
		Process(AddrSpace *);
		void IncrementChildProcessCount(int);
		void IncrementChildThreadCount();
		int Process :: GetChildThreadCount();
		void SetParentProcess(int);
		int ChildThreadCount(); 
		int GetProcessID();
		Semaphore* GetSemaphore();
		void DecrementChildThreadCount();
		int RemoveChildProcess(int);
		bool IsMainThread(Thread *th) {return th == mainThread;}
		void SetMainThread(Thread *th){mainThread = th;}
};
#endif
