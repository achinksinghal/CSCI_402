#include "syscall.h"

int mv=-1,mv2=0;
int ret=-1;
int lck=-1;



int main()
{

	mv = CreateMV("mv1");
	lck = CreateLock("lock1");

	AcquireLock(lck);
	mv2 = GetMV(mv);
	Print("Client 2: Monitor variable was read to be %d..",mv2,-1,-1);
	ReleaseLock(lck);

	mv2++;
	AcquireLock(lck);
	ret=SetMV(mv,mv2);
	mv2 = GetMV(mv);
	
	Print("Client 2: Monitor variable was set to be %d..",mv2,-1,-1);

	DestroyMV(mv);

	ReleaseLock(lck);

}
