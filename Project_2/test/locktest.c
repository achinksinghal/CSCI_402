#include "syscall.h"
int i;
int j;

int main()
{
	Print("Will try to create a lock...\n", -1, -1, -1);
	NewLine();
	i=CreateLock("Lock1");
	Print("Lock_%d has been created...\n", i, -1, -1);
	AcquireLock(i);
	Print("Lock_%d has been acquired...\n", i, -1, -1);
	AcquireLock(i+1);
	Print("Sending request to destroy Lock_%d, followed by one to release it ...\n", i, -1, -1);
	DestroyLock(i);
	ReleaseLock(i);
	ReleaseLock(i+1);
}
