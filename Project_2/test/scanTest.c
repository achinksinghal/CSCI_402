#include "syscall.h"

int loop=-1;
int scanNum=-1;
int scanLock=-1;
int scanned = 0;



void t4()
{
		AcquireLock(scanLock);

		Print("Give Input\n",-1,-1,-1);
		scanned = Scan(&scanned);
		Print("Number scanned in thread 4 is %d\n",scanned,-1,-1);

		ReleaseLock(scanLock);
		
	Exit(0);
}	

void t3()
{

		AcquireLock(scanLock);

	
		Print("Give Input\n",-1,-1,-1);
		scanned = Scan(&scanned);
		Print("Number scanned in thread 3 is %d\n",scanned,-1,-1);

		ReleaseLock(scanLock);
	
	Exit(0);
	
}	

void t2()
{
		
	
		AcquireLock(scanLock);	
	
		Print("Give Input\n",-1,-1,-1);
		scanned = Scan(&scanned);
		Print("Number scanned in thread 2 is %d\n",scanned,-1,-1);

		ReleaseLock(scanLock);

		Exit(0);
}

void t1()
{
	
		AcquireLock(scanLock);

		Print("Give Input\n",-1,-1,-1);
		scanned = Scan(&scanned);
		Print("Number scanned in thread 1 is %d\n",scanned,-1,-1);

		ReleaseLock(scanLock);

		Exit(0);
	
}


int main()
{
	scanLock = CreateLock("scanLock");

	
	Fork(t1,"Thread 1");
	Fork(t2,"Thread 2");
	Fork(t3,"Thread 3");
	Fork(t4,"Thread 4");
}

