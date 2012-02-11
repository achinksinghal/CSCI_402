#include "syscall.h"

int mv=-1,mv4=1;
int lck=-1;
int ret=-1;



int main()
{

	mv = CreateMV("mv1");
	lck = CreateLock("lock1");

	AcquireLock(lck);
	mv4 = GetMV(mv);
	Print("Client 4: Monitor variable was read to be %d..",mv4,-1,-1);
	ReleaseLock(lck);

	mv4++;
	AcquireLock(lck);
	ret=SetMV(mv,mv4);
	mv4 = GetMV(mv);
	
	Print("Client 4: Monitor variable was set to be %d..",mv4,-1,-1);
	ReleaseLock(mv);
	

}
