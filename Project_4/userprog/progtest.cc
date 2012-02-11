// progtest.cc 
//	Test routines for demonstrating that Nachos can load
//	a user program and execute it.  
//
//	Also, routines for testing the Console hardware device.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "console.h"
#include "addrspace.h"
#include "synch.h"
#include "string.h"
#include "Process.h"


#define QUANTUM 100


extern int failsafe;
//Project 4 - Extra credit
/*
*	This is a user program - a client.
*	Here we want this to run as a separate thread 
*	this funciton will keep on receiving the pings
*	that we want to use for checking in case of 
* 	the client dying - Mailbox 9 for client listens to this
*/

void Respond_Server()
{
	PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char buffer[MaxMailSize];	
	
	while (true)
	{
		//printf("\nThis client has clientPingMailBoxId as %d\n",clientPingMailBoxId);
		//printf("\nthe client can respond to server pings\n..");
		//We look for this ping message in this client's mail box number only
		postOffice -> Receive(clientPingMailBoxId,&inPktHdr, &inMailHdr, buffer);
		//printf("\n JASPR Received my ping request %s\n", buffer);
		currentThread -> Yield();
	}
}
//----------------------------------------------------------------------
// StartProcess
// 	Run a user program.  Open the executable, load it into
//	memory, and jump to it.
//----------------------------------------------------------------------
void
StartProcess(char *filename)
{
    OpenFile *executable = fileSystem->Open(filename);
    AddrSpace *space;

    if (executable == NULL) {
	printf("Unable to open file %s\n", filename);
	return;
    }
   
    space = new AddrSpace(executable);

    currentThread->space = space;
	
	Process *proc = new Process(space);
	
	int nextProcessID = proc -> GetProcessID();
	
	if(nextProcessID == -1)
	{	
		return;
	}
	
	processTable[nextProcessID] = proc;
	
	currentThread -> AssignProcess(nextProcessID);
	currentThread -> AssignMailboxId(1);

	proc -> SetMainThread(currentThread); 
	proc -> IncrementChildThreadCount();

    delete executable;			// close file

    space->InitRegisters();		// set the initial register values
    space->RestoreState();		// load page table register
	
	//Project 4 - Fork new thread for client receving pinging msg from server
	if (failsafe)
	{
		Thread *newThread = new Thread("receivePings");
		//newThread -> space = currentThread -> space;
		//newThread -> space = space;
		newThread -> Fork((VoidFunctionPtr) Respond_Server, 0);	
	}
	
    machine->Run();			// jump to the user progam
    ASSERT(FALSE);			// machine->Run never returns;
					// the address space exits
					// by doing the syscall "exit"
}
// Data structures needed for the console test.  Threads making
// I/O requests wait on a Semaphore to delay until the I/O completes.

static Console *console;
static Semaphore *readAvail;
static Semaphore *writeDone;

//----------------------------------------------------------------------
// ConsoleInterruptHandlers
// 	Wake up the thread that requested the I/O.
//----------------------------------------------------------------------

static void ReadAvail(int arg) { readAvail->V(); }
static void WriteDone(int arg) { writeDone->V(); }

//----------------------------------------------------------------------
// ConsoleTest
// 	Test the console by echoing characters typed at the input onto
//	the output.  Stop when the user types a 'q'.
//----------------------------------------------------------------------

void 
ConsoleTest (char *in, char *out)
{
    char ch;

    console = new Console(in, out, ReadAvail, WriteDone, 0);
    readAvail = new Semaphore("read avail", 0);
    writeDone = new Semaphore("write done", 0);
    
    for (;;) {
	readAvail->P();		// wait for character to arrive
	ch = console->GetChar();
	console->PutChar(ch);	// echo it!
	writeDone->P() ;        // wait for write to finish
	if (ch == 'q') return;  // if q, quit
    }
}

