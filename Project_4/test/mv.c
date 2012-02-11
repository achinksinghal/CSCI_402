#include "syscall.h"

int mv=-1;
int lck=-1;
int lck2=-1;
int cv=-1;
int ret=-1;
int mv2=-1;

void main()
{

	int num_tc;
	int tc_num;
	int cc_num;
	int tt_num;
	int num_cust;
	int cust_num;
	int custGrp_num;
	int i=0;
	int value;
	int nTT;
	int sim;
	int start;

	num_tc = CreateMV("TC\0");
	value = SetMV("TC\0", 1);
	tc_num = CreateMV("TCN\0");
	value = SetMV("TCN\0", -1);

	num_cust = CreateMV("CU\0");
	value  = SetMV("CU\0", 3);

	cc_num = CreateMV("CC\0");
	value = SetMV("CC\0", -1);
	
	tt_num = CreateMV("TT\0");
	value = SetMV("TT\0", -1);
	
	cust_num = CreateMV("CN\0");
	value = SetMV("CN\0", -1);

	custGrp_num = CreateMV("CG\0");
	value = SetMV("CG\0", -1);

        nTT = CreateMV("NTT\0");
        value = SetMV("NTT\0", 0);
        sim = CreateMV("SIM\0");
        value = SetMV("SIM\0", 2);

	start = CreateMV("Z\0");
	value = SetMV("Z\0", 0);
		
	Exec("../test/man", 11, 16);

	while(GetMV("Z\0") == 0)
	{
		 Yield();
		 Yield();
		 Yield();
		 Yield();
		 Yield();
	}
	Exec("../test/tc", 10, 2);
	Exec("../test/cc", 10, 13);
	Exec("../test/tt", 10, 14);
	Exec("../test/mt", 10, 15);
	for(i = 3; i < 13; i++)
	{
		Exec("../test/cust", 12, i);
	}
	
	Exit(0);	
	
}
