#include "syscall.h"

int lock =-1;
int cv2=-1;
int i=0;

int main()
{

	lock = CreateLock("lock");
	cv2 = CreateCV("cv2");
	
	AcquireLock("lock");

	SignalCV("lock","cv2");

	Print("Client 3 will now destroy CV...",-1,-1,-1);
	
	DestroyCV("cv2");
	
	for (i=0; i<9; i++)
		Print("\n",-1,-1,-1);


	ReleaseLock("lock");

	AcquireLock("lock");	
	
	WaitCV("lock","cv2");
	
	Print("Client 3: This prints as CV has been destroyed...",-1,-1,-1);

	ReleaseLock("lock");

	Exit(0);

}
