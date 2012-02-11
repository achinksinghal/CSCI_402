#include "syscall.h"
#include "movietheature_main.h"

/*extern void ThreadTest_ForPart1();*/
/*extern void SelfTest1_ForPart1();*/

/*struct DebugControlBlock debugCb;*/

int gDebugFileNo=1;
char *printLock;
/*
 * DEBUG LEVEL OPTION
 * change it to maximum 
 * 255 to get 
 * logs.
 * */
int gDebugLevel=1;
#define NULL 0
/* All The Locks and condition 
 * variables Which Are 
 * Required in the whole simulation
 * */
char *mtLock;
char *custLock[MAX_CUST_GRP][MAX_CUST];

char *mtCondVar;
char *custCondVar[MAX_CUST_GRP][MAX_CUST];
char print_lockName[25];
/* Movie Theature control 
 * block
 * */
MtCb mtCb;
int sim = 1;
MV simu;

void YIELD()
{
	Yield();
}
void initialize()
{
	print_lockName[0]='P';
	print_lockName[1]='R';
	print_lockName[2]='\0';
	print_lockName[3]='\0';
	printLock = print_lockName;
	CreateLock(printLock);
}

void createMv(MV *mv)
{
	mv->index = CreateMV(mv->name);

}

void setMv(MV *mv, int value)
{
	SetMV(mv->name, value);
	mv->value = value;
}

int getMv(MV *mv)
{
	mv->value= GetMV(mv->name);

	return mv->value;
}

void copy_string(char *destination, int destination_length, char const *source, int source_length, int part_1, int part_2)
{
        int i=0,j=0;
        for( i=0; i<destination_length; i++ )
        {
                if(j < source_length)
                {
                        destination[i] = source[j];
                }
                if( j == source_length )
                {
                        destination[i]= '_';
                        if(part_1 != -1)
                        {
                                destination[++i]= (char)(part_1 + 'a');
                                destination[++i]= '_';
                        }
                        destination[++i]= (char)(part_2 + '0');
                        destination[++i]= '\0';
                        destination[++i]= '\0';
                        break;
                }
                j++;
        }
}


 /*
 * Function is to obtain current state of movie technician
 * */
int getMtState(int debugId)
{
	int state;
	AcquireLock(mtLock);   /*get lock before we get it*/
	state = getMv(&mtCb.mt.state);
	ReleaseLock(mtLock); /*release the lock */
	return state;
}

/*
 * Function is to change state of movie technician
 * */
void changeMtState(int debugId, int state)
{
	AcquireLock(mtLock); /*acquire a lock before we do it*/
	setMv(&mtCb.mt.state, state);
	
	if (state == STARTED_BY_MAN) /*Mtechnician has been asked to start working */
	{
	}
	else if (state == MOVIE_IS_NOT_PLAYING) /*Mtechnician is currently working*/
	{
		setMv(&mtCb.mt.msgToMan, MOVIE_OVER);
		WaitCV(mtLock, mtCondVar);
	}
	else if ( state == MOVIE_IS_PLAYING || state == NO_MOVIE_REQD )
	{
		SignalCV(mtLock, mtCondVar);		
		setMv(&mtCb.mt.msgToMan, STARTING_MOVIE);
	}
	
	ReleaseLock(mtLock); /*release the lock */
}

/*
 * obtain current state of customer - 
 * We acquire a lock before seeing it 
 * and release lock once we are done
 * */
int getCustState(int debugId, int custId, int grpId)
{
	int state;
	AcquireLock(custLock[grpId][custId]); 
	state = getMv(&mtCb.custGrp[grpId].cust[custId].state);
	ReleaseLock(custLock[grpId][custId]); 
	return state;
}

/*
 * we set customer states here 
 * and call the associated wait 
 * and signal on CV as required
 * */
void changeCustState(int debugId, int custId, int grpId, int state)
{
	int currentState;
	AcquireLock(custLock[grpId][custId]);
	currentState = getMv(&mtCb.custGrp[grpId].cust[custId].state);
	setMv(&mtCb.custGrp[grpId].cust[custId].state, state);

       /*reactions to different states*/
	if(
			state == WAIT || 
			state == WAIT_FOR_TC || 
			state == WAIT_FOR_TICKET_BUYER_FROM_TC ||
			state == WAIT_ENGAGED_WITH_TC || 
			state == WAIT_FOR_CC || 
			state == WAIT_FOR_TICKET_BUYER_FROM_CC ||
			state == WAIT_ENGAGED_WITH_CC || 
			state == WAIT_FOR_CC_BEING_FIRST || 
			state == WAIT_FOR_TC_BEING_FIRST || 
			state == WAIT_IN_LOBBY_AS_TICKET_NOT_TAKEN_BY_TT || 
			state == WAIT_FOR_TICKET_BUYER_FROM_TT ||
			state == WAIT_FOR_TICKET_BUYER_TILL_SEATS_TAKEN || 
			state == WAIT_AS_TICKET_BUYER_HEADING_FOR_BATHROOM || 
			state == WAIT_AS_OVER || 
			state == SEAT_TAKEN 
	  )
	{
		WaitCV(custCondVar[grpId][custId], custLock[grpId][custId]);/*if the state is set to be some sort of WAIT we call wait on CV*/
	}
	else if(
			state == SIGNAL_BY_TC || 
			state == SIGNAL || 
			state == SIGNAL_ENGAGED_WITH_TC ||
			state == SIGNAL_BY_CC || 
			state == SIGNAL_ENGAGED_WITH_CC || 
			state == READY_TO_LEAVE_MOVIE_ROOM || 
			state == NEW_MOVIE_STARTING_TICKET_MAY_BE_TAKEN || 
			state == SIGNAL_TO_CHANGE_TO_ANY_OTHER_QUEUE 
	       )
	{
		SignalCV(custCondVar[grpId][custId], custLock[grpId][custId]);/*we stop waiting and signal customer to wake up*/
	}
	ReleaseLock(custLock[grpId][custId]); 
}

/*
 * this will initialize the locks and condition variables that we created for each queues, manager, ticker clerks, concession clerks, tikcet takers, customers
 * */
void initializeAllLocks(int debugId)
{
	int i,j;
	i=0, j=0; /*char *lockName;*/

	for(i=0; i<mtCb.numCustGrp; i++)
	{ /*we start with the customers and create locks and CVs for them*/
		for(j=0; j<mtCb.custGrp[i].numCust; j++)
		{
			custLock[i][j] = mtCb.custGrp[i].cust[j].name;
			CreateLock(mtCb.custGrp[i].cust[j].name);
			custCondVar[i][j] = mtCb.custGrp[i].cust[j].name;
			CreateCV(mtCb.custGrp[i].cust[j].name);
		}
	}

/*Now initialising individual locks and CVs for:*/
	/*---movie technician*/
	mtLock = mtCb.mt.name;
	CreateLock(mtCb.mt.name);
	mtCondVar = mtCb.mt.name;
	CreateCV(mtCb.mt.name);
}

/*
 * we'll be initializing all values,
 * assigning ids to all identities
 * and allocating memory to these.
 * */
void initializeAllValues(int debugId)
{
	int i=0, j=0;
	copy_string(mtCb.man.name, 20,"MA", 2, -1, 1);
	mtCb.man.state.name[0] = 'S';
	mtCb.man.state.name[1] = 'M';
	mtCb.man.state.name[2] = 'A';
	mtCb.man.state.name[3] = 'N';
	mtCb.man.state.name[4] = '\0';
	mtCb.man.msgToAll.name[0] = 'M';
	mtCb.man.msgToAll.name[1] = 'M';
	mtCb.man.msgToAll.name[2] = 'T';
	mtCb.man.msgToAll.name[3] = 'A';
	mtCb.man.msgToAll.name[4] = '\0';
	createMv(&mtCb.man.state);
	createMv(&mtCb.man.msgToAll);
	
	copy_string(mtCb.mt.name, 20,"MT", 2, -1, 1);
	mtCb.mt.state.name[0] = 'S';
	mtCb.mt.state.name[1] = 'M';
	mtCb.mt.state.name[2] = 'T';
	mtCb.mt.state.name[3] = '\0';
	mtCb.mt.msgToMan.name[0] = 'M';
	mtCb.mt.msgToMan.name[1] = 'M';
	mtCb.mt.msgToMan.name[2] = 'T';
	mtCb.mt.msgToMan.name[3] = 'M';
	mtCb.mt.msgToMan.name[4] = '\0';
	mtCb.mt.msgByMan.name[0] = 'M';
	mtCb.mt.msgByMan.name[1] = 'M';
	mtCb.mt.msgByMan.name[2] = 'B';
	mtCb.mt.msgByMan.name[3] = 'M';
	mtCb.mt.msgByMan.name[4] = '\0';
	createMv(&mtCb.mt.state);
	createMv(&mtCb.mt.msgToMan);
	createMv(&mtCb.mt.msgByMan);
	setMv(&mtCb.mt.msgToMan, INVALID_MSG);

	for(i=0; i<MAX_ROWS; i++)
	{
		for(j=0; j<MAX_COLS; j++)
		{
			mtCb.mvRoom.seat[i][j].custId.name[0] = 'S';
			mtCb.mvRoom.seat[i][j].custId.name[1] = 'C';
			mtCb.mvRoom.seat[i][j].custId.name[2] = 'U';
			mtCb.mvRoom.seat[i][j].custId.name[3] = 'I';
			mtCb.mvRoom.seat[i][j].custId.name[4] = '0' + i;
			mtCb.mvRoom.seat[i][j].custId.name[5] = '0' + j;
			mtCb.mvRoom.seat[i][j].custId.name[6] = '\0';
			mtCb.mvRoom.seat[i][j].custGrpId.name[0] = 'S';
			mtCb.mvRoom.seat[i][j].custGrpId.name[1] = 'C';
			mtCb.mvRoom.seat[i][j].custGrpId.name[2] = 'G';
			mtCb.mvRoom.seat[i][j].custGrpId.name[3] = 'I';
			mtCb.mvRoom.seat[i][j].custGrpId.name[4] = '0' + i;
			mtCb.mvRoom.seat[i][j].custGrpId.name[5] = '0' + j;
			mtCb.mvRoom.seat[i][j].custGrpId.name[6] = '\0';
			mtCb.mvRoom.seat[i][j].isTaken.name[0] = 'S';
			mtCb.mvRoom.seat[i][j].isTaken.name[1] = 'I';
			mtCb.mvRoom.seat[i][j].isTaken.name[2] = 'T';
			mtCb.mvRoom.seat[i][j].isTaken.name[3] = '0' + i;
			mtCb.mvRoom.seat[i][j].isTaken.name[4] = '0' + j;
			mtCb.mvRoom.seat[i][j].isTaken.name[5] = '\0';
			createMv(&mtCb.mvRoom.seat[i][j].custId);
			createMv(&mtCb.mvRoom.seat[i][j].custGrpId);
			createMv(&mtCb.mvRoom.seat[i][j].isTaken);
		}
	}	
	if(sim == 1 || sim == 3)
        mtCb.numCust = 30; 
        if(sim == 2)
        mtCb.numCust = 10;


	mtCb.numCustGrp=0;	
	i=mtCb.numCust;
	while(i > 0)
	{
		/*
		Each group can only have upto 5 members.
		 So we generate a Random number and 
		allocate that number to first group. 
		Next we reduce the total number of
		customers left to be allocated groups and 
		go back to start of loop to put
		more ppl in other groups.
		*/

		int randomNum = 5; 
		if(i <= 5)
		{
			randomNum = 5;
		}
		i = i- randomNum;
				
		mtCb.custGrp[mtCb.numCustGrp].grpId = mtCb.numCustGrp;
		mtCb.custGrp[mtCb.numCustGrp].numCust = randomNum; 
		copy_string(mtCb.custGrp[mtCb.numCustGrp].name, 20,"CG", 2, -1, mtCb.numCustGrp);
		mtCb.custGrp[mtCb.numCustGrp].location.name[0] = 'L';
		mtCb.custGrp[mtCb.numCustGrp].location.name[1] = 'C';
		mtCb.custGrp[mtCb.numCustGrp].location.name[2] = 'G';
		mtCb.custGrp[mtCb.numCustGrp].location.name[3] = '0' + mtCb.custGrp[mtCb.numCustGrp].grpId;
		mtCb.custGrp[mtCb.numCustGrp].location.name[4] = '\0';
		createMv(&mtCb.custGrp[mtCb.numCustGrp].location);
		mtCb.custGrp[mtCb.numCustGrp].state.name[0] = 'S';
		mtCb.custGrp[mtCb.numCustGrp].state.name[1] = 'C';
		mtCb.custGrp[mtCb.numCustGrp].state.name[2] = 'G';
		mtCb.custGrp[mtCb.numCustGrp].state.name[3] = '0' + mtCb.custGrp[mtCb.numCustGrp].grpId;
		mtCb.custGrp[mtCb.numCustGrp].state.name[4] = '\0';
		createMv(&mtCb.custGrp[mtCb.numCustGrp].state);

		for(j=0; j<mtCb.custGrp[mtCb.numCustGrp].numCust; j++)
		{
			mtCb.custGrp[mtCb.numCustGrp].cust[j].selfId = j;
			mtCb.custGrp[mtCb.numCustGrp].cust[j].grpId = mtCb.numCustGrp;
			mtCb.custGrp[mtCb.numCustGrp].cust[j].money = CUST_MONEY;
			mtCb.custGrp[mtCb.numCustGrp].cust[j].IAMTicketBuyer = 0;
			copy_string(mtCb.custGrp[mtCb.numCustGrp].cust[j].name, 20,"CU", 2, mtCb.numCustGrp, j);
			mtCb.custGrp[mtCb.numCustGrp].cust[j].state.name[0] = 'S';
			mtCb.custGrp[mtCb.numCustGrp].cust[j].state.name[1] = 'C';
			mtCb.custGrp[mtCb.numCustGrp].cust[j].state.name[2] = 'U';
			mtCb.custGrp[mtCb.numCustGrp].cust[j].state.name[3] = '0' + j;
			mtCb.custGrp[mtCb.numCustGrp].cust[j].state.name[4] = '0' + mtCb.numCustGrp;
			mtCb.custGrp[mtCb.numCustGrp].cust[j].state.name[5] = '\0';
			createMv(&mtCb.custGrp[mtCb.numCustGrp].cust[j].state);

			mtCb.custGrp[mtCb.numCustGrp].cust[j].location.name[0] = 'L';
			mtCb.custGrp[mtCb.numCustGrp].cust[j].location.name[1] = 'C';
			mtCb.custGrp[mtCb.numCustGrp].cust[j].location.name[2] = 'U';
			mtCb.custGrp[mtCb.numCustGrp].cust[j].location.name[3] = '0' + j;
			mtCb.custGrp[mtCb.numCustGrp].cust[j].location.name[4] = '0' + mtCb.numCustGrp;
			mtCb.custGrp[mtCb.numCustGrp].cust[j].location.name[5] = '\0';
			createMv(&mtCb.custGrp[mtCb.numCustGrp].cust[j].location);

			mtCb.custGrp[mtCb.numCustGrp].cust[j].seatI.name[0] = 'S';
			mtCb.custGrp[mtCb.numCustGrp].cust[j].seatI.name[1] = 'I';
			mtCb.custGrp[mtCb.numCustGrp].cust[j].seatI.name[2] = 'C';
			mtCb.custGrp[mtCb.numCustGrp].cust[j].seatI.name[3] = '0' + j;
			mtCb.custGrp[mtCb.numCustGrp].cust[j].seatI.name[4] = '0' + mtCb.numCustGrp;
			mtCb.custGrp[mtCb.numCustGrp].cust[j].seatI.name[5] = '\0';
			createMv(&mtCb.custGrp[mtCb.numCustGrp].cust[j].seatI);

			mtCb.custGrp[mtCb.numCustGrp].cust[j].seatJ.name[0] = 'S';
			mtCb.custGrp[mtCb.numCustGrp].cust[j].seatJ.name[1] = 'J';
			mtCb.custGrp[mtCb.numCustGrp].cust[j].seatJ.name[2] = 'C';
			mtCb.custGrp[mtCb.numCustGrp].cust[j].seatJ.name[3] = '0' + j;
			mtCb.custGrp[mtCb.numCustGrp].cust[j].seatJ.name[4] = '0' + mtCb.numCustGrp;
			mtCb.custGrp[mtCb.numCustGrp].cust[j].seatJ.name[5] = '\0';
			createMv(&mtCb.custGrp[mtCb.numCustGrp].cust[j].seatJ);

			mtCb.custGrp[mtCb.numCustGrp].cust[j].seatTaken.name[0] = 'S';
			mtCb.custGrp[mtCb.numCustGrp].cust[j].seatTaken.name[1] = 'E';
			mtCb.custGrp[mtCb.numCustGrp].cust[j].seatTaken.name[2] = 'T';
			mtCb.custGrp[mtCb.numCustGrp].cust[j].seatTaken.name[3] = '0' + j;
			mtCb.custGrp[mtCb.numCustGrp].cust[j].seatTaken.name[4] = '0' + mtCb.numCustGrp;
			mtCb.custGrp[mtCb.numCustGrp].cust[j].seatTaken.name[5] = '\0';
			createMv(&mtCb.custGrp[mtCb.numCustGrp].cust[j].seatTaken);
		}
		mtCb.custGrp[mtCb.numCustGrp].cust[0].IAMTicketBuyer = 1;
		mtCb.numCustGrp++;
	}

}

/*
 * Movie Technician Main's Program
 * */
void mtMain()
{
	int ret_val=0;
	int selfId;
	int debugId;
	MT *self;/*creating pointer to self*/
	Cust *curentCust;
	int currentState;
	int i=0, j=0, num=0;
	int custState;
	int movieDuration;
	int movieIterations = 0;

	simu.name[0] = 'S';
        simu.name[1] = 'I';
        simu.name[2] = 'M';
        simu.name[4] = '\0';
        createMv(&simu);
	sim = getMv(&simu);

	debugId = selfId+500;

	self = &mtCb.mt;/*creating pointer to self*/
	changeMtState(debugId, STARTED_BY_MAN);/*he is now ready to start movie*/

	while(1)
	{
		/*end_check(debugId);*/
		currentState = getMtState(debugId); /*get state intially*/
		if( getMtState(debugId) == STARTED_BY_MAN)
		{/*if state is started by manager*/
			YIELD();
		}
		if(getMtState(debugId) == NO_MOVIE_REQD )
		{ /* state is that everyone has seen movie*/
			return;
		}
		if((getMtState(debugId) == MOVIE_IS_PLAYING) && (getMv(&mtCb.man.msgToAll) == MOVIE_RUNNING))
		{/*if a movie is now to be played*/

			/*The customers have now been seated*/
			/*Movie is now being in played*/
			/*Play movie by calling Thread->Yield for some Random time between 200 and 300 */

			movieDuration = Random();
			movieDuration %= 100;
			movieDuration += 200;

			for(i=0; i<MAX_ROWS; i++)/*Making sure everyone is seated*/
			{
				for(j=0; j<MAX_COLS; j++) /*These customers have seen the movie they now ought to leave*/
				{
					int currentCustId = getMv(&mtCb.mvRoom.seat[i][j].custId);
					int currentCustGrpId = getMv(&mtCb.mvRoom.seat[i][j].custGrpId);
					if( currentCustId != -1 && currentCustGrpId != -1 )
					while(getCustState(debugId, currentCustId, currentCustGrpId) != SEAT_TAKEN)
					{
						YIELD();
						YIELD();
					}
				}
			}
			Print("The MovieTechnician has started the movie\n",-1,-1,-1);
	
			for(i=0;i<movieDuration;i++)
			{/*the movie is beingplayed by calling thread->yields				*/
				YIELD();
			}	
			Print("The MovieTechnician has ended the movie\n",-1,-1,-1);
			/*The movie is now over.. Have to tell manager*/
			/*The movie is now over so changing back status of ticket owners to READY_TO_LEAVE_MOVIE_ROOM*/
			for(i=0; i<MAX_ROWS; i++)
			{
				for(j=0; j<MAX_COLS; j++) /*These customers have seen the movie they now ought to leave*/
				{
					int currentCustId = getMv(&mtCb.mvRoom.seat[i][j].custId);
					int currentCustGrpId = getMv(&mtCb.mvRoom.seat[i][j].custGrpId);
					if( currentCustId != -1 && currentCustGrpId != -1 )
					if( getMv(&mtCb.custGrp[currentCustGrpId].cust[currentCustId].seatTaken) == 1 && getCustState(debugId, currentCustId, currentCustGrpId) == SEAT_TAKEN)
					{
						changeCustState(debugId, currentCustId, currentCustGrpId, READY_TO_LEAVE_MOVIE_ROOM);    /*customers shall be awoken here - SIGNAL them*/
					}
				}
			}
			Print("The MovieTechnician has told customers to leave the theater room\n",-1,-1,-1);

			if( movieIterations == 1 && (sim == 1 || sim == 3 || sim == 4))
			{
				AcquireLock(mtLock); /*acquire a lock before we do it*/
				setMv(&mtCb.mt.state, MOVIE_IS_NOT_PLAYING);
				ReleaseLock(mtLock); /*acquire a lock before we do it*/
				return;
			}
			if( movieIterations == 0 && (sim == 2))
			{
				AcquireLock(mtLock); /*acquire a lock before we do it*/
				setMv(&mtCb.mt.state, MOVIE_IS_NOT_PLAYING);
				ReleaseLock(mtLock); /*acquire a lock before we do it*/
				return;
			}
			movieIterations++;
			changeMtState(debugId, MOVIE_IS_NOT_PLAYING);
			
			YIELD();
		}
		YIELD();
	}	
}

/*
 * This is the first function that is called 
 * when the customised simulation is run.
 * */
void main()
{
	/*Setting up debugging information*/
	int debugId=1;
	initialize();
	initializeAllValues(debugId); /*intializing entities that shall be part of simulation*/
	initializeAllLocks(debugId);/*set up required locks*/
	mtMain();
	Exit(0);
}
