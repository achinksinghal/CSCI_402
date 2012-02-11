#include "syscall.h"

/*
 * * This file is part of testcase of movie theater 
 * * simulation with 30 customers, 3 ticket taker
 * * 3 concession clerk, 3 ticket taker, a movie 
 * * technician and a manager.
 * * */

void main()
{
	int i=0;

	CreateMV("TC\0");
	SetMV("TC\0", 1);

	CreateMV("TCN\0");
	SetMV("TCN\0", -1);

	CreateMV("CU\0");
	SetMV("CU\0", 3);

	CreateMV("CC\0");
	SetMV("CC\0", -1);
	
	CreateMV("TT\0");
	SetMV("TT\0", -1);
	
	CreateMV("CN\0");
	SetMV("CN\0", -1);

	CreateMV("CG\0");
	SetMV("CG\0", -1);

        CreateMV("NTT\0");
        SetMV("NTT\0", 0);
        CreateMV("SIM\0");
        SetMV("SIM\0", 1);

	CreateMV("Z\0");
	SetMV("Z\0", 0);
			
	Exec("../test/man", 11, 16); /* starting manager*/

	CreateMV("EP1\0");
	CreateMV("EP2\0");
	SetMV("EP1\0", 0);
	SetMV("EP2\0", 0);

	while(GetMV("Z\0") == 0)
	{
		 Yield();
		 Yield();
		 Yield();
		 Yield();
		 Yield();
	}
	Exec("../test/tc", 10, 2); /*starting ticket clerk*/
	Exec("../test/cc", 10, 13); /*starting concession clerk*/
	Exec("../test/tt", 10, 14); /*starting ticket taker*/
	Exec("../test/mt", 10, 15); /*starting movie technician*/ 
	for(i = 3; i < 13; i++)
	{
		Exec("../test/cust", 12, i); /*starting customers*/
	}
	
	Exit(0);	
	
}
