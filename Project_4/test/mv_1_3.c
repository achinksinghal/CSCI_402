#include "syscall.h"

/*This file is part of testcase of movie theater
 * simulation with 30 customers, 3 ticket taker
 * 3 concession clerk, 3 ticket taker, a movie
 * technician and a manager.
 */
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

        CreateMV("Z\0");

        while(GetMV("Z\0") == 0)
        {
                 Yield();
                 Yield();
                 Yield();
                 Yield();
                 Yield();
        }

        CreateMV("EP2\0");
        while(GetMV("EP2\0") == 0)
        {
                 Yield();
                 Yield();
        }
	
	Exec("../test/tc", 10, 34);/*starting ticket clerk*/
	Exec("../test/cc", 10, 45);/*starting concession clerk*/
	Exec("../test/tt", 10, 46);/*starting ticket taker*/
	for(i = 35; i < 45; i++)
	{
		Exec("../test/cust", 12, i);/*starting customers*/
	}
	
	Exit(0);	
	
}
