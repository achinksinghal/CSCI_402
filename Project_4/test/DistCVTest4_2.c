#include "syscall.h"

int cv2=-1, cv1=-1;
int lock =-1;
int lock2=-1;
int i=0;

int main()
{

	lock = CreateLock("lock");
	lock2 = CreateLock("lock2");

	cv1 = CreateCV("cv1");
	cv2=CreateCV("cv2");

	AcquireLock("lock");
	
	SignalCV("lock","cv1");
	
	Print("Client 2 is going to wait...",-1,-1,-1);
	
	WaitCV("lock","cv2");

	for (i=0; i<9; i++)
		Print("\n",-1,-1,-1);
	
	Print("Client 2 is now out of wait...",-1,-1,-1);

	ReleaseLock("lock");

	Exit(0);

}
