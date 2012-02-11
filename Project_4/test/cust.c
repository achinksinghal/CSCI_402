#include "syscall.h"
#include "movietheature_main.h"

/*extern void ThreadTest_ForPart1();*/
/*extern void SelfTest1_ForPart1();*/

/*struct DebugControlBlock debugCb;*/

int gDebugFileNo=1;
char *printLock;
char *custNumLock;
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
char *manLock;
char *ticketTaking;
char *tcLock[MAX_TC];
char *ttLock[MAX_TT];
char *ccLock[MAX_CC];
char *seats;
char *mtLock;
char *custLock[5];
char *custCondVar[5];
char *queueLock[MAX_QUEUE_TYPE][MAX_NUM_QUEUE];
char *queueTcLock;
char *queueCcLock;
char *queueTtLock;
char *exitLock;
char *exitCV;
int cust_id = 0;
int grp_id = 0;
int id=0;
MV second_grp;
int sec_grp;
	
CustGrp custGrp;/*customer groups*/
char *manCondVar;
char *mtCondVar;
char *tcCondVar[MAX_TC];
char *ttCondVar[MAX_TT];
char *ccCondVar[MAX_CC];
char *queueCondVar[MAX_QUEUE_TYPE][MAX_NUM_QUEUE];
/*
 * This are certain variables
 * required for specific test 
 * cases
 * */
int IsSeatsOccupied;
int money[2][5];

char *arg_lock;
char print_lockName[25];
int thread_arg;
/* Movie Theature control 
 * block
 * */
MtCb mtCb;

/*wrappers for 
 * monitor variables*/
void createMv(MV *mv)
{
	CreateMV(mv->name);
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

void end_check(int debugId)
{
	int i=0;
	
	for (i=0; i<mtCb.numCustGrp; i++)
	{
		if(getCustGrpLocation(debugId, i) == OUT_OF_MOVIETHEATURE);
		else break;
	}
	if( i == mtCb.numCustGrp )
	{
		Exit(0);
	}
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
 * Function is to get ticket clerk state.
 * */
int getTcState(int debugId, int tcId)
{
	int state;
	state = getMv(&mtCb.tc[tcId].state);

	return state;
}

/*
 * broadcasting a CV for a ticket clerk on lock 
 * */
void broadcastAllTcLock( int debugId, int tcId1)
{
	BroadcastCV(tcLock[tcId1], tcCondVar[tcId1]); 
}

/*
 * this is enclosed Wait function on CV for a ticket clerk on lock
 * */
void waitOnTcLock( int debugId, int tcId1)
{
	WaitCV(tcLock[tcId1],tcCondVar[tcId1]); 
}

/*
 * enclosing call to Signal function of CV of ticket clerk on lock
 * */
void signalToTcLock(int debugId, int tcId1)
{
	SignalCV(tcLock[tcId1],tcCondVar[tcId1]); 
}

/*
 * enclosed call to acquire ticket clerks's lock
 * */
void acquireTcLock(int debugId, int tcId1)
{
	AcquireLock(tcLock[tcId1]); 
}

/*
 * enclosing a call to release lock of ticket clerk 
 * */
void releaseTcLock(int debugId, int tcId1)
{
	ReleaseLock(tcLock[tcId1]); 
}

/*
 * setting new value for state of ticket clerk  
 * */
void changeTcState(int debugId, int tcId, int state)
{
	setMv(&mtCb.tc[tcId].state, state);
}


/*
 * obtain current state of customer - 
 * We acquire a lock before seeing it 
 * and release lock once we are done
 * */
int getCustState(int debugId, int custId, int grpId)
{
	int state;
	AcquireLock(custLock[custId]); 
	state = getMv(&custGrp.cust[custId].state);
	ReleaseLock(custLock[custId]); 
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
	AcquireLock(custLock[custId]);
	currentState = getMv(&custGrp.cust[custId].state);
	setMv(&custGrp.cust[custId].state, state);


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
		WaitCV(custLock[custId],custCondVar[custId]);/*if the state is set to be some sort of WAIT we call wait on CV*/
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
		SignalCV(custLock[custId],custCondVar[custId]);/*we stop waiting and signal customer to wake up*/
	}
	ReleaseLock(custLock[custId]); 
}

/*
 * obtaining state of entire custmer group
 * */
int getCustGrpState(int debugId, int grpId)
{
	int state;
	state = getMv(&mtCb.custGrp[grpId].state);
	return state;
}

/*
 * setting location of particular customer
 * */
void changeCustLocation(int debugId, int custId, int grpId, int location)
{
	setMv(&custGrp.cust[custId].location, location);
}

/*
 * setting a new location for the customer group
 * */
void changeCustGrpLocation(int debugId, int grpId, int location)
{
	int i=0;
	setMv(&custGrp.location, location);
	for(i=0; i < 5; i++)
	{
		if(location == LOBBY)
		{ /*PRINT*/
			Print("Customer %d in Group %d is in the lobby\n", i, grpId,-1);
		}
		else if(location == MOVIEROOM)
		{/*PRINT*/
			Print("Customer %d in Group %d is leaving the lobby\n", i, grpId,-1);
		}
		changeCustLocation(debugId, i, grpId, location);/*we set location of all individual customers as that of the group*/
	}
}

/*
 * finding value of location for a customer group
 * */
int getCustGrpLocation(int debugId, int grpId)
{
	return getMv(&custGrp.location);
}

/*
 * finding vaalue of location for a particular customer
 * */
int getCustLocation(int debugId, int custId, int grpId)
{
	return getMv(&custGrp.cust[custId].location);
}

/*
 * we set the state of whole group together here
 * */
void changeCustGrpState(int debugId, int grpId, int state)
{
	int i=0;
	/*setMv(&custGrp.state, state);*/
	if(state == WAIT)
	{
	}
	else if(state == SIGNAL)
	{
	}
	else if( state == GOT_TICKET || state == GOT_FOOD || state == TICKET_TAKEN || SEATS_READY )
	{  /*once group head gets tickets or food or has ticket taken by ticket taker or find seats, he lets other know - using a signal*/
		for(i=1; i < 5; i++)
		{
			/*if(!custGrp.cust[i].IAMTicketBuyer)*/
			{
				changeCustState(debugId, i, grpId, state);
				AcquireLock(custLock[i]);
				SignalCV(custLock[i],custCondVar[i]);
				ReleaseLock(custLock[i]);
			}
		}
	}
}

/*
 * Function is to obtain current concession clerk state
 * */
int getCcState(int debugId, int ccId)
{
	int state;
	state = getMv(&mtCb.cc[ccId].state);

	return state;
}

/*
 * Macro is broadcasting a CV for a concession clerk on lock
 * */
void broadcastAllCcLock(int debugId, int  ccId1)
{
	BroadcastCV( ccLock[ccId1], ccCondVar[ccId1]); 
}

/*
 * Macro is waiting on CV for a concession clerk on lock
 * */
void waitOnCcLock(int debugId, int ccId1)
{
	WaitCV( ccLock[ccId1], ccCondVar[ccId1]); 
}

/*
 * Macro is signalling concession clerk on CV with lock
 * */
void signalToCcLock(int debugId, int ccId1)
{
	SignalCV(ccLock[ccId1],ccCondVar[ccId1]); 
}

/*
 * Macro is acquiring concession clerk lock
 * */
void acquireCcLock(int debugId, int ccId1)
{
	AcquireLock(ccLock[ccId1]); 
}

/*
 * Macro is releasing concession clerk lock  
 * */
void releaseCcLock(int debugId, int ccId1)
{
	ReleaseLock(ccLock[ccId1]); 
}

/*
 * Macro is to change state of concession clerk
 * */
void changeCcState(int debugId,int ccId1,int state1)
{
	setMv(&mtCb.cc[ccId1].state, state1);
}


/*
 * enclosing a call to wait on CV of Ticket taker on lock
 * */
void waitOnTtLock(int debugId, int ttId)
{
	WaitCV(ttLock[ttId],ttCondVar[ttId]); 
	return ;
}

/*
 * enclosing a call to broadcast on CV on lock
 * */
void broadcastAllTtLock( int debugId, int ttId1)
{
	BroadcastCV(ttLock[ttId1], ttCondVar[ttId1]); 
}

/*
 * function enclosing a call to signal CV of Ticket taker number
 * */
void signalToTtLock(int debugId, int ttId)
{
	SignalCV(ttLock[ttId],ttCondVar[ttId]); 
	return ;
}

/*
 * function enclosing a call to acquire lock of Ticket taker number
 * */
void acquireTtLock(int debugId, int ttId)
{
	AcquireLock(ttLock[ttId]); 
	return ;
}

/*
 * function enclosing a call to release lock of Ticket taker number
 * */
void releaseTtLock(int debugId, int ttId)
{
	ReleaseLock(ttLock[ttId]); 
	return ;
}

/*
 * obtain ticket clerk current state
 * */
int getTtState(int debugId, int ttId)
{
	int state;
	state = getMv(&mtCb.tt[ttId].state);
	return state;
}

/*
 * setting new value for state of ticket taker  
 * */
void changeTtState(int debugId, int ttId, int state)
{
	setMv(&mtCb.tt[ttId].state, state);
}

/*
 * transferring ticket to customer here and add details to customer
 * */
void transferTicketFromTo(int debugId, TC *fromTc, Cust *toCust)
{
	int i=0;

	for(i=0; i<5; i++)
	{
		mtCb.custGrp[toCust->grpId].ticket[i].roomNum = 1; 
	}
}

/*
 * transferring food to customer and add details to customer
 * */
void transferFoodFromTo(int debugId, CC *fromCc, Cust *toCust)
{

}

/*
 * creates new List to use as queue 
 * */
void initializeQueue(int debugId, mtQueue *queue)
{
	queue->numCust.name[0] = 'N';
	queue->numCust.name[1] = 'Q';
	queue->numCust.name[2] = '0'+queue->queueId;
	queue->numCust.name[3] = '0'+queue->queueType;
	queue->numCust.name[4] = '\0';
	createMv(&queue->numCust);
	getMv(&queue->numCust);
	queue->state.name[0] = 'S';
	queue->state.name[1] = 'Q';
	queue->state.name[2] = '0'+queue->queueId;
	queue->state.name[3] = '0'+queue->queueType;
	queue->state.name[4] = '\0';
	createMv(&queue->state);
	getMv(&queue->state);
}

/*
 * adding members to a queue
 * first we get a lock on that queue
 * if there is no one in that queue - we let of lock
 * employee can't be on a break
 * then adding customer to requested queue
 * releasing lock
 * */
int queueAddCust(int debugId, mtQueue *queue, Cust *cust)
{
	AcquireLock(queueLock[queue->queueType][queue->queueId]);
	if(queue->queueAddress == NULL)
	{
		ReleaseLock(queueLock[queue->queueType][queue->queueId]);
		return NO_ADDRESS;
	}
	else if( queue->queueType == TC_QUEUE && getTcState(debugId, queue->queueId) == ON_BREAK )
	{ /*PRINT*/
		ReleaseLock(queueLock[queue->queueType][queue->queueId]);
		Print("Customer %d in Group %d sees TicketClerk %d is on break", cust->selfId, cust->grpId, queue->queueId );
		return NO_ADDRESS;
	}
	else if( queue->queueType == CC_QUEUE && getCcState(debugId, queue->queueId) == ON_BREAK )
	{ /*PRINT*/
		ReleaseLock(queueLock[queue->queueType][queue->queueId]);
		Print("Customer %d in Group %d sees ConcessionClerk %d is on break", cust->selfId, cust->grpId, queue->queueId );
		return NO_ADDRESS;
	}
	else if( queue->queueType == TT_QUEUE && getTtState(debugId, queue->queueId) == ON_BREAK )
	{ /*PRINT*/
		ReleaseLock(queueLock[queue->queueType][queue->queueId]);
		Print("Customer %d in Group %d sees TicketTaker %d is on break", cust->selfId, cust->grpId, queue->queueId );
		return NO_ADDRESS;
	}



	/*Print("ACHINTYA cust lock=%d cv=%d\n", queueLock[queue->queueType][queue->queueId], queueCondVar[queue->queueType][queue->queueId], -1 );*/
	getMv(&queue->numCust);
	setMv(&queue->numCust, queue->numCust.value + 1);
	WaitCV(queueLock[queue->queueType][queue->queueId],queueCondVar[queue->queueType][queue->queueId]);
	if( queue->queueType == TC_QUEUE )
	{
		setMv(&mtCb.tc[queue->queueId].currentCustId, cust->selfId);
		setMv(&mtCb.tc[queue->queueId].currentCustGrpId, cust->grpId);
		setMv(&mtCb.tc[queue->queueId].msgFromCust, CUST_REMOVED);
	}
	else if( queue->queueType == CC_QUEUE )
	{
		setMv(&mtCb.cc[queue->queueId].currentCustId, cust->selfId);
		setMv(&mtCb.cc[queue->queueId].currentCustGrpId, cust->grpId);
		setMv(&mtCb.cc[queue->queueId].msgFromCust, CUST_REMOVED);
	}
	else if( queue->queueType == TT_QUEUE )
	{
		setMv(&mtCb.tt[queue->queueId].currentCustId, cust->selfId);
		setMv(&mtCb.tt[queue->queueId].currentCustGrpId, cust->grpId);
		setMv(&mtCb.tt[queue->queueId].msgFromCust, CUST_REMOVED);
	}
	ReleaseLock(queueLock[queue->queueType][queue->queueId]);
	return ROK;
}

/*
 * this function is for linking the workers - tc,cc, and tt- to their queues 
 * */
void addAddressToQueue(int debugId, int id, int queueType, int queueId)
{
	if(queueType == TC_QUEUE)  
	{ /*associating ticket clerk with queue*/

		mtCb.tc[id].queue = &mtCb.queue[queueType][queueId];
		mtCb.tc[id].queue->queueType = queueType;
		mtCb.tc[id].queue->queueAddress = (Employee)&mtCb.tc[queueId];
	}
	else if(queueType == CC_QUEUE)  
	{ /*associating concession clerk with queue*/

		mtCb.cc[id].queue = &mtCb.queue[queueType][queueId];
		mtCb.cc[id].queue->queueType = queueType;
		mtCb.cc[id].queue->queueAddress = (Employee)&mtCb.cc[queueId];
	} 
	else if(queueType == TT_QUEUE)  
	{ /*associating ticket taker with queue*/
		mtCb.tt[id].queue = &mtCb.queue[queueType][queueId];
		mtCb.tt[id].queue->queueType = queueType;
		mtCb.tt[id].queue->queueAddress = (Employee)&mtCb.tt[queueId];
	} 
}

/*
 * Opposite to upper one, it is to 
 * de-associating queues from workers
 * */
void removeAddressToQueue(int debugId, int id, int queueType, int queueId)
{
	AcquireLock(queueLock[queueType][queueId]); 
	if(queueType == TC_QUEUE)  
	{
		mtCb.tc[id].queue->queueType = INVALID_QUEUE;
		mtCb.tc[id].queue->queueAddress = NULL;
		/*mtCb.tc[id].queue = NULL;*/
	}
	else if(queueType == CC_QUEUE)  
	{ 
		mtCb.cc[id].queue->queueType = INVALID_QUEUE;
		mtCb.cc[id].queue->queueAddress = NULL;
		/*mtCb.cc[id].queue = NULL;*/
	} 
	ReleaseLock(queueLock[queueType][queueId]); 
}

/*
 * finding total number of customer in particular queue 
 * */
int getTotalCustCount(int debugId, mtQueue *queue)
{
	int count;
	AcquireLock(queueLock[queue->queueType][queue->queueId]);
	count=getMv(&queue->numCust);
	ReleaseLock(queueLock[queue->queueType][queue->queueId]);
	return count ;
}

/*
 * We'll setup all the queues that
 * need to be part of simulation
 * */
void initializeAllQueues(int debugId )
{
	int i=0;
	i= (grp_id / 2) ;
	{
		mtCb.queue[TC_QUEUE][i].queueId = i;
		mtCb.queue[TC_QUEUE][i].queueType = TC_QUEUE;
		initializeQueue(debugId, &mtCb.queue[TC_QUEUE][i]);
		queueLock[TC_QUEUE][i] = mtCb.queue[TC_QUEUE][i].name;
		CreateLock(mtCb.queue[TC_QUEUE][i].name);

		queueCondVar[TC_QUEUE][i] = mtCb.queue[TC_QUEUE][i].name;
		
		CreateCV(mtCb.queue[TC_QUEUE][i].name);

		addAddressToQueue(debugId, i, TC_QUEUE, i);
	}
	i= (grp_id / 2) ;
	{
		mtCb.queue[CC_QUEUE][i].queueId = i;
		mtCb.queue[CC_QUEUE][i].queueType = CC_QUEUE;
		initializeQueue(debugId, &mtCb.queue[CC_QUEUE][i]);

		queueLock[CC_QUEUE][i] = mtCb.queue[CC_QUEUE][i].name;
		CreateLock(mtCb.queue[CC_QUEUE][i].name);
		
		queueCondVar[CC_QUEUE][i] = mtCb.queue[CC_QUEUE][i].name;
		CreateCV(mtCb.queue[CC_QUEUE][i].name);
		addAddressToQueue(debugId, i, CC_QUEUE, i);
	}
	i= (grp_id / 2) ;
	{
		mtCb.queue[TT_QUEUE][i].queueId = i;
		mtCb.queue[TT_QUEUE][i].queueType = TT_QUEUE;
		initializeQueue(debugId, &mtCb.queue[TT_QUEUE][i]);
		
		queueLock[TT_QUEUE][i] = mtCb.queue[TT_QUEUE][i].name;
		CreateLock(mtCb.queue[TT_QUEUE][i].name);

		queueCondVar[TT_QUEUE][i] = mtCb.queue[TT_QUEUE][i].name;
		CreateCV(mtCb.queue[TT_QUEUE][i].name);
		addAddressToQueue(debugId, i, TT_QUEUE, i);
	}


}

/*
 * this will initialize the locks and condition variables that we created for each queues, manager, ticker clerks, concession clerks, tikcet takers, customers
 * */
void initializeAllLocks(int debugId)
{
	int i,j;
	i=0, j=0; /*char *lockName;*/

	{ /*we start with the customers and create locks and CVs for them*/
		for(j=0; j < 5 ; j++)
		{
			/*if(cust_id == 0 || j == cust_id)*/
			{
				custLock[j] = custGrp.cust[j].name;
				CreateLock(custGrp.cust[j].name);

				custCondVar[j] = custGrp.cust[j].name;
				CreateCV(custGrp.cust[j].name);
			}
		}
	}

	i= (grp_id / 2 );
	if(cust_id == 0)
	{ /*locks and CVs for ticket clerks*/
		tcLock[i] = mtCb.tc[i].name;
		CreateLock(mtCb.tc[i].name);

		tcCondVar[i] = mtCb.tc[i].name; 
		CreateCV(mtCb.tc[i].name);
	}
	i= (grp_id / 2 );
	if(cust_id == 0)
	{/* locks and CVs for concession clerks */

		ccLock[i] = mtCb.cc[i].name;
		CreateLock(mtCb.cc[i].name);

		ccCondVar[i] = mtCb.cc[i].name;
		CreateCV(mtCb.cc[i].name);
	}
	i= (grp_id / 2 );
	if(cust_id == 0)
	{	/* locks and CVs for ticket takers*/

		ttLock[i] = mtCb.tt[i].name;
		CreateLock(mtCb.tt[i].name);

		ttCondVar[i] = mtCb.tt[i].name;
		CreateCV(mtCb.tt[i].name);
	}
	mtCb.ticketTaking_name[0] = 'T';
        mtCb.ticketTaking_name[1] = 't';
        mtCb.ticketTaking_name[2] = '\0';
        mtCb.ticketTaking_name[3] = '\0';
        ticketTaking = mtCb.ticketTaking_name;
		CreateLock(mtCb.ticketTaking_name);



}

/*
 * reset the number of tickets received by all ticket takers
 * */
void resetNumOfTicketTaken(int debugId)
{
	AcquireLock(ticketTaking);
	setMv(&mtCb.numOfTicketsTaken, 0);
	ReleaseLock(ticketTaking);
}

/*
 * tickets received by tt in current cycle
 * */
int getNumOfTicketTaken(int debugId)
{
	int value;
	AcquireLock(ticketTaking);
	value = getMv(&mtCb.numOfTicketsTaken);
	ReleaseLock(ticketTaking);
	return value;
}

/*
 * ticket taker has collected this many tickets
 * */
int increaseNumOfTicketTaken(int debugId, int value)
{
	int tickets;
	AcquireLock(ticketTaking);
	tickets = getMv(&mtCb.numOfTicketsTaken);

	if((tickets + value) <= MAX_SEATS)
	{
		setMv(&mtCb.numOfTicketsTaken, tickets + value);
		ReleaseLock(ticketTaking);
		return ROK;
	}
	else
	{
		ReleaseLock(ticketTaking);
		return NOK;
	}
}

/*
 * we'll be initializing 
 * all the variables for the customer
 * , assign id to identities
 * and allocating memory to these.
 * */
void initializeAllValues(int debugId)
{
	int i=0, j=0;
	int ID;
	ID = GetId();
	id = ID;
	cust_id =  ( ( ID - 3 ) % 16 ) % 5;


	grp_id = ((ID - 3) / 16 ) * 2 + (((ID - 3) % 16 ) / 5) ;
	cust_id =  cust_id % 5;	
	cust_number = cust_id;
	

	/*
	   Each group can only have upto 5 members.
	   So we generate a Random number and 
	   allocate that number to first group. 
	   Next we reduce the total number of
	   customers left to be allocated groups and 
	   go back to start of loop to put
	   more ppl in other groups.
	   */
	mtCb.numCustGrp = grp_id;
	custGrp.grpId = grp_id;

	custGrp.location.name[0] = 'L';
	custGrp.location.name[1] = 'C';
	custGrp.location.name[2] = 'G';
	custGrp.location.name[3] = '0' + custGrp.grpId;
	custGrp.location.name[4] = '\0';
	createMv(&custGrp.location);
	setMv(&custGrp.location, START);

	custGrp.state.name[0] = 'S';
	custGrp.state.name[1] = 'C';
	custGrp.state.name[2] = 'G';
	custGrp.state.name[3] = '0' + custGrp.grpId;
	custGrp.state.name[4] = '\0';
	createMv(&custGrp.state);


	copy_string(custGrp.name, 20,"CG", 2, -1, custGrp.grpId);
	for(j=0; j < 5; j++)
	{
		/*if( cust_id == 0 || j == cust_id || j == 0)*/
		{
			custGrp.cust[j].selfId = j;
			custGrp.cust[j].grpId = custGrp.grpId;
			custGrp.cust[j].money = CUST_MONEY;
			custGrp.cust[j].IAMTicketBuyer = 0;
			copy_string(custGrp.cust[j].name, 20,"CU", 2, custGrp.grpId, j);

			custGrp.cust[j].state.name[0] = 'S';
			custGrp.cust[j].state.name[1] = 'C';
			custGrp.cust[j].state.name[2] = 'U';
			custGrp.cust[j].state.name[3] = '0' + j;
			custGrp.cust[j].state.name[4] = '0' + mtCb.numCustGrp;
			custGrp.cust[j].state.name[5] = '\0';
			createMv(&custGrp.cust[j].state);

			custGrp.cust[j].location.name[0] = 'L';
			custGrp.cust[j].location.name[1] = 'C';
			custGrp.cust[j].location.name[2] = 'U';
			custGrp.cust[j].location.name[3] = '0' + j;
			custGrp.cust[j].location.name[4] = '0' + mtCb.numCustGrp;
			custGrp.cust[j].location.name[5] = '\0';
			createMv(&custGrp.cust[j].location);
			setMv(&custGrp.cust[j].location, START);

			custGrp.cust[j].seatI.name[0] = 'S';
			custGrp.cust[j].seatI.name[1] = 'I';
			custGrp.cust[j].seatI.name[2] = 'C';
			custGrp.cust[j].seatI.name[3] = '0' + j;
			custGrp.cust[j].seatI.name[4] = '0' + mtCb.numCustGrp;
			custGrp.cust[j].seatI.name[5] = '\0';
			createMv(&custGrp.cust[j].seatI);
			setMv(&custGrp.cust[j].seatI, -1);

			custGrp.cust[j].seatJ.name[0] = 'S';
			custGrp.cust[j].seatJ.name[1] = 'J';
			custGrp.cust[j].seatJ.name[2] = 'C';
			custGrp.cust[j].seatJ.name[3] = '0' + j;
			custGrp.cust[j].seatJ.name[4] = '0' + mtCb.numCustGrp;
			custGrp.cust[j].seatJ.name[5] = '\0';
			createMv(&custGrp.cust[j].seatJ);
			setMv(&custGrp.cust[j].seatJ, -1);

			custGrp.cust[j].seatTaken.name[0] = 'S';
			custGrp.cust[j].seatTaken.name[1] = 'E';
			custGrp.cust[j].seatTaken.name[2] = 'T';
			custGrp.cust[j].seatTaken.name[3] = '0' + j;
			custGrp.cust[j].seatTaken.name[4] = '0' + mtCb.numCustGrp;
			custGrp.cust[j].seatTaken.name[5] = '\0';
			createMv(&custGrp.cust[j].seatTaken);
			setMv(&custGrp.cust[j].seatTaken, 0);
		}
	}
	custGrp.cust[0].IAMTicketBuyer = 1;

	if(grp_id == 1 || grp_id == 3 || grp_id == 5 )
	{
			second_grp.name[0] = 'S';
			second_grp.name[1] = 'C';
			second_grp.name[2] = 'U';
			second_grp.name[3] = '0';
			second_grp.name[4] = '0' + (grp_id -1);
			second_grp.name[5] = '\n';
			createMv(&second_grp);
	}

	mtCb.numTC = 3;
	
	i= (grp_id / 2 );
	if(cust_id == 0)
	{
		copy_string(mtCb.queue[TC_QUEUE][i].name, 20,"QQ", 2, 0, i);
		copy_string(mtCb.tc[i].name, 20,"TC", 2, -1, i);

		mtCb.tc[i].state.name[0] = 'S';
		mtCb.tc[i].state.name[1] = 'T';
		mtCb.tc[i].state.name[2] = 'C';
		mtCb.tc[i].state.name[3] = '0' + i;
		mtCb.tc[i].state.name[4] = '\0';
		mtCb.tc[i].currentCustId.name[0] = 'C';
		mtCb.tc[i].currentCustId.name[1] = 'C';
		mtCb.tc[i].currentCustId.name[2] = 'U';
		mtCb.tc[i].currentCustId.name[3] = '0' + i;
		mtCb.tc[i].currentCustId.name[4] = 'T';
		mtCb.tc[i].currentCustId.name[5] = '\0';
		mtCb.tc[i].currentCustGrpId.name[0] = 'C';
		mtCb.tc[i].currentCustGrpId.name[1] = 'C';
		mtCb.tc[i].currentCustGrpId.name[2] = 'G';
		mtCb.tc[i].currentCustGrpId.name[3] = '0' + i;
		mtCb.tc[i].currentCustGrpId.name[4] = 'T';
		mtCb.tc[i].currentCustGrpId.name[5] = '\0';
		mtCb.tc[i].msgFromCust.name[0] = 'M';
		mtCb.tc[i].msgFromCust.name[1] = 'C';
		mtCb.tc[i].msgFromCust.name[2] = 'U';
		mtCb.tc[i].msgFromCust.name[3] = '0' + i;
		mtCb.tc[i].msgFromCust.name[4] = 'T';
		mtCb.tc[i].msgFromCust.name[5] = '\0';
		mtCb.tc[i].msgBuffer.name[0] = 'M';
		mtCb.tc[i].msgBuffer.name[1] = 'B';
		mtCb.tc[i].msgBuffer.name[2] = '0' + i;
		mtCb.tc[i].msgBuffer.name[3] = 'T';
		mtCb.tc[i].msgBuffer.name[4] = '\0';
		createMv(&mtCb.tc[i].state);
		createMv(&mtCb.tc[i].currentCustId);
		createMv(&mtCb.tc[i].currentCustGrpId);
		createMv(&mtCb.tc[i].msgFromCust);
		createMv(&mtCb.tc[i].msgBuffer);
	}

	mtCb.numCC = 3;

	i= (grp_id / 2 );
	if(cust_id == 0)
	{
		copy_string(mtCb.queue[CC_QUEUE][i].name, 20,"QQ", 2, 1, i);
		copy_string(mtCb.cc[i].name, 20,"CC", 2, -1, i);

		mtCb.cc[i].state.name[0] = 'S';
		mtCb.cc[i].state.name[1] = 'C';
		mtCb.cc[i].state.name[2] = 'C';
		mtCb.cc[i].state.name[3] = '0' + i;
		mtCb.cc[i].state.name[4] = '\0';
		mtCb.cc[i].currentCustId.name[0] = 'C';
		mtCb.cc[i].currentCustId.name[1] = 'C';
		mtCb.cc[i].currentCustId.name[2] = 'U';
		mtCb.cc[i].currentCustId.name[3] = '0' + i;
		mtCb.cc[i].currentCustId.name[4] = 'C';
		mtCb.cc[i].currentCustId.name[5] = '\0';
		mtCb.cc[i].currentCustGrpId.name[0] = 'C';
		mtCb.cc[i].currentCustGrpId.name[1] = 'C';
		mtCb.cc[i].currentCustGrpId.name[2] = 'G';
		mtCb.cc[i].currentCustGrpId.name[3] = '0' + i;
		mtCb.cc[i].currentCustGrpId.name[4] = 'C';
		mtCb.cc[i].currentCustGrpId.name[5] = '\0';
		mtCb.cc[i].msgFromCust.name[0] = 'M';
		mtCb.cc[i].msgFromCust.name[1] = 'C';
		mtCb.cc[i].msgFromCust.name[2] = 'U';
		mtCb.cc[i].msgFromCust.name[3] = '0' + i;
		mtCb.cc[i].msgFromCust.name[4] = 'C';
		mtCb.cc[i].msgFromCust.name[5] = '\0';
		mtCb.cc[i].msgBuffer.name[0] = 'M';
		mtCb.cc[i].msgBuffer.name[1] = 'B';
		mtCb.cc[i].msgBuffer.name[2] = '0' + i;
		mtCb.cc[i].msgBuffer.name[3] = 'C';
		mtCb.cc[i].msgBuffer.name[4] = '\0';
		createMv(&mtCb.cc[i].state);
		createMv(&mtCb.cc[i].currentCustId);
		createMv(&mtCb.cc[i].currentCustGrpId);
		createMv(&mtCb.cc[i].msgFromCust);
		createMv(&mtCb.cc[i].msgBuffer);
	}

	i= (grp_id / 2 );
	if(cust_id == 0)
	{
		copy_string(mtCb.queue[TT_QUEUE][i].name, 20,"QQ", 2, 2, i);
		copy_string(mtCb.tt[i].name, 20,"TT", 2, -1, i);

		mtCb.tt[i].state.name[0] = 'S';
		mtCb.tt[i].state.name[1] = 'T';
		mtCb.tt[i].state.name[2] = 'T';
		mtCb.tt[i].state.name[3] = '0' + i;
		mtCb.tt[i].state.name[4] = '\0';
		mtCb.tt[i].currentCustId.name[0] = 'C';
		mtCb.tt[i].currentCustId.name[1] = 'C';
		mtCb.tt[i].currentCustId.name[2] = 'U';
		mtCb.tt[i].currentCustId.name[3] = '0' + i;
		mtCb.tt[i].currentCustId.name[4] = 'D';
		mtCb.tt[i].currentCustId.name[5] = '\0';
		mtCb.tt[i].currentCustGrpId.name[0] = 'C';
		mtCb.tt[i].currentCustGrpId.name[1] = 'C';
		mtCb.tt[i].currentCustGrpId.name[2] = 'G';
		mtCb.tt[i].currentCustGrpId.name[3] = '0' + i;
		mtCb.tt[i].currentCustGrpId.name[4] = 'D';
		mtCb.tt[i].currentCustGrpId.name[5] = '\0';
		mtCb.tt[i].msgFromCust.name[0] = 'M';
		mtCb.tt[i].msgFromCust.name[1] = 'C';
		mtCb.tt[i].msgFromCust.name[2] = 'U';
		mtCb.tt[i].msgFromCust.name[3] = '0' + i;
		mtCb.tt[i].msgFromCust.name[4] = 'D';
		mtCb.tt[i].msgFromCust.name[5] = '\0';
		mtCb.tt[i].msgToCust.name[0] = 'M';
		mtCb.tt[i].msgToCust.name[1] = 'T';
		mtCb.tt[i].msgToCust.name[2] = 'C';
		mtCb.tt[i].msgToCust.name[3] = '0' + i;
		mtCb.tt[i].msgToCust.name[4] = 'D';
		mtCb.tt[i].msgToCust.name[5] = '\0';
		createMv(&mtCb.tt[i].state);
		createMv(&mtCb.tt[i].currentCustId);
		createMv(&mtCb.tt[i].currentCustGrpId);
		createMv(&mtCb.tt[i].msgFromCust);
		createMv(&mtCb.tt[i].msgToCust);
	}

	mtCb.man.msgToAll.name[0] = 'M';
	mtCb.man.msgToAll.name[1] = 'M';
	mtCb.man.msgToAll.name[2] = 'T';
	mtCb.man.msgToAll.name[3] = 'A';
	mtCb.man.msgToAll.name[4] = '\0';
	createMv(&mtCb.man.msgToAll);

	mtCb.numOfTicketsTaken.name[0] = 'N';
	mtCb.numOfTicketsTaken.name[1] = 'T';
	mtCb.numOfTicketsTaken.name[2] = 'T';
	mtCb.numOfTicketsTaken.name[3] = '\0';
	createMv(&mtCb.numOfTicketsTaken);

	i = grp_id % 5;
	{
		for(j=0; j < 5; j++)
		{
			if( cust_id == 0 || j == cust_id)
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
	}	
}

/*
 * we use this whenever we need to handle 
 * probablility - basically we generate a 
 * Random number if that number is less than 
 * equal to desired percent reqd 
 * we say yes - done
 * */
int checkChances(int debugId, int percentChances)
{
	int randomNum = Random(); 
	randomNum = randomNum % 100;
	if( randomNum <= percentChances )
	{
		return 1;
	}
	else if( randomNum > percentChances )
	{
		return 0;
	}
}

int askPopcorn(int debugId, Cust *self)
{/*the group head is asking others if they need popcorn*/
        int i=0;
        int numPopcorn=0;
        for(i=0; i < 5; i++)
        {/*there is a 75% probability that popcorn will be needed - this function checkChances will give 1 if that is the case*/
                custGrp.cust[i].takePopcorn = checkChances(debugId, 75);
                if(custGrp.cust[i].takePopcorn)
                {
                        numPopcorn++;/*total number of popcorn for that group enhanced by 1*/
                }
        }
        return numPopcorn;
}

int askSoda(int debugId, Cust *self)
{/*the group head is asking others if they need soda*/
        int i=0;
        int numSoda=0;
        for(i=0; i < 5; i++)
        {
                custGrp.cust[i].takeSoda = checkChances(debugId, 75);/*there is a 75% probability that soda will be needed - this function checkChances will give 1 if that is the case*/
                if(custGrp.cust[i].takeSoda)
                {
                        numSoda++;/*total number of soda for that group enhanced by 1*/
                }
        }
        return numSoda;
}

/*
 * This is to initialize 
 * manager's money
 * */
void initializeMoney()
{
	int i=0, j=0;
	for(i=0; i<2; i++)
	{
		for(j=0; j<5; j++)
		{
			money[i][j]=0;
		}
	}
	mtCb.man.money = 0;
}

/*
 * this is to setup queues for each of ticket taker
 * ,ticket clerk and concession clerk
 * */
int selectAndAddInQueue(int debugId, Cust *self, int queueType)
{
    int queueId=0, numCust=10000;
    int ret=0,num_ele=0;
    int reqdQueueId=-1;
    int tcState, ccState, ttState, state; /*we acquire respective locks depending on the queue type we are trying to manage*/
    if(queueType == TC_QUEUE)
    {
        num_ele=mtCb.numTC;
    }
    else if(queueType == CC_QUEUE)
    {
        num_ele=mtCb.numCC;
    }
    else if(queueType == TT_QUEUE)
    {
        num_ele=mtCb.numTT;
    }

    queueId= (grp_id / 2) ;
    {/*we get state of associated clerk whose queue was passed in as parameter to this function*/
        if(queueType == TC_QUEUE)
        {
            state = getTcState(debugId, queueId);
        }
	else if(queueType == CC_QUEUE)
        {
            state = getCcState(debugId, queueId);
        }
        else if(queueType == TT_QUEUE)
        {
            state = getTtState(debugId, queueId);
        }

        {
            if(getMv(&mtCb.queue[queueType][queueId].state) != NO_CUSTOMER_NO_ADDRESS)
            {
                if(state != ON_BREAK)
                {/*this is to manage if a queue has too many ppl - we get someone off from break*/
                    reqdQueueId = queueId;
                }
            }
        }
    }
    if(reqdQueueId == -1)
    {
	    return NOK;
    }
    if(queueType == TC_QUEUE)
    {
	    tcState = getTcState(debugId, reqdQueueId);
	    if(tcState != ON_BREAK)
	    {
		    ret=queueAddCust(debugId, &mtCb.queue[queueType][reqdQueueId], self);/*attempting to add customer to queue of queueType*/
		    if(ret != NO_ADDRESS )
		    {/*there was a line ot get into*/
			    Print("Customer %d in Group %d is getting in TicketClerk line %d\n", self->selfId, self->grpId, reqdQueueId );

			    self->queue = &mtCb.queue[queueType][reqdQueueId];
			    changeCustState(debugId, self->selfId, self->grpId, WAIT);
			    if(getTcState(debugId, reqdQueueId ) == ON_BREAK)
			    {/*the clerk was on break - so we look for one that wasnt on break*/
				    self->queue = NULL;
				    changeCustState(debugId, self->selfId, self->grpId, LOOKING_FOR_TC_QUEUE);
				    return NOK;
			    }
			    acquireTcLock(debugId, reqdQueueId);/*we get a lock on that ticket clerk*/
		    }
		    else
		    {
			    return NOK;
		    }
	    }
	    else if(tcState == ON_BREAK)
	    {/* we see clerk on lock*/
		    Print("Customer %d in Group %d sees TicketClerk %d is on break\n", self->selfId, self->grpId, reqdQueueId );
	    }           
    }
    else if(queueType == CC_QUEUE)
    {/*now managing concession clerk queue*/
        ccState = getCcState(debugId, reqdQueueId);
        if(ccState != ON_BREAK)
        {
            ret=queueAddCust(debugId, &mtCb.queue[queueType][reqdQueueId], self);
            if(ret != NO_ADDRESS )
            {/*there was space in queue*/
                Print("Customer %d in Group %d is getting in ConcessionClerk line %d\n", self->selfId, self->grpId, reqdQueueId );
                self->queue = &mtCb.queue[queueType][reqdQueueId];
                changeCustState(debugId, self->selfId, self->grpId, WAIT);
                if(getCcState(debugId, reqdQueueId ) == ON_BREAK)
                {	/*cc was on break so we look for another*/
                    self->queue = NULL;
                    changeCustState(debugId, self->selfId, self->grpId, LOOKING_FOR_CC_QUEUE);
                    return NOK;
                }
                acquireCcLock(debugId, reqdQueueId);/*we get lock for that cc before we interact with him*/
            }
            else
            {/*nobody there - we let of of our lock*/
		    return NOK;
            }
        }
        else if (ccState == ON_BREAK)
        {
		Print("Customer %d in Group %d sees ConcessionClerk %d is on break\n", self->selfId, self->grpId, reqdQueueId );
        }
    }
    else if(queueType == TT_QUEUE)
    {/*now working with ticker taker's queue*/
        ttState = getTtState(debugId, reqdQueueId);
        if(ttState != ON_BREAK)
	{
		ret=queueAddCust(debugId, &mtCb.queue[queueType][reqdQueueId], self);
		if(ret != NO_ADDRESS )
		{/*found a place in ticket taker's line*/
			Print("Customer %d in Group %d is getting in TicketTaker line %d\n", self->selfId, self->grpId, reqdQueueId );

			self->queue = &mtCb.queue[queueType][reqdQueueId];
			changeCustState(debugId, self->selfId, self->grpId, WAIT);
			if(getTtState(debugId, reqdQueueId ) == ON_BREAK)
			{
				self->queue = NULL;
				changeCustState(debugId, self->selfId, self->grpId, LOOKING_FOR_TT);
				return NOK;
			}
			acquireTtLock(debugId, reqdQueueId);
		}
		else
		{
			return NOK;
		}
	}
        else if(ttState == ON_BREAK)
	{
		Print("Customer %d in Group %d sees TicketTaker %d is on break\n", self->selfId, self->grpId, reqdQueueId );
	}           
    }

    return ROK;
}


void transferMoneyFromCustToTc(int debugId, Cust *fromCust, int toTcId)
{ /*showing that customer is pay for his buys - to ticket clerk*/
}

/*
 * Customer Main Program
 * */
void custMain()
{
	int selfId=0;
	int popcorns, sodas;
	int debugId=0, i=0, j=0, k=0, seatsTaken, seatTook;
	int ret_val=0;
	Cust *self;
	CustGrp *selfGrp;
	int useDuration;

	/*initializing some values before beginning interactions*/
	seatsTaken = 0;
	seatTook = 0;
	selfId= cust_number;
	self = &custGrp.cust[selfId];
	selfGrp = &custGrp;

	if(self->IAMTicketBuyer)/*if this is the group head*/
	{
		if(grp_id == 1 || grp_id == 3 || grp_id == 5 )
		{
			while(getMv(&second_grp) != SEAT_TAKEN)
			{
				YIELD();
			}
		}
	}
	while(1)
	{
		
		if(getCustState(debugId, selfId, self->grpId) == STARTED_BY_MAIN)/*customers have been just created*/
		{
			if(self->IAMTicketBuyer)/*if this is the group head*/
			{	
				Print("Customer %d in Group %d has entered the movie theater.\n", selfId, self->grpId, -1);
				for(i=1; i< 5; i++)
				{   /*these are other customers in the group - they all wait for him to get back from ticket clerk*/
					while(getCustState(debugId, i, self->grpId) != WAIT_FOR_TICKET_BUYER_FROM_TC)
					{ 
						YIELD(); 
					}
				}
				changeCustState(debugId, self->selfId, self->grpId, LOOKING_FOR_TC_QUEUE);/*it will now look to enter queue*/
				changeCustGrpLocation(debugId, self->grpId, TICKET_CLERK);/*the group is now at ticket clerk*/
			}
			else
			{	/*this aint group head*/
				Print("Customer %d of Group %d is waiting for HeadCustomer\n", self->selfId, self->grpId, -1);
				changeCustState(debugId, self->selfId, self->grpId, WAIT_FOR_TICKET_BUYER_FROM_TC);/*non group heading customers are waiting for him*/
			}
		}
		if(getCustState(debugId, self->selfId, self->grpId) == LOOKING_FOR_TC_QUEUE)
		{ /*if this thread is waiting for a place in a tc queue (will only happen if this grp head)*/
			{
				if(selectAndAddInQueue(debugId, self, TC_QUEUE)==ROK)
				{/*found a queue*/
					if(getCustState(debugId, self->selfId, self->grpId) == SIGNAL)
					{/*the clerk signalled this customer*/
						Print("Customer %d in Group %d in TicketClerk line %d is paying ", self->selfId, self->grpId, self->queue->queueId );
						Print("%d for tickets\n", 5 * PER_TICKET_COST, -1,-1);
						transferMoneyFromCustToTc(debugId, self, self->queue->queueId);	
/*giving money and taking tickets*/
						signalToTcLock(debugId,  self->queue->queueId);
/*get tc off lock on CV by giving him money*/

						waitOnTcLock(debugId,  self->queue->queueId);/*wait for him to give me tickets*/
						Print("Customer %d in Group %d is leaving TicketClerk %d\n", self->selfId, self->grpId, self->queue->queueId );
						signalToTcLock(debugId,  self->queue->queueId);
						releaseTcLock(debugId,  self->queue->queueId);/*done with ticket clerk so dont need to him lock him up anymore - other customers can use him now*/

						changeCustState(debugId, self->selfId, self->grpId, GOT_TICKET);/*have tickets - let's go*/
						Print("HeadCustomer %d of Group %d has told the group to proceed\n", self->selfId, self->grpId,-1);
						changeCustGrpState(debugId, self->grpId, GOT_TICKET);	
					}
				}
			}
		}







		/*now onto concession clerk*/
		if(getCustState(debugId, self->selfId, self->grpId) == GOT_TICKET)
		{


			if(self->IAMTicketBuyer)
			{   /*if this is the group head*/
				for(i=1; i< 5; i++)
				{/*these are other customers in the group - they all wait for him to get back from concession clerk*/
					while(getCustState(debugId, i, self->grpId) != WAIT_FOR_TICKET_BUYER_FROM_CC)
					{ YIELD(); }
				}
				changeCustState(debugId, self->selfId, self->grpId, LOOKING_FOR_CC_QUEUE);/*it will now look to enter queue*/
				changeCustGrpLocation(debugId, self->grpId, CONCESSION_CLERK);/*the group is now at concession clerk*/
			}
			else
			{/*this aint group head*/
				Print("Customer %d in Group %d has entered the movie theater.\n", selfId, self->grpId, -1);
				Print("Customer %d of Group %d is waiting for HeadCustomer\n", self->selfId, self->grpId, -1 );/*non group heading customers are waiting for him*/
				changeCustState(debugId, self->selfId, self->grpId, WAIT_FOR_TICKET_BUYER_FROM_CC);
			}
		}
		
		if(getCustState(debugId, self->selfId, self->grpId) == LOOKING_FOR_CC_QUEUE)
		{/*if this thread is waiting for a place in a cc queue (will only happen if this grp head)*/
			{
				popcorns = askPopcorn(debugId, self);/*asking itself and other grp members how many popcorns the group needs*/
				sodas = askSoda(debugId, self);/*find group's soda requirement*/

				if(selectAndAddInQueue(debugId, self, CC_QUEUE)==ROK)
				{/*found a line*/
					for(i=0; i< 5; i++)
					{
						if(custGrp.cust[i].takePopcorn || custGrp.cust[i].takeSoda)
						{
							AcquireLock(printLock);
							Print("Customer %d in Group %d has %d popcorn and ", self->selfId, self->grpId, custGrp.cust[i].takePopcorn );
							Print("%d soda request from a group member\n", custGrp.cust[i].takeSoda, -1,-1 );
							ReleaseLock(printLock);
						}
					}


					if(getCustState(debugId, self->selfId, self->grpId) == SIGNAL)
					{/*the clerk signalled this customer*/
						setMv(&self->msgBuffer, popcorns);/*noting the number needed*/

						signalToCcLock(debugId,  self->queue->queueId);

						waitOnCcLock(debugId,  self->queue->queueId);/*waiting for opportunity to reach cc*/






						setMv(&self->msgBuffer, sodas);/*noting requirement*/
/*will now go and get it form concession clerk*/
						AcquireLock(printLock);
						Print("Customer %d in Group %d is walking up to ConcessionClerk %d ", self->selfId, self->grpId, self->queue->queueId );
						Print("to buy %d popcorn and %d soda\n", popcorns, sodas, -1 );
						ReleaseLock(printLock);
						signalToCcLock(debugId,  self->queue->queueId);/*wake up cc to get him to give food to me*/

						waitOnCcLock(debugId,  self->queue->queueId);/*wait for him to get back to me with food and then give him money*/





						/*transferMoneyFromCustToCc(debugId, self, self->queue->queueId);   giving money*/
						AcquireLock(printLock);/*PRINT*/
						Print("Customer %d in Group %d in ConcessionClerk line %d ", self->selfId, self->grpId, self->queue->queueId);
						Print("is paying %d for food\n", getMv(&mtCb.cc[self->queue->queueId].msgBuffer),-1,-1 );
						ReleaseLock(printLock);
						Print("Customer %d in Group %d is leaving ConcessionClerk %d\n", self->selfId, self->grpId, self->queue->queueId);

/*everything for food is done - let's go*/

						signalToCcLock(debugId,  self->queue->queueId);
						releaseCcLock(debugId,  self->queue->queueId);



						changeCustState(debugId, self->selfId, self->grpId, GOT_FOOD);
						Print("HeadCustomer %d of Group %d has told the group to proceed\n", self->selfId, self->grpId, -1); /*head customer asks others to get  move on*/

						changeCustGrpState(debugId, self->grpId, GOT_FOOD);   
					}
				}
			}
		}


		if(getCustState(debugId,  self->selfId, self->grpId) == GOT_FOOD)
		{
/*food done - now lets try to get into movie room*/
			if(self->IAMTicketBuyer)
			{/*this is group head*/

				for(i=1; i< 5; i++)
				{/*these are other customers in the group - they all wait for him to get back from ticket taker*/
					while(getCustState(debugId, i, self->grpId) != WAIT_FOR_TICKET_BUYER_FROM_TT)
					{ YIELD(); }
				}
				changeCustGrpLocation(debugId, self->grpId, LOBBY);/*evryone is in lobby*/
				changeCustState(debugId, self->selfId, self->grpId, LOOKING_FOR_TT);/*the group head is looking for ticket taker*/
			}
			else
			{/*others are waiting for grp head to get back ticket taker*/
				Print("Customer %d of Group %d is waiting for HeadCustomer\n", self->selfId, self->grpId, -1 );
				changeCustState(debugId, self->selfId, self->grpId, WAIT_FOR_TICKET_BUYER_FROM_TT);
			}
		}

		if(getCustState(debugId,  self->selfId, self->grpId) == LOOKING_FOR_TT)
		{/*if this thread is waiting for a place in a tt queue (will only happen if this grp head)*/
			{
				if(selectAndAddInQueue(debugId, self, TT_QUEUE)==ROK)
				{/*found a line*/
					if(getCustState(debugId, self->selfId, self->grpId) == SIGNAL)
					{    /*the ticket taker signalled this customer      */

						changeTtState(debugId, self->queue->queueId, BUSY_WITH_CUST); /*the tt is now busy with this customer*/
						mtCb.tt[self->queue->queueId].currentCust = NULL;
						if(getMv(&mtCb.tt[self->queue->queueId].msgToCust) == YES)
						{/*ticket taker says there is place inside for this group*/
							if(increaseNumOfTicketTaken(debugId, 5 ) == ROK)
							{/*add tickets in this group to total tickets with customer*/
								Print("Customer %d in Group %d is walking upto TicketTaker %d to give", self->selfId, self->grpId, self->queue->queueId);
								Print(" %d tickets.\n", 5, -1, -1);
								Print("TicketTaker %d is allowing the group into the theater. The number of tickets taken is %d. \n", self->queue->queueId, 5, -1);
								changeCustGrpLocation(debugId, self->grpId, MOVIEROOM);/*the grp will now move to movie room*/
								signalToTtLock(debugId, self->queue->queueId);
								Print("Customer %d in Group %d is leaving TicketTaker %d\n", self->selfId, self->grpId, self->queue->queueId);/* they have now gone past ticket taker*/
								releaseTtLock(debugId, self->queue->queueId);/*release the ticket taker's lock*/
							}
							else
							{/*the taker is not taking tickets - so the grp head releases its locks and moves back to lobby to his other group mates*/
								signalToTtLock(debugId, self->queue->queueId);
								releaseTtLock(debugId, self->queue->queueId);
								Print("Customer %d in Group %d sees TicketTaker %d is no longer taking tickets. Going to the lobby\n", self->selfId, self->grpId, self->queue->queueId);
								changeCustState(debugId, self->selfId, self->grpId, WAIT_IN_LOBBY_AS_TICKET_NOT_TAKEN_BY_TT);
								changeCustState(debugId, self->selfId, self->grpId, LOOKING_FOR_TT);/*he must now look for a working ticket taker again*/
							}
						}   
						else
						{/*ticket taker says sorry no space inside*/
							signalToTtLock(debugId, self->queue->queueId);
							releaseTtLock(debugId, self->queue->queueId);
							Print("Customer %d in Group %d sees TicketTaker %d is no longer taking tickets. Going to the lobby\n", 
							self->selfId, self->grpId, self->queue->queueId);
							changeCustState(debugId, self->selfId, self->grpId, WAIT_IN_LOBBY_AS_TICKET_NOT_TAKEN_BY_TT);
							changeCustState(debugId, self->selfId, self->grpId, LOOKING_FOR_TT);/*he must now look for a working ticket taker again*/
						}

					}

				}

				/*once we make it to movie room*/

				if(getCustGrpLocation(debugId, self->grpId) == MOVIEROOM)
				{
					changeCustState(debugId, self->selfId, self->grpId, TICKET_TAKEN);/*We have tickets!!*/
					Print("HeadCustomer %d of Group %d has told the group to proceed\n", self->selfId, self->grpId, -1 );
					changeCustGrpState(debugId, self->grpId, TICKET_TAKEN); 
				}
				YIELD();/*now we wait for others for do their work*/
				if(getMv(&mtCb.man.msgToAll) == MOVIE_RUNNING && getCustGrpLocation(debugId, self->grpId) == LOBBY)
				{/*manager has said that movie is running - ie ticket taker is no longer taking tickets.. but grp is in lobby - so it waits till next movie*/
					changeCustState(debugId, self->selfId, self->grpId, WAIT_IN_LOBBY_AS_TICKET_NOT_TAKEN_BY_TT);
					changeCustState(debugId, self->selfId, self->grpId, LOOKING_FOR_TT);

				}
				YIELD();
				if(getCustGrpLocation(debugId, self->grpId) == LOBBY && 5 > (MAX_SEATS - getNumOfTicketTaken(debugId)))
				{/*ticket taker is taking tickets but your grp is too big t fit so you need to wait*/
					changeCustState(debugId, self->selfId, self->grpId, WAIT_IN_LOBBY_AS_TICKET_NOT_TAKEN_BY_TT);
					changeCustState(debugId, self->selfId, self->grpId, LOOKING_FOR_TT);
				}
			}
		}





		/*now lets get seated in movie room*/
		if(getCustState(debugId,  self->selfId, self->grpId) == TICKET_TAKEN)
		{/*we have tickets*/
			if(self->IAMTicketBuyer)
			{/*this is group head*/
				for(i=1; i< 5; i++)
				{/*these are other customers in the group - they all wait for him to tell them everyone's all seated and set*/
					while(getCustState(debugId, i, self->grpId) != WAIT_FOR_TICKET_BUYER_TILL_SEATS_TAKEN)
					{ YIELD(); }
				}
				changeCustState(debugId, self->selfId, self->grpId, LOOKING_FOR_SEATS);/*i am trying to seat my group*/
			}
			else
			{/* i am not grp head - waiting for him to tell us that seats taken*/
				changeCustState(debugId, self->selfId, self->grpId, WAIT_FOR_TICKET_BUYER_TILL_SEATS_TAKEN);
			}
		}

		if(getCustState(debugId,  self->selfId, self->grpId) == LOOKING_FOR_SEATS)
		{
			AcquireLock(seats);/*get the lock to look for seats*/
			/*looking for seats in same row*/
			i = grp_id % 5;
			{
				for(j=0; j<MAX_COLS; j++)
				{
					{/*allocating seats*/
						mtCb.mvRoom.seat[i][j].cust = self;
						self->seat = &mtCb.mvRoom.seat[i][j];
						setMv(&mtCb.mvRoom.seat[i][j].custId,  j); 
						setMv(&mtCb.mvRoom.seat[i][j].custGrpId, self->grpId); 
						setMv(&custGrp.cust[j].seatI, i);
						setMv(&custGrp.cust[j].seatJ, j);
						Print("Customer %d in Group %d has found the following seat: row %d and ", j, grp_id, i);
						Print("seat %d\n", j, -1,-1);

					}

				}
				seatsTaken = 1;
			}
			/*looking for seats in consecutive rows*/
			ReleaseLock(seats);
		}

		if(seatsTaken)
		{/*people have selected seats*/

			changeCustState(debugId, self->selfId, self->grpId, SEATS_READY);
			changeCustGrpState(debugId, self->grpId, SEATS_READY);
		}
		if(getCustState(debugId,  self->selfId, self->grpId) == SEATS_READY)
		{
			{
				self->seat = &mtCb.mvRoom.seat[getMv(&self->seatI)][getMv(&self->seatJ)];
				setMv(&self->seat->isTaken, 1);
				setMv(&self->seatTaken, 1);
				if(self->IAMTicketBuyer)
				{
					for(i=1; i< 5; i++)
					{/*wait for evryone in group to sit down*/
						while( getCustState(debugId, i, self->grpId) != SEAT_TAKEN )
						{ YIELD(); }
					}
				}
				Print("Customer %d in Group %d is sitting in a theater room seat\n", self->selfId, self->grpId,-1);
				changeCustState(debugId, self->selfId, self->grpId, SEAT_TAKEN);
			}
		}





		/*time to go after movie*/
		if((getCustState(debugId,  self->selfId, self->grpId) == READY_TO_LEAVE_MOVIE_ROOM) )
		{
			{
				setMv(&self->seat->isTaken, 0);
				setMv(&self->seatTaken, 0);
				setMv(&self->seat->custId, -1); 
				setMv(&self->seat->custGrpId, -1); 
				setMv(&self->seatI, -1);
				setMv(&self->seatJ, -1);
				self->seat->cust = NULL;
				self->seat = NULL;
			}
			{
				if(!self->IAMTicketBuyer)
				{	
					while(getCustState(debugId, 0, self->grpId) != LEFT_MOVIE_ROOM_AFTER_MOVIE)
					{ YIELD(); }
				}
			}

			if(self->IAMTicketBuyer)/* allowing the group to proceed*/
			{
				Print("HeadCustomer %d of Group %d has told the group to proceed\n", self->selfId, self->grpId, -1 );
			}
			Print("Customer %d in Group %d is getting out of a theater room seat\n", self->selfId, self->grpId, -1);
			changeCustState(debugId, self->selfId, self->grpId, LEFT_MOVIE_ROOM_AFTER_MOVIE);
		}

		if(getCustState(debugId,  self->selfId, self->grpId) == LEFT_MOVIE_ROOM_AFTER_MOVIE )
		{
			if(self->IAMTicketBuyer)
			{   /*I am grp head - getting evryone out of movie room*/
				changeCustGrpLocation(debugId, self->grpId, OUT_OF_MOVIEROOM);
				for(i=1; i< 5; i++)
				{  /*checking for others*/
					while(
						getCustState(debugId, i, self->grpId) != HEADING_FOR_BATHROOM &&
						getCustState(debugId, i, self->grpId) != USING_BATHROOM &&
						getCustState(debugId, i, self->grpId) != READY_TO_GO_OUT_OF_MT
					     )
					{ YIELD(); }/*while everyone is ready to leave theater, we wait*/
				}
			}
			changeCustState(debugId, self->selfId, self->grpId, HEADING_FOR_BATHROOM); /*movie over, heading to bathroom*/

		}
		if(getCustState(debugId,  self->selfId, self->grpId) == HEADING_FOR_BATHROOM)
		{/*we'll see here if I do get to go*/
			if(checkChances(debugId, 25))
			{/*25% probability that customer will go*/
				Print("Customer %d in Group %d is going to the bathroom.\n", self->selfId, self->grpId,-1);
				changeCustState(debugId, self->selfId, self->grpId, USING_BATHROOM);

				useDuration = Random();
				useDuration %= 10;
				useDuration += 5;
				/*we assume once a peson goes to bathroom, he returns after doing his business in some Random time between 5 and 15 thread yields*/
				for(i=0; i<useDuration; i++)
				{				
					YIELD();
				}
				Print("Customer %d in Group %d is leaving the bathroom.\n", self->selfId, self->grpId, -1);
			}
			changeCustState(debugId, self->selfId, self->grpId, READY_TO_GO_OUT_OF_MT);
		}
		if(getCustState(debugId,  self->selfId, self->grpId) == READY_TO_GO_OUT_OF_MT)
		{ /*ready to go out of theater*/
			Print("Customer %d in Group %d is in the lobby\n", self->selfId, self->grpId,-1);
			for(i=0; i< 5; i++)
			{  /*checking if people in my group needed tto use bathroom or not*/
				while(
						getCustState(debugId, i, self->grpId) != CUSTOMER_SIMULATION_COMPLETED &&
						getCustState(debugId, i, self->grpId) != READY_TO_GO_OUT_OF_MT
				     )
				{ YIELD(); }/*we wait for everyone to leave theater and simulation completes*/
			}
			Print("Customer %d in Group %d is leaving the lobby\n", self->selfId, self->grpId, -1);

			Print("Customer %d in Group %d has left the movie theater.\n", self->selfId, self->grpId,-1);
			
			changeCustState(debugId, self->selfId, self->grpId, CUSTOMER_SIMULATION_COMPLETED);/*once everyone has left theater - the simulation is over*/
			if(self->IAMTicketBuyer)
			{ /*the grp head waits for everyone to leave theater and simulation ending*/
				for(i=1; i< 5; i++)
				{
					while(
							getCustState(debugId, i, self->grpId) != CUSTOMER_SIMULATION_COMPLETED
					     )
					{ YIELD(); }
				}
				changeCustGrpLocation(debugId, self->grpId, OUT_OF_MOVIETHEATURE);
			}
			return;
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
	char dbg[10], logFileName[100];
	int i=0,ret_val=0;
	int debugId=1;

	initialize();
	initializeAllValues(debugId); /*intializing entities that shall be part of simulation*/
	if(cust_id == 0)
	{
		initializeAllQueues(debugId);
        }/*initializing all different queues that are needed*/
	initializeAllLocks(debugId);/*set up required locks*/
	changeCustState(debugId, cust_number, custGrp.grpId, STARTED_BY_MAIN);
	changeCustLocation(debugId, cust_number, custGrp.grpId, START);
	custMain();
	Exit(0);
}
