#include "syscall.h"

int cv=-1;
int lck=-1;

void main()
{
	lck = CreateLock("lock1");
	cv = CreateCV("CV1");
	Print("Client 2: Created Lock is %d and CV is %d\n", lck, cv, -1);
	
	AcquireLock(lck);
	Print("Client 2: About to signal Client 1\n", -1, -1, -1);

	SignalCV(lck,cv);
	
	Print("Client 2: About to go on wait...\n", -1, -1, -1);

	
	WaitCV(lck,cv);
	
	Print("Client 2: Now out of wait...\n", -1, -1, -1);
	ReleaseLock(lck);
}
