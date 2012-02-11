#include "syscall.h"

int cv=-1;
int lck=-1;
int num=0;
int i=0;

void main()
{
	
	lck = CreateLock("lock");

	cv = CreateCV("cv");

	AcquireLock("lock");

	Print("Client 2 to go on wait...\n",-1,-1,-1);

	WaitCV("lock","cv");
	for (i=0; i<10; i++)
		Print("\n",-1,-1,-1);
	Print("Client 2 out of wait...\n",-1,-1,-1);
	
	ReleaseLock("lock");

	Exit(0);
	
}
