// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"
#include "table.h"
#include "synch.h"

//#define STEP3 1

extern "C" { int bzero(char *, int); };
extern List *ppnQ;   // queue for fifo page replacement policy.
extern BitMap *swapBitMap; //swap bit map
extern OpenFile *swapFile;//swap file pointer
extern char swapFileName[100];//swap file pointer
extern Lock *iptLock;	//ipt lock
extern Lock *swapLock;	//swap file lock
extern Lock *ppnQLock;	//fifo page replacement policy lock

extern int replacementPolicy; //which page replacement policy is used.

/*
 * IPT class is defined here
 * */
class IPT:public TranslationEntry //subclass of Translation entry class
{
        public:
                AddrSpace *space;// addrspace class pointer to identify process
                //int usage;
	IPT()
	{
                //usage = 0;
	}
};

IPT *ipt = new IPT[NumPhysPages]; //ipt is created.

Table::Table(int s) : map(s), table(0), lock(0), size(s) {
    table = new void *[size];
    lock = new Lock("TableLock");
}

Table::~Table() {
    if (table) {
	delete table;
	table = 0;
    }
    if (lock) {
	delete lock;
	lock = 0;
    }
}

void *Table::Get(int i) {
    // Return the element associated with the given if, or 0 if
    // there is none.

    return (i >=0 && i < size && map.Test(i)) ? table[i] : 0;
}

int Table::Put(void *f) {
    // Put the element in the table and return the slot it used.  Use a
    // lock so 2 files don't get the same space.
    int i;	// to find the next slot

    lock->Acquire();
    i = map.Find();
    lock->Release();
    if ( i != -1)
	table[i] = f;
    return i;
}

void *Table::Remove(int i) {
    // Remove the element associated with identifier i from the table,
    // and return it.

    void *f =0;

    if ( i >= 0 && i < size ) {
	lock->Acquire();
	if ( map.Test(i) ) {
	    map.Clear(i);
	    f = table[i];
	    table[i] = 0;
	}
	lock->Release();
    }
    return f;
}

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	"executable" is the file containing the object code to load into memory
//
//      It's possible to fail to fully construct the address space for
//      several reasons, including being unable to allocate memory,
//      and being unable to read key parts of the executable.
//      Incompletely consretucted address spaces have the member
//      constructed set to false.
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executable1) : fileTable(MaxOpenFiles) {
	NoffHeader noffH;
	unsigned int i, size;

	// Don't allocate the input or output to disk files
	fileTable.Put(0);
	fileTable.Put(0);
	addrSpaceLock = new Lock("pageTableLock");

	this->executable = executable1;//executable pointer is saved

	executable1->ReadAt((char *)&noffH, sizeof(noffH), 0);
	if ((noffH.noffMagic != NOFFMAGIC) && 
			(WordToHost(noffH.noffMagic) == NOFFMAGIC))
		SwapHeader(&noffH);
	ASSERT(noffH.noffMagic == NOFFMAGIC);

	size = noffH.code.size + noffH.initData.size + noffH.uninitData.size ;
	stackStart = divRoundUp(size, PageSize); // store the start of the stack for the current thread
	numPages = stackStart + divRoundUp(UserStackSize,PageSize);
	// we need to increase the size
	// to leave room for the stack
	size = numPages * PageSize;
	

	//ASSERT(numPages <= NumPhysPages);		// check we're not trying
	// to run anything too big --
	// at least until we have
	// virtual memory

	DEBUG('a', "Initializing address space, num pages %d, size %d\n", 
			numPages, size);
	// first, set up the translation 
	pageTable = new TranslationEntry[numPages];
	pageTableX = new PageTableEnhancedData[numPages]; //extended page table is declared

	int nextAvailablePage = 0;

	for (i = 0; i < numPages; i++) 
	{
		pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
		pageTable[i].physicalPage = -1; //physical page is not allocated yet.
		pageTable[i].valid = FALSE;
		pageTable[i].use = FALSE;
		pageTable[i].dirty = FALSE;
		pageTable[i].readOnly = FALSE;  // if the code segment was entirely on 
		// a separate page, we could set its 
		// pages to be read-only

		if(i <  divRoundUp(noffH.code.size + noffH.initData.size, PageSize) )	//code and initialize data offset are stored in pagetable, location is set executable
		{
			pageTableX[i].diskLocation = EXECUTABLE;
			pageTableX[i].byteOffset = noffH.code.inFileAddr + (i * PageSize);//offset in executable;   
		}   
		else // this is not in executable, so its offset is not required.
		{
			pageTableX[i].diskLocation = NOT_ON_DISK;
			pageTableX[i].byteOffset = -1;   
		}
	}

	// we'll use the last page mapped to find the stack pointer and put that in StackReg
	lastPhysicalPageMapped = nextAvailablePage;
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
//
// 	Dealloate an address space.  release pages, page tables, files
// 	and file tables
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
    delete pageTable;
    delete pageTableX;
}


//----------------------------------------------------------------------
// AddrSpace::populateTLBFromPagetable
//
// 	populate the TLB from pageTable, if TLB miss is found with pageTable entry
//----------------------------------------------------------------------

int currentTLBIndex= 0; //this is the tlb index where last changes to tlb are made
void AddrSpace::populateTLBFromPagetable(int currentVPN)
{
	IntStatus oldLevel = interrupt->SetLevel(IntOff);//interupts are made off this required in STEP 1 of project 3 implementation
	currentTLBIndex = (currentTLBIndex + 1)% TLBSize; //current tlb index is set to next tlb index
	// for STEP 1 of project 3, all fields are copied from pagetable
	machine->tlb[currentTLBIndex].virtualPage = pageTable[currentVPN].virtualPage;
	machine->tlb[currentTLBIndex].physicalPage = pageTable[currentVPN].physicalPage;
	machine->tlb[currentTLBIndex].valid = pageTable[currentVPN].valid;
	machine->tlb[currentTLBIndex].use = pageTable[currentVPN].use;
	machine->tlb[currentTLBIndex].dirty = pageTable[currentVPN].dirty;
	machine->tlb[currentTLBIndex].readOnly = pageTable[currentVPN].readOnly;	
	(void) interrupt->SetLevel(oldLevel);
}

/*
 * evictPageFromMemory(int currentVPN)
 * This function evicts a page from physical memory
 * which can be further loaded with a new virtual
 * page.
 * */
int evictPageFromMemory(int currentVPN)
{

	int evictedPPN, i=0;
	int evictedVPN;

	if(replacementPolicy == 1) //if fifo is used a page replacement policy
	{ 
		do //it finds an currently unused page in memory
		{
			evictedPPN = -1;
			ppnQLock->Acquire();
			if(!ppnQ->IsEmpty())
				evictedPPN = (int)ppnQ->Remove();	//removed physical page from fifo q.
			ppnQLock->Release();
			if( evictedPPN >= 0 )
			{
				iptLock->Acquire();
				if(ipt[evictedPPN].use == FALSE) 		//if evicted page is not in use by some other thread
				{
					ipt[evictedPPN].use = TRUE;
					if(ipt[evictedPPN].space == currentThread->space)	//if it belongs to same process
					{
						IntStatus oldLevel = interrupt->SetLevel(IntOff); //then interupts are made off
						for(i=0; i<=TLBSize; i++)
						{
							if(machine->tlb[i].physicalPage == evictedPPN && machine->tlb[i].valid == TRUE)	//if it is a valid tlb entry 
							{
								ipt[evictedPPN].dirty = machine->tlb[i].dirty; //then propagate tlb dirty bit.
								machine->tlb[i].valid = FALSE;			//and invalidate it
							}
						}
						(void) interrupt->SetLevel(oldLevel); 				//interupts are reset
					}
					evictedVPN = ipt[evictedPPN].virtualPage;				//evicted vpn is saved
					ipt[evictedPPN].virtualPage = currentVPN;
					if(replacementPolicy == 1)
					{ 

						ppnQLock->Acquire();						//evicted ppn is appended in the queue.
						ppnQ->Append((void *)evictedPPN); 
						ppnQLock->Release();
					}

					ipt[evictedPPN].space->addrSpaceLock->Acquire();			//lock of evicted ppn's vpn page table is acquired
					iptLock->Release();
					//ipt[evictedPPN].usage++;
					break;
				}
				else  					// if evicted page is in use by some other thread
				{
					if(replacementPolicy == 1)	//fifo is used	
					{ 

						ppnQLock->Acquire();
						ppnQ->Prepend((void *)evictedPPN); //then prepend it to fifo queue.
						ppnQLock->Release();
						iptLock->Release();		//ipt lock is released
					}
				}
			}
		}
		while(true);
	}
	else//if random is used a page replacement policy
	{ 
		do//it finds an currently unused page in memory
		{
			evictedPPN = random() % NumPhysPages; //random page is taken 
			iptLock->Acquire();//lock is taken to provide mutual exclusion
			if(ipt[evictedPPN].use == FALSE)	//evicted ppn is not in use by other thread
			{
				ipt[evictedPPN].use = TRUE;
				if(ipt[evictedPPN].space == currentThread->space) //if it belongs to same process
				{
					IntStatus oldLevel = interrupt->SetLevel(IntOff);//then interupts are made off

					for(i=0; i<=TLBSize; i++)
					{
						if(machine->tlb[i].physicalPage == evictedPPN && machine->tlb[i].valid == TRUE)//if it is a valid tlb entry 
						{
							ipt[evictedPPN].dirty = machine->tlb[i].dirty;//then propagate tlb dirty bit
							machine->tlb[i].valid = FALSE;//and invalidate it
						}
					}
					(void) interrupt->SetLevel(oldLevel);	//interupts are reset

				}
				evictedVPN = ipt[evictedPPN].virtualPage;//evicted vpn is saved
				ipt[evictedPPN].virtualPage = currentVPN;
				ipt[evictedPPN].space->addrSpaceLock->Acquire();//lock of evicted ppn's vpn page table is acquired
				iptLock->Release();
				//ipt[evictedPPN].usage++;
				break;
			}
			else iptLock->Release();
		}
		while(true);
	}

	swapLock->Acquire();
	int swapLocation = (int)swapBitMap->Find(); //free location in swap file is found
	swapLock->Release();

	if(ipt[evictedPPN].dirty == TRUE) //if evicted page is dirty i.e. its contents are changed or written then it is saved in the swapfile
	{
		swapFile->WriteAt(&(machine->mainMemory[ipt[evictedPPN].physicalPage * PageSize]),PageSize , swapLocation * PageSize); //wrting to swap file
		ipt[evictedPPN].space->pageTableX[evictedVPN].byteOffset = swapLocation * PageSize; //saving swap file location
		ipt[evictedPPN].space->pageTableX[evictedVPN].diskLocation = SWAP_FILE; //changing location of page to swap file
	}
	ipt[evictedPPN].space->addrSpaceLock->Release(); //lock of evicted ppn's vpn page table is released

	return evictedPPN;
}
/*
 * handleIPTMiss(int currentVPN)
 * This functions handles an IPT miss
 * first it finds the memory, if page is not
 * available there then it evict page from
 * memory and uses it.
 * */
int AddrSpace::handleIPTMiss(int currentVPN)
{
	int nextAvailablePage = 0;
	bitMapLock -> Acquire();
	nextAvailablePage = bitMap -> Find(); //first memory bitmap is searched for a free entry
	bitMapLock -> Release();

	if(nextAvailablePage == -1)//if page is not available there then a page is evicted from the memory
	{
		nextAvailablePage = evictPageFromMemory(currentVPN);
	}
	else // and if available, then that page is used.
	{
		iptLock->Acquire();			
		ipt[nextAvailablePage].use = TRUE; //page is made to be use
		if(replacementPolicy == 1) //if fifo is in use as a page replacement policy then
		{ 
			ppnQLock->Acquire();
			ppnQ->Append((void *)nextAvailablePage); //page is appended in the fifo queue
			ppnQLock->Release();
		}
		iptLock->Release();
		//ipt[nextAvailablePage].usage++;
	}

	currentThread->space->addrSpaceLock->Acquire();
	currentThread->space->pageTable[currentVPN].physicalPage = nextAvailablePage;//available page is set as a current ppn

	currentThread->space->pageTable[currentVPN].valid = TRUE; //valid is made true
	if(currentThread->space->pageTableX[currentVPN].diskLocation == EXECUTABLE) //if it is executable page then read it from executable
	{
		executable -> ReadAt(&(machine -> mainMemory[nextAvailablePage * PageSize]), PageSize, currentThread->space->pageTableX[currentVPN].byteOffset);
	}
	else if(currentThread->space->pageTableX[currentVPN].diskLocation == SWAP_FILE)// else if it is in swap, read from there
	{
		swapFile -> ReadAt(&(machine -> mainMemory[nextAvailablePage * PageSize]), PageSize, currentThread->space->pageTableX[currentVPN].byteOffset);
	}

	currentThread->space->addrSpaceLock->Release();

	iptLock->Acquire();
	ipt[nextAvailablePage].virtualPage = currentThread->space->pageTable[currentVPN].virtualPage;	//vpn is copied in ipt
	ipt[nextAvailablePage].physicalPage = currentThread->space->pageTable[currentVPN].physicalPage; 
	ipt[nextAvailablePage].valid = TRUE; 								//valid is made true
	ipt[nextAvailablePage].dirty = FALSE;								// dirty is set to true
	ipt[nextAvailablePage].readOnly = currentThread->space->pageTable[currentVPN].readOnly; 	
	ipt[nextAvailablePage].space = currentThread->space; 						//current address space is set.
	iptLock->Release();
	return nextAvailablePage;
}

//----------------------------------------------------------------------
// populateTLBFromIPT
// function is to 
// populate the TLB from IPT 
//----------------------------------------------------------------------

int populateTLBFromIPT(int currentVPN)
{
	int i=0; 
	int foundInIPT=0;
	int physicalPage = -1;

	iptLock -> Acquire();//ipt lock is acquired to provide mutual exclusion
	for(i=0; i<NumPhysPages; i++)//ipt is searched if required vpn entry is in page table or not
	{
		if(		//checked if there is in valid, currently unused and same process entry required vpn in ipt or not
				ipt[i].virtualPage == currentVPN &&
				ipt[i].valid == TRUE &&
				ipt[i].use == FALSE &&
				ipt[i].space == currentThread->space
		  )
		{
			physicalPage = i;			// if it is there then, that ppn is used can be used
			ipt[i].use = TRUE; 			// use bit is made true.
			foundInIPT = 1;
			break;
		}
	}
	iptLock -> Release(); 
	//if(physicalPage != -1) 
	//{
		//ipt[physicalPage].usage++;
	//}

	//means there is a IPT miss, that should be handled
	if(physicalPage == -1)
	{
		physicalPage = currentThread->space->handleIPTMiss(currentVPN); //handling ip miss here
	}

	//filling values to TLB after IPT is populated
	{
		foundInIPT = 1;
		IntStatus oldLevel = interrupt->SetLevel(IntOff); //interupts are made off
		currentTLBIndex = (currentTLBIndex + 1)% TLBSize; //current index is moved to next location

		if(machine->tlb[currentTLBIndex].valid == TRUE)   //check if current tlb index has some valid tlb entry...
		{
			ipt[machine->tlb[currentTLBIndex].physicalPage].dirty = machine->tlb[currentTLBIndex].dirty;	//then, dirty bit is propagated
		}

		machine->tlb[currentTLBIndex].virtualPage = ipt[physicalPage].virtualPage; //copy vpn
		machine->tlb[currentTLBIndex].physicalPage = ipt[physicalPage].physicalPage; //copy vpn
		machine->tlb[currentTLBIndex].valid = ipt[physicalPage].valid; //valid bit is set
		machine->tlb[currentTLBIndex].use = FALSE;			//use bit is set
		machine->tlb[currentTLBIndex].dirty = ipt[physicalPage].dirty;	//dirty bit is set
		machine->tlb[currentTLBIndex].readOnly = ipt[physicalPage].readOnly;	//readonly bit is set
		(void) interrupt->SetLevel(oldLevel); //interupts are reset
	}
	iptLock -> Acquire();
	ipt[physicalPage].use = FALSE;
	//ipt[physicalPage].usage--;
	iptLock -> Release(); 
	return foundInIPT;
}



//----------------------------------------------------------------------
// AddrSpace :: SetDefaultStack
// Sets the stack top for the first thread of the process
//
//----------------------------------------------------------------------

void AddrSpace :: SetDefaultStack()
{
	currentThread -> tcb -> SetStackStart(stackStart);
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %x\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::ResizePageTable
//      Stores the current page table in a temprary memory and increases the 
// size of the page table on demand. New pages are added to the pageTable
// of the process if it spawns a new thread
//
//-----------------------------------------------------------------------

int AddrSpace :: ResizePageTable()
{
        addrSpaceLock -> Acquire();
        int numberOfStackPages = UserStackSize / PageSize;

        // increase the size of the page table to 

        TranslationEntry tempPageTable[numPages + numberOfStackPages];
        PageTableEnhancedData tempPageTableX[numPages + numberOfStackPages];

        for(int idx = 0; idx < numPages; idx++)
        {
                // //copy the data to temporary page table
               tempPageTable[idx] = pageTable[idx];
               tempPageTableX[idx] = pageTableX[idx]; //extended page table data is copied
        }

        // we need to find the next virtual adderess as we are removing stack pages. because the virtual address

        int pgIdx = 0;

        // the next pages belong to the stack;
        currentThread -> tcb -> SetStackStart(numPages);

        // add the stack pages to the page table
        for(int Idx = 0; Idx < 8; Idx++)
        {
                pgIdx = numPages + Idx;
                tempPageTable[pgIdx].virtualPage = pgIdx;
		tempPageTable[pgIdx].physicalPage = -1;
                tempPageTable[pgIdx].valid = FALSE;
                tempPageTable[pgIdx].use = FALSE;
                tempPageTable[pgIdx].dirty = FALSE;
                tempPageTable[pgIdx].readOnly = FALSE;  // if the code segment was entirely on 
                                        // a separate page, we could set its 
                                        // pages to be read-only
		tempPageTableX[pgIdx].diskLocation = NOT_ON_DISK; //as it is on stack so made not on disk
		tempPageTableX[pgIdx].byteOffset = -1;//As on stack, so no offset meaning   

        }
        numPages += 8;

        pageTable = new TranslationEntry[numPages];
        pageTableX = new PageTableEnhancedData[numPages];

        for(int i = 0; i < numPages; i++)
        {
                pageTable[i] = tempPageTable[i];
                pageTableX[i] = tempPageTableX[i];
        }

        addrSpaceLock -> Release();

        return pgIdx * PageSize;
}


//-------------------------------------------   -------------------------------------
//AddrSpace::RemoveThreadStack
// Deallocate the stack pages related to thread that is about to finish
//
//---------------------------------------------------------------------------------

void AddrSpace :: RemoveThreadStack()
{       
        int stackSt = currentThread -> tcb -> GetStackStart();
	iptLock->Acquire();
        for(int idx = 0; idx < numPages; idx++) //complete ipt is searched, if the valid and same process vpn is in ipt
        {
            if(((idx >= stackSt) && (idx < stackSt + 8)) && (pageTable[idx].valid == TRUE) && 
					(ipt[pageTable[idx].physicalPage].space == currentThread -> space))
            {
		    ipt[pageTable[idx].physicalPage].valid = FALSE; //IPT entry is made invalidated.
	    }

        }
        iptLock -> Release();
}

//---------------------------------------------------------------------
// AddrSpace :: ReclaimPhysicalPages
// clears out the memory for the entire space allocated to the process
//
//
//---------------------------------------------------------------------

void AddrSpace :: ReclaimPhysicalPages()
{

	for(int i = 0; i < numPages; i++)//complete ipt is checked
	{
		iptLock->Acquire();
		if((ipt[pageTable[i].physicalPage].space == currentThread -> space)) //if it belongs to same process 
		{
			ipt[pageTable[i].physicalPage].valid = FALSE; //IPT entry is made invalidated.
		}
		iptLock->Release();
	}
	fileSystem->Remove(swapFileName); //removing the swap file
}


//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
	machine->pageTableSize = numPages;
	{
		int i = 0;
		IntStatus oldLevel = interrupt->SetLevel(IntOff);//on context switching, dirty bit is propagated and tlb entries are invalidated
		for(i=0; i < TLBSize; i++)
		{
			if(machine->tlb[i].valid == TRUE)
			{
				ipt[machine->tlb[i].physicalPage].dirty = machine->tlb[i].dirty;	//dirty bit is propagated 
			}
			machine->tlb[i].valid = FALSE;//tlb entries are invalidated
		}
		(void) interrupt->SetLevel(oldLevel); //interupts are reset.
	}
}

//--------------------------------------------------------------------------
// AddrSpace :: GetNumPages
//
// Get the total number of pages used by the address space
//--------------------------------------------------------------------------
int AddrSpace :: GetNumPages()
{
	addrSpaceLock -> Acquire();
	int pages = numPages;
	addrSpaceLock -> Release();
	
	return pages;
}
