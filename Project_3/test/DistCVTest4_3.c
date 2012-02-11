#include "syscall.h"

int lock =-1;
int cv2=-1;


int main()
{

	lock = CreateLock("lock1");
	cv2 = CreateCV("cv 2");
	
	AcquireLock(lock);

	SignalCV(lock,cv2);

	Print("Client 3 will now destroy CV...",-1,-1,-1);

	DestroyCV(cv2);


	ReleaseLock(lock);

	AcquireLock(lock);	
	
	WaitCV(lock,cv2);
	
	Print("Client 3: This prints as CV has been destroyed...",-1,-1,-1);

	ReleaseLock(0);

}
