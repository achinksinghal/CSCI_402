#include "syscall.h"

int main()
{
	Write("Creating Movietheature child process", 37, 1);
	Exec("../test/movietheature", 21);
	Exit(0);
	}
