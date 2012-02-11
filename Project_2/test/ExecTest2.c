#include "syscall.h"

int main()
{
	Print("Creating ThreadTest child process", -1,-1,-1);
	Exec("../test/threadtest", 18);
	
	Exec("../test/threadtest", 18);
	Exit(0);
}
