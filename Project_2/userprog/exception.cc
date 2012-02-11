// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "../threads/system.h"
#include "syscall.h"
#include <stdio.h>
#include <iostream>
#include "thread.h"
#include "synch.h"
#include <string.h>
#include "Process.h"
#include "ThreadControl.h"

#define NameSize 30
#define PrintLength 100
#define ScanLength 1

int totalFork =0 ;

using namespace std;

Lock *printLock = new Lock("Lock for printing");
Lock *forkLock = new Lock("ForkLock");

enum YES_NO
{
	YES=2,
	NO=3
};


//The data structure that defines a lock
typedef struct _KernelLocks
{
    Lock *lock;
    AddrSpace *currentAddrSpace;
    YES_NO inUse;
    YES_NO isGoingToBeDeleted;
    int isGoingToBeUsed;
}KernelLocks;

//the total number of locks available in the nachos kernel to be provided to the user programs
typedef struct _KlCb
{
    KernelLocks kl[MAX_LOCKS];
    Lock *kLock;
    int nextLockLocation;
    char kLockName[20];
}KlCb;
KlCb klCb;

// the data structure that defines a condition variable for a user program
typedef struct _KernelCVs
{
    Condition *cv;
    AddrSpace *currentAddrSpace;
    YES_NO inUse;
    YES_NO isGoingToBeDeleted;
    int isGoingToBeUsed;
}KernelCVs;


// the totaal number of condition variables in the nachos kernel to be provided to the user programs
typedef struct _KcvCb
{
    KernelCVs kcv[MAX_CVS];
    Lock *kLock;
    int nextCVLocation;
    char kLockName[20];
}KcvCb;
KcvCb kcvCb;


int copyin(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes from the current thread's virtual address vaddr.
    // Return the number of bytes so read, or -1 if an error occors.
    // Errors can generally mean a bad virtual address was passed in.
    bool result;
    int n=0;			// The number of bytes copied in
    int *paddr = new int;

    while ( n >= 0 && n < len) {
      result = machine->ReadMem( vaddr, 1, paddr );
      while(!result) // FALL 09 CHANGES
	  {
   			result = machine->ReadMem( vaddr, 1, paddr ); // FALL 09 CHANGES: TO HANDLE PAGE FAULT IN THE ReadMem SYS CALL
	  }	
      
      buf[n++] = *paddr;
     
      if ( !result ) {
	//translation failed
	return -1;
      }

      vaddr++;
    }

    delete paddr;
    return len;
}

int copyout(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes to the current thread's virtual address vaddr.
    // Return the number of bytes so written, or -1 if an error
    // occors.  Errors can generally mean a bad virtual address was
    // passed in.
    bool result;
    int n=0;			// The number of bytes copied in

    while ( n >= 0 && n < len) {
      // Note that we check every byte's address
      result = machine->WriteMem( vaddr, 1, (int)(buf[n++]) );

      if ( !result ) {
	//translation failed
	return -1;
      }

      vaddr++;
    }

    return n;
}

void Create_Syscall(unsigned int vaddr, int len) {
    // Create the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  No
    // way to return errors, though...
    char *buf = new char[len+1];	// Kernel buffer to put the name in

    if (!buf) return;

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("%s","Bad pointer passed to Create\n");
	delete buf;
	return;
    }

    buf[len]='\0';

    fileSystem->Create(buf,0);
    delete[] buf;
    return;
}

int Open_Syscall(unsigned int vaddr, int len) {
    // Open the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  If
    // the file is opened successfully, it is put in the address
    // space's file table and an id returned that can find the file
    // later.  If there are any errors, -1 is returned.
    char *buf = new char[len+1];	// Kernel buffer to put the name in
    OpenFile *f;			// The new open file
    int id;				// The openfile id

    if (!buf) {
	printf("%s","Can't allocate kernel buffer in Open\n");
	return -1;
    }

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("%s","Bad pointer passed to Open\n");
	delete[] buf;
	return -1;
    }

    buf[len]='\0';

    f = fileSystem->Open(buf);
    delete[] buf;

    if ( f ) {
	if ((id = currentThread->space->fileTable.Put(f)) == -1 )
	    delete f;
	return id;
    }
    else
	return -1;
}

void Write_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one. For disk files, the file is looked
    // up in the current address space's open file table and used as
    // the target of the write.
    
    char *buf;		// Kernel buffer for output
    OpenFile *f;	// Open file for output

    if ( id == ConsoleInput) return;
    
    if ( !(buf = new char[len]) ) {
	printf("%s","Error allocating kernel buffer for write!\n");
	return;
    } else {
        if ( copyin(vaddr,len,buf) == -1 ) {
	    printf("%s","Bad pointer passed to to write: data not written\n");
	    delete[] buf;
	    return;
	}
    }

    if ( id == ConsoleOutput) {
      for (int ii=0; ii<len; ii++) {
	printf("%c",buf[ii]);
      }

    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    f->Write(buf, len);
	} else {
	    printf("%s","Bad OpenFileId passed to Write\n");
	    len = -1;
	}
    }

    delete[] buf;
}

int Read_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one.    We reuse len as the number of bytes
    // read, which is an unnessecary savings of space.
    char *buf;		// Kernel buffer for input
    OpenFile *f;	// Open file for output

    if ( id == ConsoleOutput) return -1;
    
    if ( !(buf = new char[len]) ) {
	printf("%s","Error allocating kernel buffer in Read\n");
	return -1;
    }

    if ( id == ConsoleInput) {
      //Reading from the keyboard
      scanf("%s", buf);

      if ( copyout(vaddr, len, buf) == -1 ) {
	printf("%s","Bad pointer passed to Read: data not copied\n");
      }
    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    len = f->Read(buf, len);
	    if ( len > 0 ) {
	        //Read something from the file. Put into user's address space
  	        if ( copyout(vaddr, len, buf) == -1 ) {
		    printf("%s","Bad pointer passed to Read: data not copied\n");
		}
	    }
	} else {
	    printf("%s","Bad OpenFileId passed to Read\n");
	    len = -1;
	}
    }

    delete[] buf;
    return len;
}

void Close_Syscall(int fd) {
    // Close the file associated with id fd.  No error reporting.
    OpenFile *f = (OpenFile *) currentThread->space->fileTable.Remove(fd);

    if ( f ) {
      delete f;
    } else {
      printf("%s","Tried to close an unopen file\n");
    }
}


// Kernel_Thread is the actual execution stream associated with each thread
void Kernel_Thread(unsigned int vaddr)
{
	forkLock -> Acquire();
	machine -> WriteRegister(PCReg, vaddr);
	machine -> WriteRegister(NextPCReg, vaddr + 4);
	
	// need to do the memory re-jig here. We are creating a new thread and hence we will allocate the new space to the stack at the end on-demand.
	int stackTop = currentThread -> space -> ResizePageTable();
	
	// copy back the page table to machine
	currentThread -> space -> RestoreState();
	
	//copy the virtual address of the stack top to the StackReg
	machine -> WriteRegister(StackReg, stackTop - 16);
	forkLock -> Release();
	
	machine -> Run();
}

void Fork_Syscall(unsigned int vaddr, unsigned int vaddrName)
{

	char *threadName;		// buffer for output

	if ( !(threadName = new char[NameSize]) ) {
		DEBUG('x',"\nError allocating kernel buffer for write!");
		return;
	} 
	else 
	{
		if ( copyin(vaddrName,NameSize,threadName) == -1 ) 
		{
			DEBUG('x', "\nBad pointer passed to to write: data not written");
			delete[] threadName;
			return;
		}
	}
	
	//find the parent process to which the new thread will belong to
	int parentProcessID = currentThread -> GetParentProcess();

	Process *parentProc = processTable[parentProcessID];
	/// increase the total number of threads forked
	totalFork++;

	if (totalFork > MAX_THREAD) // check if we have overrun the number of threads supported at a time
	{
		DEBUG('x', "\nYou have forked too many new threads. No more are allowed.");
		return;
	}
	else
	{
		//create a new thread which will be associated with the new execution stream
		Thread *newThread = new Thread(threadName);
		delete []threadName;
		// allocate the address space to the new kernel thread. This will be the address space of the parent process and hence will be copied from  

		newThread -> space = currentThread -> space;
		// assign the new thread to the parent process
		DEBUG('x',"A new thread was created; It is named - %s\n",newThread->getName());
		newThread -> AssignProcess(parentProcessID);
		//increment the childcount of the parent process
		parentProc -> IncrementChildThreadCount();
		// create a kernel level thread. pass the virtual address to the kernel thread
		newThread -> Fork((VoidFunctionPtr) Kernel_Thread, vaddr);
	}

}

//create the execution stream of a new process
void Exec_Thread(int inner)
{
	// copy the stack start of the main thread of the process to the thread control block of the main thread
	currentThread -> space -> SetDefaultStack();

	//initialize the registers for the new execution and copy the pageTable of the process to the machine
	currentThread -> space -> InitRegisters();
	currentThread -> space -> RestoreState();
	
	machine -> Run();
}

// create a new process
SpaceId Exec_Syscall(unsigned int vaddr, int length)
{
	char *buf = new char[length + 1];

	int id;

	if(copyin(vaddr, length, buf) == -1)
	{
		DEBUG('e',"\nBad pointer passed ");
		return -1;
	}

	if(!buf)
	{
		DEBUG('e',"\nCan't allow kernel buffer in Exec\n");
		return -1;
	}

	buf[length] = '\0';

	// open the executable file and start reading it 
	OpenFile *fd = fileSystem -> Open(buf);

	if(fd == NULL)
	{
		DEBUG('e',"\nFailure to open the executable file.\n");
		return -1;
	}

	//create address space of the process
	AddrSpace *newProcessSpace = new AddrSpace(fd);

	// create a new entry in the process table for this process
	Process *proc = new Process(newProcessSpace);

	int nextProcessID = proc -> GetProcessID();

	if(nextProcessID == -1)
	{
		return -1;
	}

	delete fd;

	int parentProcessID = currentThread -> GetParentProcess();

	proc -> SetParentProcess(parentProcessID);

	processTable[nextProcessID] = proc;
	// create a main thread for the process and allocate the space to it

	char *name = new char[30];

	sprintf(name, "ProcessMainThread%d", ProcessCount);

	Thread *processMainThread = new Thread(name);
	
	// assign the new address space created to the new process
	processMainThread -> space = newProcessSpace;
	proc -> IncrementChildThreadCount();

	// assign the thread to the process
	processMainThread -> AssignProcess(nextProcessID);

	// get the reference of the parent process as we need to add details to it as well.
	Process *parentProcess = processTable[parentProcessID];

	//copy the reference of the child process
	parentProcess -> IncrementChildProcessCount(nextProcessID);

	// fork the new thread
	processMainThread -> Fork((VoidFunctionPtr) Exec_Thread, 0);

	return nextProcessID;

}



void newLine_Syscall()
{
	printLock->Acquire();
	printf("\n");
	printLock->Release();
}

void printTab_Syscall()
{
	printf("\t");
}

AddrSpace* currentProcessAddrspace()
{
  return (AddrSpace*) (currentThread->space);
}


void Print_Syscall(unsigned int vaddr, int int1, int int2, int int3) //we need to put a null byte at end of array to be printed when we call this function
{
    // Write the given character buffer input to the console - the rest 
    // are numbers that we have to print
    // 
    
	//Old working
	
 	
	printLock->Acquire();

	char *buf;		// Kernel buffer for output
	
	
	if ( !(buf = new char[PrintLength]) ) {
		printf("%s","Error allocating kernel buffer for write!\n");
		return;
		} 
		else 
		{
			if ( copyin(vaddr,PrintLength,buf) == -1 ) 
			{
			printf("%s","Bad pointer passed to to write: data not written\n");
			delete[] buf;
			return;
			}
		}

	int numbersPrinted = 0;
	int ii=0;
	
    for (ii=0;(ii<PrintLength && buf[ii]!='\0'); ii++) 
	{
		if (buf[ii]=='%')
		{
			if ( ((ii + 1)<PrintLength)  && (buf[ii+1] == 'd'))
			{
				if (numbersPrinted < 3)
				{	
					numbersPrinted++;
					
					if (numbersPrinted==1)
						printf("%d",int1);
					else if (numbersPrinted==2)
						printf("%d",int2);
					else if (numbersPrinted==3)
						printf("%d",int3);

					ii++;//this is to skip next letter in char array which would be a d and we've already taken care of that
					continue;
				}
				else
				{
					printf("%c", buf[ii]); //if we have already had 3 numbers printed we just print %d as is - ie %d
				}
			}
				
		}
		else if ((buf[ii] == '\\') && (buf[ii+1] == 'n' ) && (ii+1 < PrintLength))
		{
			printf("\n");
			ii++; //skip next
		}		
		else if ((buf[ii] == '\\') && (buf[ii+1] == 't' ) && (ii+1 < PrintLength))
		{
			printf("\t");
			ii++; //skip next
		}		
		else
		{
			printf("%c",buf[ii]);
		}
    
	
	}
	delete[] buf;

   	printLock->Release();

	return;
	
}

int Scan_Syscall(unsigned int vaddr) {
	// This system scans and returns the number that has been input
	// We only scan 2 digits as that is all we need to determine input
	char *buf;		// Kernel buffer for input
	int ii;
	int number=-1;

	if ( !(buf = new char[ScanLength]) ) {
		DEBUG('x', "\nError allocating kernel buffer for Input");
		return -1;
	}

	//Reading from the keyboard
	scanf("%1s", buf);

	if ( copyout(vaddr, ScanLength, buf) == -1 ) {
		DEBUG('x',"\nBad Input: Data not scanned");
	}

	//printf("\nYour input was %c",buf[0]);


	number=(buf[0] - '0');

	//printf("\nNumber is now %d",number);	
	delete[] buf;

	return number;
}


void Print2_Syscall(unsigned int vaddr, int int1, int int2, int int3) //the null character is passed into here
{
	// Write the given character buffer input to the console - the rest 
	// are numbers that we have to print
	// 

	//We will be printing in-parts one segment, then numbers and the segment again - We need to mantain lock

	printLock->Acquire();

	char *buf;		// Kernel buffer for output
	int len = 100;

	if ( !(buf = new char[len]) ) {
		DEBUG('x',"\nError allocating kernel buffer for write!");
		return;
	} 
	else 
	{
		if ( copyin(vaddr,len,buf) == -1 ) 
		{
			DEBUG('x',"\nBad pointer passed to to write: data not written");
			delete[] buf;
			return;
		}
	}

	int numbersPrinted = 0;
	int ii=0;

	for (ii=0;ii<len; ii++) 
	{
		if (buf[ii]=='%')
		{
			if ( ((ii + 1)<len)  && (buf[ii+1] == 'd'))
			{
				if (numbersPrinted < 3)
				{	
					numbersPrinted++;

					if (numbersPrinted==1)
						printf("%d",int1);
					else if (numbersPrinted==2)
						printf("%d",int2);
					else if (numbersPrinted==3)
						printf("%d",int3);

					ii++;//this is to skip next letter in char array which would be a d and we've already taken care of that
					continue;
				}
				else
				{
					printf("%c", buf[ii]); //if we have already had 3 numbers printed we just print %d as is - ie %d
				}
			}

		}
		printf("%c",buf[ii]);

	}



	delete[] buf;
	printLock->Release();
	//return len;
	return;
}

void initializeKlCb()
{
    int i = 0;
    sprintf(klCb.kLockName, "%s","KernelLock\0");
    klCb.kLock = new Lock(klCb.kLockName);
    klCb.nextLockLocation = 0;
    for(i=0; i< MAX_LOCKS; i++)
    {
        klCb.kl[i].lock = NULL;
        klCb.kl[i].inUse = NO;
		klCb.kl[i].lock = NULL;
		klCb.kl[i].currentAddrSpace = NULL;
        klCb.kl[i].isGoingToBeUsed = 0;
        klCb.kl[i].isGoingToBeDeleted = NO;
    }
}

void initializeKcvCb()
{
    int i = 0;
    sprintf(klCb.kLockName, "%s","KernelLock\0");
//   kcvCb.kLockName = {'K','e','r','n','e','l','L','o','c','k','\0','\0'};
    kcvCb.kLock = new Lock(kcvCb.kLockName);
    kcvCb.nextCVLocation = 0;
    for(i=0; i< MAX_CVS; i++)
    {
        kcvCb.kcv[i].cv = NULL;
        kcvCb.kcv[i].inUse = NO;
		kcvCb.kcv[i].cv = NULL;
		kcvCb.kcv[i].currentAddrSpace = NULL;
        kcvCb.kcv[i].isGoingToBeDeleted = NO;
        kcvCb.kcv[i].isGoingToBeUsed = 0;
    }
}

int findFreeLock()
{
    int i = 0;
    for( i=0; i< MAX_LOCKS; i++)
    {
        if( klCb.kl[i].lock == NULL &&  klCb.kl[i].currentAddrSpace == NULL && klCb.kl[i].inUse == NO)
        {
            return i;
        }
    }
    if( i == MAX_LOCKS )
    {
        klCb.nextLockLocation = MAX_LOCKS;
	DEBUG('x',"\nLocks are over");
        return -1;
    }
}

int findFreeCV()
{
    int i = 0;
    for( i=0; i< MAX_CVS; i++)
    {
        if( kcvCb.kcv[i].cv == NULL &&  kcvCb.kcv[i].currentAddrSpace == NULL && kcvCb.kcv[i].inUse == NO)
        {
            return i;
        }
    }
    if( i == MAX_CVS )
    {
        kcvCb.nextCVLocation = MAX_CVS;
	DEBUG('x',"\nCVs are over");
        return -1;
    }
}

// reclaim the ccondition variables allocated tko the exiting process
void freeCVHeldByProcess(AddrSpace *processAddrSpace)
{
    if(currentProcessAddrspace() == processAddrSpace)

    {
        int i = 0;
        for(i=0; i< MAX_CVS; i++)
        {
            if(kcvCb.kcv[i].currentAddrSpace == processAddrSpace )
            {
                //DESTRUCTOR 
		delete kcvCb.kcv[i].cv;
                kcvCb.kcv[i].cv = NULL;
                kcvCb.kcv[i].currentAddrSpace = NULL;
                kcvCb.kcv[i].inUse = NO;
                kcvCb.nextCVLocation = i;

                kcvCb.kcv[i].isGoingToBeUsed = 0;
                kcvCb.kcv[i].isGoingToBeDeleted = NO;
            }
        }
    }
    else
    {
	DEBUG('x',"\nOnly current process can free its resources");
    }
}

// claim all the locks that are held by the exiting process
void freeLockHeldByProcess(AddrSpace *processAddrSpace)
{
    if(currentProcessAddrspace() == processAddrSpace)

    {
        int i = 0;
        for(i=0; i< MAX_LOCKS; i++)
        {
            if(klCb.kl[i].currentAddrSpace == processAddrSpace )
            {
                //DESTRUCTOR
                delete klCb.kl[i].lock;
                //klCb.kl[i].lock->~Lock();
                
                klCb.kl[i].lock = NULL;

                klCb.kl[i].currentAddrSpace = NULL;
                klCb.kl[i].inUse = NO;
                klCb.nextLockLocation = i;

                klCb.kl[i].isGoingToBeUsed = 0;
                klCb.kl[i].isGoingToBeDeleted = NO;
            }
        }
    }
    else
    {
	DEBUG('x',"\nOnly current process can free its resources");
    }
    kcvCb.kLock->Release();
}

// The functions will check if next
// location of lock  (which is incremented by 1 everytime a new one has been created) has reached MAXIMUM allowed 
// value or not (MAX_LOCKS). If it hasnt and the next location is valid number (ie a whole number), it shall
// proceed to check if associated lock is in 
//	initalized state.
int CreateLock_Syscall(unsigned int vaddr)
{

	char *lockName;		// buffer for name

	if ( !(lockName = new char[NameSize]) ) {
		DEBUG('x',"\nError allocating kernel buffer for write!");
		return -2;
	} else {
		if ( copyin(vaddr,NameSize,lockName) == -1 ) {
			DEBUG('x',"\nBad pointer passed to to write: data not written");
			delete[] lockName;
			return -2;
		}
	}

	int lockId = 0;
	klCb.kLock->Acquire();
	if(klCb.nextLockLocation < MAX_LOCKS && klCb.nextLockLocation >= 0)
	{
		if( klCb.kl[klCb.nextLockLocation].lock == NULL &&  klCb.kl[klCb.nextLockLocation].currentAddrSpace == NULL && klCb.kl[klCb.nextLockLocation].inUse == NO)
		{

			klCb.kl[klCb.nextLockLocation].lock = new Lock(lockName);
			delete[] lockName;
			klCb.kl[klCb.nextLockLocation].currentAddrSpace = currentProcessAddrspace();
			klCb.kl[klCb.nextLockLocation].inUse = YES;
			DEBUG('l',"\nLockId value is %d", lockId);
			lockId = klCb.nextLockLocation;
			DEBUG('l',"\nNew LockId value is %d", lockId);
			klCb.kl[lockId].isGoingToBeUsed = 0;
			klCb.kl[lockId].isGoingToBeDeleted = NO;
			DEBUG('x'," Lock_%d is created and is named %s.\n", lockId, klCb.kl[lockId].lock->getName());
			klCb.nextLockLocation = findFreeLock();

		}
		else
		{
			klCb.nextLockLocation = findFreeLock();
			klCb.kl[klCb.nextLockLocation].lock = new Lock(lockName);
			delete[] lockName;
			klCb.kl[klCb.nextLockLocation].currentAddrSpace = currentProcessAddrspace();
			klCb.kl[klCb.nextLockLocation].inUse = YES;
			lockId = klCb.nextLockLocation;
			klCb.kl[lockId].isGoingToBeUsed = 0;
			klCb.kl[lockId].isGoingToBeDeleted = NO;
			DEBUG('x'," Lock_%d is created and is named %s.\n", lockId, klCb.kl[lockId].lock->getName());
			klCb.nextLockLocation = findFreeLock();
		}
	}
	else
	{
		DEBUG('x',"All locks are occupied. Need to wait.\n");
		lockId = -1;
	}
	klCb.kLock->Release();
	return lockId;
}


//The user program will send an integer as
//	argument to this system call. This syscall will first acquire a kernel lock for manipulation of the global data structure
//		KlCb. The lock associated with the lockId gets acquired and associated with the process 
void AcquireLock_Syscall(int lockId)
{

	if (lockId < 0 || lockId >= MAX_LOCKS) 
	{
		DEBUG('x',"Bad Acquire Request: No such lock number exists \n");
		return ;
	}


	klCb.kLock->Acquire();
	if(klCb.kl[lockId].inUse == YES && klCb.kl[lockId].currentAddrSpace == currentProcessAddrspace() && klCb.kl[lockId].isGoingToBeDeleted == NO)
	{
		klCb.kl[lockId].isGoingToBeUsed++;

		klCb.kLock->Release(); 
		DEBUG('x',"\n Lock_%d is now acquired.", lockId);
		klCb.kl[lockId].lock->Acquire();

		return;

	}
	else if(klCb.kl[lockId].inUse == NO )
	{
		DEBUG('x',"\n Bad Acquire Request: Lock_%d is not created yet.", lockId);
	}
	else if(klCb.kl[lockId].currentAddrSpace != currentProcessAddrspace() )
	{
		DEBUG('x', "\n Bad Acquire Request: Lock_%d is not held by current process.", lockId);
	}
	else if(klCb.kl[lockId].isGoingToBeDeleted == YES)
	{
		DEBUG('x',"\n Bad Acquire Request: Lock_%d is going to be deleted so cannot be acquired.", lockId);
	}
	klCb.kLock->Release();
}


//On successful release, our lock::release
//function in thread directory returns a 1. Once we get that we decrement isGoingToBeUsed counter.
void ReleaseLock_Syscall(int lockId)
{

	if (lockId < 0 || lockId >= MAX_LOCKS) 
	{
		DEBUG('x',"\nBad Release Request: No such lock number exists");
		return ;
	}


	klCb.kLock->Acquire();
	if(klCb.kl[lockId].inUse == YES && klCb.kl[lockId].currentAddrSpace == currentProcessAddrspace())
	{
		if(klCb.kl[lockId].lock->Release() == 1)
		{
			klCb.kl[lockId].isGoingToBeUsed--;
		}
		if(klCb.kl[lockId].isGoingToBeDeleted == YES && klCb.kl[lockId].isGoingToBeUsed <= 0)
		{
			delete klCb.kl[lockId].lock;
			klCb.kl[lockId].lock = NULL;
			klCb.kl[lockId].currentAddrSpace = NULL;
			klCb.kl[lockId].inUse = NO;
			klCb.nextLockLocation = lockId;
			klCb.kl[lockId].isGoingToBeUsed = 0;
			klCb.kl[lockId].isGoingToBeDeleted = NO;
			DEBUG('x'," Lock_%d is now deleted.\n", lockId);
		}
		klCb.kLock->Release();
		return;

	}
	else if(klCb.kl[lockId].inUse == NO )
	{
		DEBUG('x',"\n Bad Release Request: Lock_%d is not created yet.", lockId);
	}
	else if(klCb.kl[lockId].currentAddrSpace != currentProcessAddrspace() )
	{
		DEBUG('x',"\n Bad Release Request: Lock_%d is not held by current process.", lockId);
	}
	klCb.kLock->Release();
}


// The function destroys the lock that has been created .
//If the GoingToBeUsed value is non-zero, ie more Acquire requests than executed release requests, we dont delete but simply save
//the message by setting the ToBeDeleted value.
void DestroyLock_Syscall(int lockId)
{
	if (lockId < 0 || lockId >= MAX_LOCKS) 
	{
		DEBUG('x',"\nBad Destroy Request: No such lock number exists.");
		return ;
	}

	klCb.kLock->Acquire();
	if(klCb.kl[lockId].inUse == YES && klCb.kl[lockId].currentAddrSpace == currentProcessAddrspace())
	{
		if(klCb.kl[lockId].isGoingToBeUsed > 0) 
		{
			klCb.kl[lockId].isGoingToBeDeleted = YES;
			DEBUG('x'," Lock_%d is not deleted for now as it is in use.\n", lockId);
		}
		else
		{
			delete klCb.kl[lockId].lock;
			klCb.kl[lockId].lock = NULL;
			klCb.kl[lockId].lock = NULL;
			klCb.kl[lockId].currentAddrSpace = NULL;
			klCb.kl[lockId].inUse = NO;
			klCb.nextLockLocation = lockId;
			klCb.kl[lockId].isGoingToBeUsed = 0;
			klCb.kl[lockId].isGoingToBeDeleted = NO;
			DEBUG('x'," Lock_%d is now deleted.\n", lockId);
		}
	}
	else if(klCb.kl[lockId].inUse == NO )
	{
		DEBUG('x', "\n Bad Destroy Request: Lock_%d is not created yet.", lockId);
	}
	else if(klCb.kl[lockId].currentAddrSpace != currentProcessAddrspace() )
	{
		DEBUG('x', "\n Bad Destroy Request: Lock_%d is not held by current process.", lockId);
	}
	klCb.kLock->Release();
}


// The functions will check if next
// location of CV  (which is incremented by 1 everytime a new one has been created) has reached MAXIMUM allowed 
// value or not (MAX_CV). If it hasnt and the next location is valid number (ie a whole number), it shall
// proceed to check if associated CV is in 
//	initalized state.
int CreateCV_Syscall(unsigned int vaddr)
{
	char *cvName;		// Kernel buffer for output

	if ( !(cvName = new char[NameSize]) ) {
		DEBUG('x',"\nError allocating kernel buffer for write!");
		return -2;
	} else {
		if ( copyin(vaddr,NameSize,cvName) == -1 ) {
			DEBUG('x',"\nBad pointer passed to to write: data not written");
			delete[] cvName;
			return -2;
		}
	}

	int cvId = 0;
	kcvCb.kLock->Acquire();
	if(kcvCb.nextCVLocation < MAX_CVS && kcvCb.nextCVLocation >= 0)
	{
		if( kcvCb.kcv[kcvCb.nextCVLocation].cv == NULL &&  kcvCb.kcv[kcvCb.nextCVLocation].currentAddrSpace == NULL && kcvCb.kcv[kcvCb.nextCVLocation].inUse == NO)
		{
			kcvCb.kcv[kcvCb.nextCVLocation].cv = new Condition(cvName);
			delete[] cvName;
			kcvCb.kcv[kcvCb.nextCVLocation].currentAddrSpace = currentProcessAddrspace();
			kcvCb.kcv[kcvCb.nextCVLocation].inUse = YES;
			cvId = kcvCb.nextCVLocation;
			kcvCb.kcv[cvId].isGoingToBeUsed = 0;
			kcvCb.kcv[cvId].isGoingToBeDeleted = NO;
			DEBUG('x'," CV_%d is created and is named %s.\n", cvId, kcvCb.kcv[cvId].cv->getName());
			kcvCb.nextCVLocation = findFreeCV();

		}
		else
		{
			kcvCb.nextCVLocation = findFreeCV();
			kcvCb.kcv[kcvCb.nextCVLocation].cv = new Condition(cvName);
			delete[] cvName;
			kcvCb.kcv[kcvCb.nextCVLocation].currentAddrSpace = currentProcessAddrspace();
			kcvCb.kcv[kcvCb.nextCVLocation].inUse = YES;
			cvId = kcvCb.nextCVLocation;
			kcvCb.kcv[cvId].isGoingToBeUsed = 0;
			kcvCb.kcv[cvId].isGoingToBeDeleted = NO;
			DEBUG('x'," CV_%d is created.\n", cvId);
			kcvCb.nextCVLocation = findFreeCV();
		}
	}
	else
	{
		DEBUG('x'," All CVs are occupied. Need to wait.\n");
		cvId = -1;
	}
	kcvCb.kLock->Release();
	return cvId;
}

// routine used to call wait on the condition variable. The lock ID and the condition variable id is passed to the routine
void WaitCV_Syscall(int lockId, int cvId)
{
	if (lockId < 0 || lockId >= MAX_LOCKS) 
	{
		DEBUG('x',"\nBad Wait Request: No such lock number exists");
		return ;
	}

	if (cvId < 0 || cvId >= MAX_CVS) 
	{
		DEBUG('x',"\nBad Wait Request: No such cv number exists");
		return ;
	}

	kcvCb.kLock->Acquire();
	if(klCb.kl[lockId].inUse == NO || klCb.kl[lockId].currentAddrSpace != currentProcessAddrspace())
	{
		DEBUG('x',"\nLock_%d passed is invalid.", lockId);
	}	

	if(kcvCb.kcv[cvId].inUse == YES && kcvCb.kcv[cvId].currentAddrSpace == currentProcessAddrspace() && kcvCb.kcv[cvId].isGoingToBeDeleted == NO)
	{
		kcvCb.kcv[cvId].isGoingToBeUsed++;
		DEBUG('x',"\n CV_%d is going on wait for Lock_%d.", cvId, lockId);
		kcvCb.kLock->Release();
		kcvCb.kcv[cvId].cv->Wait(klCb.kl[lockId].lock);
		DEBUG('x',"\n CV_%d is out of wait for Lock_%d.", cvId, lockId);
		return;

	}
	else if(kcvCb.kcv[cvId].inUse == NO )
	{
		DEBUG('x',"\n Bad Wait Request: CV_%d is not created yet.", cvId);
	}
	else if(kcvCb.kcv[cvId].currentAddrSpace != currentProcessAddrspace() )
	{
		DEBUG('x',"\n Bad Wait Request: CV_%d is not held by current process.", cvId);
	}
	else if(kcvCb.kcv[cvId].isGoingToBeDeleted == YES)
	{
		DEBUG('x',"\n CV_%d is going to be deleted soon.", cvId);
	}
	kcvCb.kLock->Release();
}


// routine used to call signal on a condition variable. The lock id and the condition variable id is passed to the function
void SignalCV_Syscall(int lockId, int cvId)
{


	if (lockId < 0 || lockId >= MAX_LOCKS) 
	{
		DEBUG('x',"\nBad Signal Request: No such lock number exists.");
		return ;
	}

	if (cvId < 0 || cvId >= MAX_CVS) 
	{
		DEBUG('x',"Bad Signal Request: No such CV number exists.");
		return ;
	}

	kcvCb.kLock->Acquire();

	if(klCb.kl[lockId].inUse == NO || klCb.kl[lockId].currentAddrSpace != currentProcessAddrspace())
	{
		DEBUG('x'," Lock_%d passed is invalid.\n", lockId);
	}	


	if(kcvCb.kcv[cvId].inUse == YES && kcvCb.kcv[cvId].currentAddrSpace == currentProcessAddrspace())
	{
		if(kcvCb.kcv[cvId].cv->Signal(klCb.kl[lockId].lock) == 1)
		{
			kcvCb.kcv[cvId].isGoingToBeUsed--;
		}
		DEBUG('x'," CV_%d is now Signalling on Lock_%d.\n", cvId, lockId);
		if(kcvCb.kcv[cvId].isGoingToBeDeleted == YES && kcvCb.kcv[cvId].isGoingToBeUsed <= 0)
		{
			//DESTRUCTOR
			delete kcvCb.kcv[cvId].cv;
			kcvCb.kcv[cvId].cv = NULL;
			kcvCb.kcv[cvId].currentAddrSpace = NULL;
			kcvCb.kcv[cvId].inUse = NO;
			kcvCb.nextCVLocation = cvId;
			kcvCb.kcv[cvId].isGoingToBeUsed = 0;
			kcvCb.kcv[cvId].isGoingToBeDeleted = NO;
			DEBUG('x'," CV_%d is now deleted.\n", cvId);
		}
		kcvCb.kLock->Release();
		return;

	}
	else if(kcvCb.kcv[cvId].inUse == NO )
	{
		DEBUG('x',"\n Bad Signal Request: CV_%d is not created yet.", cvId);
	}
	else if(kcvCb.kcv[cvId].currentAddrSpace != currentProcessAddrspace() )
	{
		DEBUG('x', "\n Bad Signal Request: CV_%d is not held by current process.", cvId);
	}
	kcvCb.kLock->Release();
}

// used to call broadcast on a condition variable. The lock id and cv id are passed to the the function. 

void BroadcastCV_Syscall(int lockId, int cvId)
{
	if (lockId < 0 || lockId >= MAX_LOCKS) 
	{
		DEBUG('x',"\nBad Broadcast Request: No such lock number exists.");
		return ;
	}

	if (cvId < 0 || cvId >= MAX_CVS) 
	{
		DEBUG('x',"\nBad Broadcast Request: No such CV number exists");
		return ;
	}

	kcvCb.kLock->Acquire();
	if(klCb.kl[lockId].inUse == NO || klCb.kl[lockId].currentAddrSpace != currentProcessAddrspace())
	{
		//printf(" Lock %d passed is invalid.\n", lockId);
	}	

	if(kcvCb.kcv[cvId].inUse == YES && kcvCb.kcv[cvId].currentAddrSpace == currentProcessAddrspace())
	{
		kcvCb.kcv[cvId].cv->Broadcast(klCb.kl[lockId].lock);
		kcvCb.kcv[cvId].isGoingToBeUsed = 0;
		DEBUG('x'," CV_%d is now Broadcasting on Lock_%d\n", cvId, lockId);
		if(kcvCb.kcv[cvId].isGoingToBeDeleted == YES)
		{
			//DESTRUCTOR 
			delete kcvCb.kcv[cvId].cv;
			kcvCb.kcv[cvId].cv = NULL;
			kcvCb.kcv[cvId].currentAddrSpace = NULL;
			kcvCb.kcv[cvId].inUse = NO;
			kcvCb.nextCVLocation = cvId;
			kcvCb.kcv[cvId].isGoingToBeUsed = 0;
			kcvCb.kcv[cvId].isGoingToBeDeleted = NO;
			DEBUG('x'," CV_%d is now deleted.\n", cvId);
		}
		kcvCb.kLock->Release();
		return;

	}
	else if(kcvCb.kcv[cvId].inUse == NO )
	{
		DEBUG('x',"\n Bad Broadcast Request: CV_%d is not created yet.", cvId);
	}
	else if(kcvCb.kcv[cvId].currentAddrSpace != currentProcessAddrspace() )
	{
		DEBUG('x',"\n Bad Broadcast Request: CV_%d is not held by current process.", cvId);
	}
	kcvCb.kLock->Release();
}

// routine used to destroy the condition variable. The id of the condition variable is passed to the function. 
// The validation is done to check if the CVID passed is valid and then the memory allocated to the condition variable is reclaimed
void DestroyCV_Syscall(int cvId)
{

	if (cvId < 0 || cvId >= MAX_CVS) 
	{
		DEBUG('x',"\nBad Destroy Request: No such CV number exists");
		return ;
	}


	kcvCb.kLock->Acquire();
	if(kcvCb.kcv[cvId].inUse == YES && kcvCb.kcv[cvId].currentAddrSpace == currentProcessAddrspace())
	{
		if(kcvCb.kcv[cvId].isGoingToBeUsed > 0)
		{
			kcvCb.kcv[cvId].isGoingToBeDeleted = YES;
			DEBUG('x'," CV_%d is not deleted for now as it is in use.\n", cvId);
		}
		else
		{
			delete kcvCb.kcv[cvId].cv;
			kcvCb.kcv[cvId].cv = NULL;
			kcvCb.kcv[cvId].currentAddrSpace = NULL;
			kcvCb.kcv[cvId].inUse = NO;
			kcvCb.nextCVLocation = cvId;
			kcvCb.kcv[cvId].isGoingToBeUsed = 0;
			kcvCb.kcv[cvId].isGoingToBeDeleted = NO;
			DEBUG('x'," CV_%d is deleted.\n",  cvId);
		}
	}
	else if(kcvCb.kcv[cvId].inUse == NO )
	{
		DEBUG('x',"\n Bad Request: CV_%d is not created yet.", cvId);
	}
	else if(kcvCb.kcv[cvId].currentAddrSpace != currentProcessAddrspace() )
	{
		DEBUG('x',"\n Bad Request: CV_%d is not held by current process.", cvId);
	}
	kcvCb.kLock->Release();
}

// the memory clear function will reclaim the pages that will become free. The option indicates whether the whole memory related to the process needs to
// be cleared or just the stack space for the current thread 
void MemClear(int option)
{
	// if option is 1 then we will clear the entire memory allocated to the process
	// else we just need to clear the stack pages allocated to the thread
	
	if(option == 1) // main thread of the process has exited
	{
		// restore the page table so that machine copies the current page table enteries
		
		currentThread -> space -> ReclaimPhysicalPages();
	}
	else // only the stack pages need to be deleted
	{
		currentThread -> space -> RemoveThreadStack();
	}
}


// create a routine to make the process exit
void Exit_Syscall(unsigned int status)
{
	DEBUG('e', "\n%s Entered exit\n\n", currentThread -> getName());
	int processID = currentThread -> GetParentProcess();
	Process *proc = processTable[processID];

	proc -> DecrementChildThreadCount();

	if(ProcessCount == 1) // this is the last process of the 
	{
		if(proc -> ChildThreadCount() == 0)
		{ // this is the last thread of the last executing process
			ProcessCount--;
			DEBUG('e',"\nLast process Exiting\n");
			MemClear(1); // clear the memory for the process
			freeCVHeldByProcess(currentThread->space); // reclaim kernel resources
			freeLockHeldByProcess(currentThread->space);
			interrupt -> Halt();
		}

	}
	else
	{
		// this is not the last executing process
		// do two things, Check if the parent process has more threads
		// or if it is the last thread of the process

		int childCount = proc -> ChildThreadCount();

		if(childCount > 0) // there are still mor children of the process
		{
			MemClear(0); // reclaim the pages for stack for this thread
		}
		else if(childCount == 0) // last child of this process
		{
			ProcessCount--;
			DEBUG('e',"\n Clearing the memory for the thread %x", currentThread);
			MemClear(1); // reclaim all the pages allocated to the process

			// ACHINTYA to write the logic to reclaim the synchronization Params
			freeCVHeldByProcess(currentThread->space);// reclaim kernel resources, i.e Locks and CVs
			freeLockHeldByProcess(currentThread->space);
		}

		DEBUG('e',"\n%x completed exit sys call", currentThread);

	}

	currentThread -> Finish();

	DEBUG('e', "\nThe last thread %s belonged to process %d", currentThread -> getName(), currentThread -> GetParentProcess());
	DEBUG('e', "\nThe child count of process %d is %d", processID, proc -> ChildThreadCount());

}



void ExceptionHandler(ExceptionType which) {
    int type = machine->ReadRegister(2); // Which syscall?
    int rv=0; 	// the return value from a syscall

    if ( which == SyscallException ) {
	    switch (type) {
		    default:
			    DEBUG('a', "Unknown syscall - shutting down.\n");

		    case SC_Halt:
			    DEBUG('a', "Shutdown, initiated by user program.\n");
			    interrupt->Halt();
			    break;

		    case SC_Exit:
			    DEBUG('a', "Exit syscall.\n");		
			    //currentThread->Finish();
			    Exit_Syscall(machine -> ReadRegister(4));		
			    break;

		    case SC_Create:
			    DEBUG('a', "Create syscall.\n");
			    Create_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
			    break;
		    case SC_Open:
			    DEBUG('a', "Open syscall.\n");
			    rv = Open_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
			    break;
		    case SC_Write:
			    DEBUG('a', "Write syscall.\n");
			    Write_Syscall(machine->ReadRegister(4),
					    machine->ReadRegister(5),
					    machine->ReadRegister(6));
			    break;
		    case SC_Read:
			    DEBUG('a', "Read syscall.\n");
			    rv = Read_Syscall(machine->ReadRegister(4),
					    machine->ReadRegister(5),
					    machine->ReadRegister(6));
			    break;
		    case SC_Close:
			    DEBUG('a', "Close syscall.\n");
			    Close_Syscall(machine->ReadRegister(4));
			    break;

		    case SC_Fork:
			    DEBUG('a', "Fork syscall.\n");
			    Fork_Syscall(machine -> ReadRegister(4),
					    machine->ReadRegister(5));
			    break;

		    case SC_Exec:
			    DEBUG('a', "Exec syscall.\n");
			    SpaceId procID = Exec_Syscall(machine -> ReadRegister(4), 
					    machine->ReadRegister(5));
			    rv = (int) procID;
			    break;

		    case SC_Print:
			    DEBUG('a', "Print syscall.\n");
			    Print_Syscall(machine->ReadRegister(4),
					    machine->ReadRegister(5),
					    machine->ReadRegister(6),
					    machine->ReadRegister(7));
			    break;

		    case SC_Scan:
			    DEBUG('a', "Scan syscall.\n");		
			    rv=Scan_Syscall(machine->ReadRegister(4));
			    break;

		    case SC_Print2:
			    DEBUG('a', "Print-2 syscall.\n");
			    Print2_Syscall(machine->ReadRegister(4),
					    machine->ReadRegister(5),
					    machine->ReadRegister(6),
					    machine->ReadRegister(7));
			    break;

		    case SC_NewLine:
			    DEBUG('a',"New line syscall\n");
			    newLine_Syscall();
			    break;

		    case SC_PrintTab:
			    DEBUG('a',"Print tab syscall\n");
			    printTab_Syscall();
			    break;

		    case SC_Yield:
			    DEBUG('a', "Yield syscall.\n");
			    currentThread -> Yield();
			    break;

		    case SC_CreateLock:
			    DEBUG('a', "CreateLock syscall.\n");
			    rv = CreateLock_Syscall(machine->ReadRegister(4));
			    break;

		    case SC_AcquireLock:
			    DEBUG('a', "AcquireLock syscall.\n");
			    AcquireLock_Syscall(machine->ReadRegister(4));
			    break;

		    case SC_ReleaseLock:
			    DEBUG('a', "ReleaseLock syscall.\n");
			    ReleaseLock_Syscall(machine->ReadRegister(4));
			    break;

		    case SC_DestroyLock:
			    DEBUG('a', "DestroyLock syscall.\n");
			    DestroyLock_Syscall(machine->ReadRegister(4));
			    break;

		    case SC_CreateCV:
			    DEBUG('a', "CreateCV syscall.\n");
			    rv = CreateCV_Syscall(machine->ReadRegister(4));
			    break;

		    case SC_WaitCV:
			    DEBUG('a', "WaitCV syscall.\n");
			    WaitCV_Syscall(machine->ReadRegister(4),machine->ReadRegister(5));
			    break;

		    case SC_SignalCV:
			    DEBUG('a', "SignalCV syscall.\n");
			    SignalCV_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
			    break;

		    case SC_BroadcastCV:
			    DEBUG('a', "BroadcastCV syscall.\n");
			    BroadcastCV_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
			    break;

		    case SC_DestroyCV:
			    DEBUG('a', "DestroyCV syscall.\n");
			    DestroyCV_Syscall(machine->ReadRegister(4));
			    break;		

		    case SC_Random:
			    DEBUG('a', "Random syscall.\n");
			    rv = random();
			    break;


	    }

	    // Put in the return value and increment the PC
	    //printf("rv value is %d",rv);
	    machine->WriteRegister(2,rv);
	    machine->WriteRegister(PrevPCReg,machine->ReadRegister(PCReg));
	    machine->WriteRegister(PCReg,machine->ReadRegister(NextPCReg));
	    machine->WriteRegister(NextPCReg,machine->ReadRegister(PCReg)+4);
	    return;
    } else {
      cout<<"Unexpected user mode exception - which:"<<which<<"  type:"<< type<<endl;
      interrupt->Halt();
    }
}
