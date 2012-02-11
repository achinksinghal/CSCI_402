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

extern "C" { int bzero(char *, int); };
//extern BitMap *bitMap;     // bit map object for managing memory
//extern Lock *bitMapLock;   // lock to access the bit map object.

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

AddrSpace::AddrSpace(OpenFile *executable) : fileTable(MaxOpenFiles) {
    NoffHeader noffH;
    unsigned int i, size;

    // Don't allocate the input or output to disk files
    fileTable.Put(0);
    fileTable.Put(0);
	addrSpaceLock = new Lock("pageTableLock");

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
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

    ASSERT(numPages <= NumPhysPages);		// check we're not trying
						// to run anything too big --
						// at least until we have
						// virtual memory

    DEBUG('a', "Initializing address space, num pages %d, size %d\n", 
					numPages, size);
// first, set up the translation 
    pageTable = new TranslationEntry[numPages];
	
	int nextAvailablePage = 0;
	
    for (i = 0; i < numPages; i++) 
	{
		pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
		
		bitMapLock -> Acquire();
		nextAvailablePage = bitMap -> Find();
		if(nextAvailablePage == -1)
		{
			printf("\nNachos ran out of memory : It will now halt\n");
			interrupt -> Halt(); // Halt nachos and exit 
		}
		bitMapLock -> Release();
		pageTable[i].physicalPage = nextAvailablePage;
		pageTable[i].valid = TRUE;
		pageTable[i].use = FALSE;
		pageTable[i].dirty = FALSE;
		pageTable[i].readOnly = FALSE;  // if the code segment was entirely on 
					// a separate page, we could set its 
					// pages to be read-only
					
		// zero out the physical page that is being copied. The index is multiplied with page size as the mainMemory is a character array.
		//bzero(&(machine->mainMemory[nextAvailablePage * PageSize]), PageSize);
		
		// read the executable page by page and copy it to the main memory.
		executable -> ReadAt(&(machine -> mainMemory[nextAvailablePage * PageSize]), PageSize, noffH.code.inFileAddr + (i * PageSize));
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
        //printf("\n%s acquired lock inside ResizePageTable\n", currentThread -> getName());

        int numberOfStackPages = UserStackSize / PageSize;

        //printf("\nSUMIT The number of stack pages Allocated to thread %s is %d\n", currentThread -> getName(), numberOfStackPages);

        // increase the size of the page table to 


        TranslationEntry tempPageTable[numPages + numberOfStackPages];

        for(int idx = 0; idx < numPages; idx++)
        {
                // //copy the data to temporary page table
               tempPageTable[idx] = pageTable[idx];
        }

        // we need to find the next virtual adderess as we are removing stack pages. because the virtual address

        int pgIdx = 0;

        // the next pages belong to the stack;
        currentThread -> tcb -> SetStackStart(numPages);


        // add the stack pages to the page table
        for(int Idx = 0; Idx < 8; Idx++)
        {
                pgIdx = numPages + Idx;


                //printf("\n\nSUMIT : Thread %s was allocated PageNumber : %d\n", currentThread -> getName(), pgIdx);

                if(pgIdx >= NumPhysPages)
                {
                        printf("Nachos ran out of memory\n");
                        interrupt -> Halt();
                }

                tempPageTable[pgIdx].virtualPage = pgIdx;

                bitMapLock -> Acquire();
                int nextAvailablePage = bitMap -> Find();
                bitMapLock -> Release();

               if(nextAvailablePage == -1)
                {
                        printf("\nNachos ran out of memory\n");
                        interrupt -> Halt(); // Halt nachos and exit 
                }
                tempPageTable[pgIdx].physicalPage = nextAvailablePage;
                tempPageTable[pgIdx].valid = TRUE;
                tempPageTable[pgIdx].use = FALSE;
                tempPageTable[pgIdx].dirty = FALSE;
                tempPageTable[pgIdx].readOnly = FALSE;  // if the code segment was entirely on 
                                        // a separate page, we could set its 
                                        // pages to be read-only

                // zero out the physical page that is being copied. The index is multiplied with page size as the mainMemory is a character array.
                //bzero(&(machine->mainMemory[nextAvailablePage * PageSize]), PageSize);
        }

        // increment the size of the table to include the newly added pages for the stack.

        //printf("\nthe Last Size for process %d is %d\n", (currentThread -> GetParentProcess), (pgIdx * PageSize));

        numPages += 8;

        //pageTable = tempPageTable;

        //delete pageTable;
        pageTable = new TranslationEntry[numPages];

        for(int i = 0; i < numPages; i++)
        {
                pageTable[i] = tempPageTable[i];
        }

        //delete tempPageTable;


        //printf("\n%s released lock in ResizePageTable\n", currentThread -> getName());
        addrSpaceLock -> Release();

        // return the last stack top address back to the exception handler to write to StackReg.

        //printf("\nSUMIT : the virtual stack address for thread %s is %d\n\n", currentThread -> getName(), pgIdx * PageSize);
        return pgIdx * PageSize;
}


//-------------------------------------------   -------------------------------------
//AddrSpace::RemoveThreadStack
// Deallocate the stack pages related to thread that is about to finish
//
//---------------------------------------------------------------------------------

void AddrSpace :: RemoveThreadStack()
{
        addrSpaceLock -> Acquire();
        //printf("\n%s acquired the lock in RemoveThreadStack\n", currentThread -> getName());

        //TranslationEntry tempPageTable[numPages];
        
        int stackSt = currentThread -> tcb -> GetStackStart();
        
        for(int idx = 0; idx < numPages; idx++)
        {
                if(((idx >= stackSt) && (idx < stackSt + 8)) && (pageTable[idx].valid == TRUE))
                {
                        //printf("\n\nSUMIT : Thread %s was removed of PageNumber : %d\n", currentThread -> getName(), idx);

                        // deallocate this page
                        bitMap -> Clear(pageTable[idx].physicalPage);
                }

        }

        //printf("\n%s released the lock in RemoveThreadStack\n", currentThread -> getName());
        addrSpaceLock -> Release();
}




//----------------------------------------------------------------------
// AddrSpace::ResizePageTable
//	Stores the current page table in a temprary memory and increases the 
// size of the page table on demand. New pages are added to the pageTable
// of the process if it spawns a new thread
//
//-----------------------------------------------------------------------

/*int AddrSpace :: ResizePageTable()
{
	addrSpaceLock -> Acquire();
	//create a new temporary page table and add a physical page entry to it
	TranslationEntry *tempPageTable = pageTable;
	
	int numberOfStackPages = UserStackSize / PageSize;
	
	// increase the size of the page table to 
	pageTable = new TranslationEntry[numPages + numberOfStackPages];
	
	for(int idx = 0; idx < numPages; idx++)
	{
		//copy back the priginal contents of the page table
		pageTable[idx] = tempPageTable[idx]; 
	}
	
	int pgIdx = 0;
	
	// add the stack pages to the page table
	for(int Idx = 0; Idx < numberOfStackPages; Idx++)
	{	
		pgIdx = numPages + Idx;
		
		pageTable[pgIdx].virtualPage = pgIdx;	
		
		bitMapLock -> Acquire();
		int nextAvailablePage = bitMap -> Find();
		bitMapLock -> Release();
		pageTable[pgIdx].physicalPage = nextAvailablePage;
		pageTable[pgIdx].valid = TRUE;
		pageTable[pgIdx].use = FALSE;
		pageTable[pgIdx].dirty = FALSE;
		pageTable[pgIdx].readOnly = FALSE;  // if the code segment was entirely on 
					// a separate page, we could set its 
					// pages to be read-only
					
		// zero out the physical page that is being copied. The index is multiplied with page size as the mainMemory is a character array.
		bzero(&(machine->mainMemory[nextAvailablePage * PageSize]), PageSize);
	}
	
	// increment the size of the table to include the newly added pages for the stack. 
	
	numPages += numberOfStackPages;
	
	addrSpaceLock -> Release();
	// return the last stack top address back to the exception handler to write to StackReg.
	return pgIdx * PageSize;
}
*/
//--------------------------------------------------------------------------------
//AddrSpace::RemoveThreadStack
// Deallocate the stack pages related to thread that is about to finish
//
//---------------------------------------------------------------------------------

/*void AddrSpace :: RemoveThreadStack()
{
	TranslationEntry *tempPageTable = new TranslationEntry[numPages - 8];
	
	int stackStart = currentThread -> tcb -> GetStackStart();
	
	int count = 0;
	for(int idx = 0; idx < numPages; idx++)
	{
		if((pageTable[idx].physicalPage >= stackStart) && (pageTable[idx].physicalPage <= stackStart + 8))
		{
			// deallocate this page
			bitMap -> Clear(pageTable[idx].physicalPage);
		}
		else
		{
			// copy the remaining pages to the temporary page table
			tempPageTable[count++] = pageTable[idx];
		}
	}
	
	pageTable = tempPageTable;
}
*/

//---------------------------------------------------------------------
// AddrSpace :: ReclaimPhysicalPages
// clears out the memory for the entire space allocated to the process
//
//
//---------------------------------------------------------------------

void AddrSpace :: ReclaimPhysicalPages()
{
        
        for(int i = 0; i < numPages; i++)
        {
                bitMap -> Clear(pageTable[i].physicalPage);
        }
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
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
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
