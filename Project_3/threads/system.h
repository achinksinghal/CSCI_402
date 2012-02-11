// system.h 
//	All global variables used in Nachos are defined here.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef SYSTEM_H
#define SYSTEM_H


#include "copyright.h"
#include "utility.h"
#include "thread.h"
#include "scheduler.h"
#include "interrupt.h"
#include "stats.h"
#include "timer.h"
//#include "../userprog/bitmap.h"
//#include "../machine/machine.h"
#include "synch.h"
#include "Process.h"

// Initialization and cleanup routines
extern void Initialize(int argc, char **argv); 	// Initialization,
						// called before anything else
extern void Cleanup();				// Cleanup, called when
						// Nachos is done.

extern Thread *currentThread;			// the thread holding the CPU
extern Thread *threadToBeDestroyed;  		// the thread that just finished
extern Scheduler *scheduler;			// the ready list
extern Interrupt *interrupt;			// interrupt status
extern Statistics *stats;			// performance metrics
extern Timer *timer;				// the hardware alarm clock



#ifdef USER_PROGRAM
#include "machine.h"

extern Machine* machine;	// user program memory and registers
extern BitMap *bitMap;				//bitmap object - to find empty pages JS
extern Lock *bitMapLock;
extern Process **processTable;
extern BitMap *processBitMap;
extern Lock *processLock; 
extern int ProcessCount;
#endif

#ifdef FILESYS_NEEDED 		// FILESYS or FILESYS_STUB 
#include "filesys.h"
extern FileSystem  *fileSystem;
#endif

#ifdef FILESYS
#include "synchdisk.h"
extern SynchDisk   *synchDisk;
#endif

#ifdef NETWORK
#include "post.h"
extern PostOffice* postOffice;

#define LOCK_SIZE 250
#define CV_SIZE 250
#define MV_SIZE 250

extern ServerLock* ServerLocks[LOCK_SIZE];
extern ServerCondition* ServerConditions[CV_SIZE];
extern MonitorVariable* MonitorVariables[MV_SIZE];

#endif

#endif // SYSTEM_H
