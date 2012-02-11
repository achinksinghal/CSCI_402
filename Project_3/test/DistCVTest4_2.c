#include "syscall.h"

int cv2=-1, cv1=-1;
int lock =-1;
int lock2=-1;

int main()
{

	lock = CreateLock("lock1");
	lock2 = CreateLock("lock2");

	cv1 = CreateCV("cv 1");
	cv2=CreateCV("cv 2");

	AcquireLock(lock);
	
	SignalCV(lock,cv1);
	
	Print("Client 2 is going to wait...",-1,-1,-1);
	
	WaitCV(lock,cv2);

	Print("Client 2 is now out of wait...",-1,-1,-1);

	ReleaseLock(lock);

}
