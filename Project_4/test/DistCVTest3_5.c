#include "syscall.h"

int mv=-1,mv5=1;
int lck=-1;
int ret=-1;

int main()
{

	mv = CreateMV("mv1");
	lck = CreateLock("lock1");

	
	AcquireLock("lock1");
	mv5 = GetMV("mv1");
	Print("Client 5: Monitor variable was read to be %d..",mv5,-1,-1);
	ReleaseLock("lock1");

	mv5++;
	AcquireLock("lock1");
	ret=SetMV("mv1",mv5);
	mv5 = GetMV("mv1");
	
	Print("Client 5: Monitor variable was set to be %d..",mv5,-1,-1);
	ReleaseLock("lock1");

	Exit(0);
}
