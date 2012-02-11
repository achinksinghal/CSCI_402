#include "syscall.h"

int mv=-1,mv3=1;
int lck=-1;
int ret=-1;


int main()
{
	mv = CreateMV("mv1");
	lck = CreateLock("lock1");

	AcquireLock("lock1");
	mv3 = GetMV("mv1");
	Print("Client 3: Monitor variable was read to be %d..",mv3,-1,-1);
	ReleaseLock("lock1");

	mv3++;
	AcquireLock("lock1");
	ret=SetMV("mv1",mv3);
	mv3 = GetMV("mv1");
	
	Print("Client 3: Monitor variable was set to be %d..",mv3,-1,-1);
	ReleaseLock("lock1");

	Exit(0);

	}
