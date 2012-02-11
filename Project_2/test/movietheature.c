#include "syscall.h"
#include "movietheature_main.h"

/*extern void ThreadTest_ForPart1();*/
/*extern void SelfTest1_ForPart1();*/

/*struct DebugControlBlock debugCb;*/

int gDebugFileNo=1;
int printLock;
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
int manLock;
int ticketTaking;
int tcLock[MAX_TC];
int ttLock[MAX_TT];
int ccLock[MAX_CC];
int seats;
int mtLock;
int custLock[MAX_CUST_GRP][MAX_CUST];
int queueLock[MAX_QUEUE_TYPE][MAX_NUM_QUEUE];
int queueTcLock;
int queueCcLock;
int queueTtLock;
int exitLock;
int exitCV;

int manCondVar;
int mtCondVar;
int tcCondVar[MAX_TC];
int ttCondVar[MAX_TT];
int ccCondVar[MAX_CC];
int custCondVar[MAX_CUST_GRP][MAX_CUST];
int queueCondVar[MAX_QUEUE_TYPE][MAX_NUM_QUEUE];
int sleepThreads=0;
int forkThreads=0;
/*
 * This are certain variables
 * required for specific test 
 * cases
 * */
int IsSeatsOccupied;
int money[2][5];
int mtTest4=0;
int mtTest3=0;
int mtTest3Called=0;
int mtTest2=0;
int mtTest1=0;
int mtTest2Called=0;
int mtTest6=0;
int mtTest6Called=0;
int mtTest4Called=0;

int arg_lock;
int arg_in_use = 0;
char arg_lockName[25];
int thread_arg;
/* Movie Theature control 
 * block
 * */
MtCb mtCb;
typedef void (*ThreadFuncPtr)();

void YIELD()
{
	Yield();
}

void initializeForkMec()
{
	arg_lockName[0]='A';
	arg_lockName[1]='L';
	arg_lockName[2]='\0';
	arg_lockName[3]='\0';
	arg_in_use = 0;
	arg_lock = CreateLock(arg_lockName);
	printLock = CreateLock(arg_lockName);
	exitLock = CreateLock(arg_lockName);
	exitCV = CreateCV(arg_lockName);
}

void thread_sleep()
{
	sleepThreads++;
}

void start_fork_thread(char *threadName, ThreadFuncPtr threadFunc, int arg)
{
	while( 1 == 1 )
	{
		AcquireLock(arg_lock);
		if( arg_in_use == 1 )
		{
			ReleaseLock(arg_lock);
			YIELD();
		}
		else 
		{
			break;
		}	
		/*ReleaseLock(arg_lock);*/
	}
	forkThreads++;
	arg_in_use = 1;
	ReleaseLock(arg_lock);
	thread_arg = arg;
	Fork(threadFunc, threadName);
}

void end_fork_thread(int *arg)
{
	*arg = thread_arg;
	AcquireLock(arg_lock);
	arg_in_use = 0;
	ReleaseLock(arg_lock);
}

void end_check(int debugId)
{
	int i=0;
	
	for (i=0; i<mtCb.numCustGrp; i++)
	{
		if(getCustGrpLocation(debugId, i) == OUT_OF_MOVIETHEATURE);
		/*if(getCustGrpLocation(debugId, i) == LOBBY);*/
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
 * Function is to obtain current state of movie technician
 * */
int getMtState(int debugId)
{
	int state;
	AcquireLock(mtLock);   /*get lock before we get it*/
	state = mtCb.mt.state;
	ReleaseLock(mtLock); /*release the lock */
	return state;
}

/*
 * Function is to change state of movie technician
 * */
void changeMtState(int debugId, int state)
{
	AcquireLock(mtLock); /*acquire a lock before we do it*/
	mtCb.mt.state = state;
	
	if (state == STARTED_BY_MAN) /*Mtechnician has been asked to start working */
	{
	}
	else if (state == MOVIE_IS_NOT_PLAYING) /*Mtechnician is currently working*/
	{
		mtCb.mt.msgToMan = MOVIE_OVER;
		WaitCV(mtLock,mtCondVar);
	}
	else if ( state == MOVIE_IS_PLAYING || state == NO_MOVIE_REQD )
	{
		SignalCV(mtLock,mtCondVar);		
		mtCb.mt.msgToMan = STARTING_MOVIE;
	}
	
	ReleaseLock(mtLock); /*release the lock */
}

/*
 * Function is to obtain current concession clerk state
 * */
int getCcState(int debugId, int ccId)
{
	int state;
	state = mtCb.cc[ccId].state;

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
void signalToCcLock(debugId, ccId1)
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
void changeCcState(debugId, ccId1, state1)
{
	mtCb.cc[ccId1].state = state1;
}


/*
 * Function is to get ticket clerk state.
 * */
int getTcState(int debugId, int tcId)
{
	int state;
	state = mtCb.tc[tcId].state;

	return state;
}

/*
 * broadcasting a CV for a ticket clerk on lock 
 * */
void broadcastAllTcLock( debugId, tcId1)
{
	BroadcastCV(tcLock[tcId1], tcCondVar[tcId1]); 
}

/*
 * this is enclosed Wait function on CV for a ticket clerk on lock
 * */
void waitOnTcLock( debugId, tcId1)
{
	WaitCV(tcLock[tcId1],tcCondVar[tcId1]); 
}

/*
 * enclosing call to Signal function of CV of ticket clerk on lock
 * */
void signalToTcLock(debugId, tcId1)
{
	SignalCV(tcLock[tcId1],tcCondVar[tcId1]); 
}

/*
 * enclosed call to acquire ticket clerks's lock
 * */
void acquireTcLock(debugId, tcId1)
{
	AcquireLock(tcLock[tcId1]); 
}

/*
 * enclosing a call to release lock of ticket clerk 
 * */
void releaseTcLock(debugId, tcId1)
{
	ReleaseLock(tcLock[tcId1]); 
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
void broadcastAllTtLock( debugId, ttId1)
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
	state = mtCb.tt[ttId].state;
	return state;
}

/*
 * setting new value for state of ticket taker  
 * */
void changeTtState(int debugId, int ttId, int state)
{
	mtCb.tt[ttId].state = state;
}

/*
 * setting new state for ticket clerk
 * */
void changeTcState(debugId, tcId1, state1)
{
	mtCb.tc[tcId1].state = state1;
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
	state = mtCb.custGrp[grpId].cust[custId].state;
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
	currentState = mtCb.custGrp[grpId].cust[custId].state;
	mtCb.custGrp[grpId].cust[custId].state = state;

	

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
		WaitCV(custLock[grpId][custId],custCondVar[grpId][custId]);/*if the state is set to be some sort of WAIT we call wait on CV*/
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
		SignalCV(custLock[grpId][custId],custCondVar[grpId][custId]);/*we stop waiting and signal customer to wake up*/
	}
	ReleaseLock(custLock[grpId][custId]); 

}

/*
 * obtaining state of entire custmer group
 * */
int getCustGrpState(int debugId, int grpId)
{
	int state;
	state = mtCb.custGrp[grpId].state;
	return state;
}

/*
 * setting location of particular customer
 * */
void changeCustLocation(int debugId, int custId, int grpId, int location)
{
	mtCb.custGrp[grpId].cust[custId].location = location;
}

/*
 * setting a new location for the customer group
 * */
void changeCustGrpLocation(int debugId, int grpId, int location)
{
	int i=0;
	mtCb.custGrp[grpId].location = location;
	for(i=0; i < mtCb.custGrp[grpId].numCust; i++)
	{
		if(location == LOBBY)
		{ /*PRINT*/
			AcquireLock(printLock);
			Print("Customer %d in Group %d is in the lobby\n", i, grpId,-1);
			ReleaseLock(printLock);
		}
		else if(location == MOVIEROOM)
		{/*PRINT*/
			AcquireLock(printLock);
			Print("Customer %d in Group %d is leaving the lobby\n", i, grpId,-1);
			ReleaseLock(printLock);			
		}
		changeCustLocation(debugId, i, grpId, location);/*we set location of all individual customers as that of the group*/
	}
}

/*
 * finding value of location for a customer group
 * */
int getCustGrpLocation(int debugId, int grpId)
{
	return mtCb.custGrp[grpId].location;
}

/*
 * finding vaalue of location for a particular customer
 * */
int getCustLocation(int debugId, int custId, int grpId)
{
	return mtCb.custGrp[grpId].cust[custId].location;
}

/*
 * we set the state of whole group together here
 * */
void changeCustGrpState(int debugId, int grpId, int state)
{
	int i=0;
	mtCb.custGrp[grpId].state = state;
	if(state == WAIT)
	{
	}
	else if(state == SIGNAL)
	{
	}
	else if( state == GOT_TICKET || state == GOT_FOOD || state == TICKET_TAKEN || SEATS_READY )
	{  /*once group head gets tickets or food or has ticket taken by ticket taker or find seats, he lets other know - using a signal*/
		for(i=0; i < mtCb.custGrp[grpId].numCust; i++)
		{
			if(!mtCb.custGrp[grpId].cust[i].IAMTicketBuyer)
			{
				changeCustState(debugId, i, grpId, state);
				AcquireLock(custLock[grpId][i]);
				SignalCV(custLock[grpId][i],custCondVar[grpId][i]);
				ReleaseLock(custLock[grpId][i]);
			}
		}
	}
}


/*
 * transferring ticket to customer here and add details to customer
 * */
void transferTicketFromTo(int debugId, TC *fromTc, Cust *toCust)
{
	int i=0;

	for(i=0; i<mtCb.custGrp[toCust->grpId].numCust; i++)
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
	queue->numCust = 0;
	queue->state = NO_CUSTOMER_NO_ADDRESS;
	queue->queueAddress=NULL;
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
		AcquireLock(printLock);
		Print("Customer %d in Group %d sees TicketClerk %d is on break", cust->selfId, cust->grpId, queue->queueId );
		ReleaseLock(printLock);
		ReleaseLock(queueLock[queue->queueType][queue->queueId]);
		return NO_ADDRESS;
	}
	else if( queue->queueType == CC_QUEUE && getCcState(debugId, queue->queueId) == ON_BREAK )
	{/*PRINT*/
		AcquireLock(printLock);	
		Print("Customer %d in Group %d sees ConcessionClerk %d is on break", cust->selfId, cust->grpId, queue->queueId );
		ReleaseLock(printLock);
		ReleaseLock(queueLock[queue->queueType][queue->queueId]);
		return NO_ADDRESS;
	}
	else if( queue->queueType == TT_QUEUE && getTtState(debugId, queue->queueId) == ON_BREAK )
	{
		AcquireLock(printLock);	
		Print("Customer %d in Group %d sees TicketTaker %d is on break", cust->selfId, cust->grpId, queue->queueId );
		ReleaseLock(printLock);
		ReleaseLock(queueLock[queue->queueType][queue->queueId]);
		return NO_ADDRESS;
	}
	queue->numCust++;
	WaitCV(queueLock[queue->queueType][queue->queueId],queueCondVar[queue->queueType][queue->queueId]);
	if( queue->queueType == TC_QUEUE )
	{
		mtCb.tc[queue->queueId].currentCust = cust;
		mtCb.tc[queue->queueId].msgFromCust = CUST_REMOVED;
	}
	else if( queue->queueType == CC_QUEUE )
	{
		mtCb.cc[queue->queueId].currentCust = cust;
		mtCb.cc[queue->queueId].msgFromCust = CUST_REMOVED;
	}
	else if( queue->queueType == TT_QUEUE )
	{
		mtCb.tt[queue->queueId].currentCust = cust;
		mtCb.tt[queue->queueId].msgFromCust = CUST_REMOVED;
	}
	else
	{
		ReleaseLock(queueLock[queue->queueType][queue->queueId]);
		return NO_ADDRESS;
	}
	ReleaseLock(queueLock[queue->queueType][queue->queueId]);
	return ROK;
}

/*
 * removing a customer from a queue
 * */
Cust* queueRemoveCust(int debugId, mtQueue *queue)
{
	/*We acquire a lock on the queue and then modify it. A customer is removed and is ready to be served.*/
	Cust *cust;


	
	SignalCV(queueLock[queue->queueType][queue->queueId],queueCondVar[queue->queueType][queue->queueId]);
	if( queue->queueType == TC_QUEUE )
	{
		while(mtCb.tc[queue->queueId].msgFromCust != CUST_REMOVED)
		{ 
			YIELD();
		}
		mtCb.tc[queue->queueId].msgFromCust = INVALID_MSG;
		cust = mtCb.tc[queue->queueId].currentCust;
	}
	else if( queue->queueType == CC_QUEUE )
	{
		while(mtCb.cc[queue->queueId].msgFromCust != CUST_REMOVED)
		{ 
			YIELD();
		}
		mtCb.cc[queue->queueId].msgFromCust = INVALID_MSG;
		cust = mtCb.cc[queue->queueId].currentCust;
	}
	else if( queue->queueType == TT_QUEUE )
	{
		while(mtCb.tt[queue->queueId].msgFromCust != CUST_REMOVED)
		{ 
			YIELD();
		}
		mtCb.tt[queue->queueId].msgFromCust = INVALID_MSG;
		cust = mtCb.tt[queue->queueId].currentCust;
	}


	AcquireLock(queueLock[queue->queueType][queue->queueId]);

	if(cust!=NULL)
	{/*if there were customers in line, we take them out and reduce count of number of customers in queue. After that, we release lock and return pointer to customer*/
		queue->numCust--;
		ReleaseLock(queueLock[queue->queueType][queue->queueId]);
		return cust;
	}
	else 
	{/*if there were no customers in queue we simply release lock and return a Null signifying no customers*/

		ReleaseLock(queueLock[queue->queueType][queue->queueId]);
		return NULL;
	}
}

/*
 * this function is for linking the workers - tc,cc, and tt- to their queues 
 * */
void addAddressToQueue(int debugId, int id, int queueType, int queueId)
{
	AcquireLock(queueLock[queueType][queueId]); 
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
	ReleaseLock(queueLock[queueType][queueId]); 
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
	count=queue->numCust;
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
	/*char *queueName;*/
	for(i=0; i<mtCb.numTC; i++) /*setting up requisite number of ticket clerk queues*/
	{
		initializeQueue(debugId, &mtCb.queue[TC_QUEUE][i]);
		mtCb.queue[TC_QUEUE][i].queueId = i;
		mtCb.queue[TC_QUEUE][i].queueType = TC_QUEUE;

		queueLock[TC_QUEUE][i] = CreateLock(mtCb.queue[TC_QUEUE][i].name);
		queueCondVar[TC_QUEUE][i] = CreateCV(mtCb.queue[TC_QUEUE][i].name);
	}

	for(i=0; i<mtCb.numCC; i++) /*setting up requisite number of concession clerk queues*/
	{
		initializeQueue(debugId, &mtCb.queue[CC_QUEUE][i]);
		mtCb.queue[CC_QUEUE][i].queueId = i;
		mtCb.queue[CC_QUEUE][i].queueType = CC_QUEUE;
		queueLock[CC_QUEUE][i] = CreateLock(mtCb.queue[CC_QUEUE][i].name);
		queueCondVar[CC_QUEUE][i] = CreateCV(mtCb.queue[CC_QUEUE][i].name);
	}

	for(i=0; i<mtCb.numTT; i++) /*setting up requisite number of ticket taker queues*/
	{
		initializeQueue(debugId, &mtCb.queue[TT_QUEUE][i]);
		mtCb.queue[TT_QUEUE][i].queueId = i;
		mtCb.queue[TT_QUEUE][i].queueType = TT_QUEUE;
		queueLock[TT_QUEUE][i] = CreateLock(mtCb.queue[TT_QUEUE][i].name);
		queueCondVar[TT_QUEUE][i] = CreateCV(mtCb.queue[TT_QUEUE][i].name);
	}

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
			custLock[i][j]=CreateLock(mtCb.custGrp[i].cust[j].name);
			custCondVar[i][j]=CreateCV(mtCb.custGrp[i].cust[j].name);
		}
	}

	for(i=0; i<mtCb.numTC; i++)
	{ /*locks and CVs for ticket clerks*/
		tcLock[i]=CreateLock(mtCb.tc[i].name);
		tcCondVar[i]=CreateCV(mtCb.tc[i].name);
	}
	for(i=0; i<mtCb.numCC; i++)
	{/* locks and CVs for concession clerks */

		ccLock[i]=CreateLock(mtCb.cc[i].name);
		ccCondVar[i]=CreateCV(mtCb.cc[i].name);
	}
	for(i=0; i<mtCb.numTT; i++)
	{	/* locks and CVs for ticket takers*/

		ttLock[i]=CreateLock(mtCb.tt[i].name);
		ttCondVar[i]=CreateCV(mtCb.tt[i].name);
	}

/*Now initialising individual locks and CVs for:*/
	/*---- manager*/
	manLock=CreateLock(mtCb.man.name);
	manCondVar=CreateCV(mtCb.man.name);

	/*--- seats*/
	mtCb.ticketTaking_name[0] = 'T';
	mtCb.ticketTaking_name[1] = 't';
	mtCb.ticketTaking_name[2] = '\0';
	mtCb.ticketTaking_name[3] = '\0';
	mtCb.seats_name[0] = 'S';
	mtCb.seats_name[1] = 't';
	mtCb.seats_name[2] = '\0';
	mtCb.seats_name[3] = '\0';
	ticketTaking=CreateLock(mtCb.ticketTaking_name);
	/*---ticket takers*/
	seats=CreateLock(mtCb.seats_name);
	/*---movie technician*/
	mtLock=CreateLock(mtCb.mt.name);
	mtCondVar=CreateCV(mtCb.mt.name);
}

/*
 * reset the number of tickets received by all ticket takers
 * */
void resetNumOfTicketTaken(int debugId)
{
	AcquireLock(ticketTaking);
	mtCb.numOfTicketsTaken  = 0;
	ReleaseLock(ticketTaking);
}

/*
 * tickets received by tt in current cycle
 * */
int getNumOfTicketTaken(int debugId)
{
	int value;
	AcquireLock(ticketTaking);
	value = mtCb.numOfTicketsTaken;
	ReleaseLock(ticketTaking);
	return value;
}

/*
 * ticket taker has collected this many tickets
 * */
int increaseNumOfTicketTaken(int debugId, int value)
{
	AcquireLock(ticketTaking);

	if((mtCb.numOfTicketsTaken + value) <= MAX_SEATS)
	{
		mtCb.numOfTicketsTaken = mtCb.numOfTicketsTaken + value;
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
 * This function sets first customer of group (ie, one with cust index 0) as Ticket Buyer - the Head customer
 * */
int isCustTicketBuyer(int debugId, int grpId, int custId)
{
	if(custId == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
 * we'll be taking inputs here to see how many of what entities is to be created
 * and allocating memory to these.
 * */
void initializeAllValues(int debugId)
{
	int i=0, j=0;
	AcquireLock(printLock);	/*PRINT*/
	Print("Write Number Of Ticket Clerks\n",-1,-1,-1);
	ReleaseLock(printLock);	
	/*mtCb.numTC=Scan(&mtCb.numTC);*/
	/*scanf("%d", &mtCb.numTC);*/
	for(i=0; i<mtCb.numTC; i++)
	{
		copy_string(mtCb.queue[TC_QUEUE][i].name, 20,"QQ", 2, 0, i);
		copy_string(mtCb.tc[i].name, 20,"TC", 2, -1, i);
	}

	AcquireLock(printLock);	/*PRINT*/
	Print("Write Number Of Concession Clerks\n",-1,-1,-1);
	ReleaseLock(printLock);
	/*mtCb.numCC=Scan(&mtCb.numCC);*/
	/*scanf("%d", &mtCb.numCC);*/
	for(i=0; i<mtCb.numCC; i++)
	{
		copy_string(mtCb.queue[CC_QUEUE][i].name, 20,"QQ", 2, 1, i);
		copy_string(mtCb.cc[i].name, 20,"CC", 2, -1, i);
	}

	AcquireLock(printLock);	/*PRINT*/
	Print("Write Number Of Ticket Takers\n",-1,-1,-1);
	ReleaseLock(printLock);
	/*mtCb.numTT=Scan(&mtCb.numTT);*/
	/*scanf("%d", &mtCb.numTT);*/
	for(i=0; i<mtCb.numTT; i++)
	{
		copy_string(mtCb.queue[TT_QUEUE][i].name, 20,"QQ", 2, 2, i);
		copy_string(mtCb.tt[i].name, 20,"TT", 2, -1, i);
	}

	copy_string(mtCb.man.name, 20,"MA", 2, -1, 1);
	copy_string(mtCb.mt.name, 20,"MT", 2, -1, 1);
	AcquireLock(printLock);/*PRINT*/
	Print("Write Total Number Of Customers\n",-1,-1,-1);
	ReleaseLock(printLock);
	/*scanf("%d", &mtCb.numCust);*/
	/*mtCb.numCust=Scan(&mtCb.numCust);*/

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

		int randomNum = Random(); 
		randomNum = (randomNum % 5) + 1;
		if(i <= 5)
		{
			randomNum = i;
		}
		i = i- randomNum;
				
		mtCb.custGrp[mtCb.numCustGrp].grpId = mtCb.numCustGrp;
		mtCb.custGrp[mtCb.numCustGrp].location = START;
		mtCb.custGrp[mtCb.numCustGrp].numCust = randomNum; 
		copy_string(mtCb.custGrp[mtCb.numCustGrp].name, 20,"CG", 2, -1, mtCb.numCustGrp);
		for(j=0; j<mtCb.custGrp[mtCb.numCustGrp].numCust; j++)
		{
			mtCb.custGrp[mtCb.numCustGrp].cust[j].selfId = j;
			mtCb.custGrp[mtCb.numCustGrp].cust[j].grpId = mtCb.numCustGrp;
			mtCb.custGrp[mtCb.numCustGrp].cust[j].money = CUST_MONEY;
			mtCb.custGrp[mtCb.numCustGrp].cust[j].IAMTicketBuyer = isCustTicketBuyer(debugId, mtCb.numCustGrp,j);
			copy_string(mtCb.custGrp[mtCb.numCustGrp].cust[j].name, 20,"CU", 2, mtCb.numCustGrp, j);
		}
		mtCb.numCustGrp++;

	}


/*Entities given are being checked here.*/
	if( 
			mtCb.numTC > 5 ||  mtCb.numTC <1 ||
			mtCb.numCC > 5 ||  mtCb.numCC <1 ||
			mtCb.numTT > 3 ||  mtCb.numTT <1 
	  )
	{
		AcquireLock(printLock);	/*PRINT*/
		Print("Number of employees, you entered are higher than the allowable limit. So, the program will now Exit.\n", mtCb.numCust,-1,-1);
		ReleaseLock(printLock);
		Exit(0);
	}
	if( 
			mtCb.numCust > 40
	  )
	{
		AcquireLock(printLock);	/*PRINT*/		
		Print("WARNING: Number of customers are greater than 50. \n\n", mtCb.numCust,-1,-1);
		ReleaseLock(printLock);	
	}


	/*Printing entities*/
	AcquireLock(printLock);		/*PRINT*/
	Print("Number Of Customers = %d \n", mtCb.numCust,-1,-1);
	Print("Number Of Groups = %d \n", mtCb.numCustGrp,-1,-1);
	Print("Number Of TicketClerks = %d \n", mtCb.numTC,-1,-1);
	Print("Number Of ConcessionClerks = %d \n", mtCb.numCC,-1,-1);
	Print("Number Of TicketTakers = %d \n", mtCb.numTT,-1,-1);
	ReleaseLock(printLock);
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
 * Manager managing accounts with Ticket clerk
 * */
void moneyTcManage(int debugId)
{		
	int i=0;
	int currentMoney=0;	
	for( i=0; i<mtCb.numTC; i++)
	{	
		currentMoney =  mtCb.tc[i].money;
		
		money[0][i] = currentMoney - money[0][i];
		mtCb.man.money = money[0][i] + mtCb.man.money;
		AcquireLock(printLock);/*PRINT*/
		Print("Manager collected %d from TicketClerk %d\n", money[0][i], i, -1);
		ReleaseLock(printLock);
		money[0][i] = currentMoney;
	}
}

/*
 * Manager managing accounts with Concession clerk
 * */
void moneyCcManage(int debugId)
{		
	int i=0;	
	int currentMoney=0;	
	for( i=0; i<mtCb.numCC; i++)
	{
		currentMoney =  mtCb.cc[i].money;
		
		money[1][i] = currentMoney - money[1][i];
		mtCb.man.money = money[1][i] + mtCb.man.money;
		AcquireLock(printLock);/*PRINT*/
		Print("Manager collected %d from ConcessionClerk %d\n", money[1][i], i,-1);
		ReleaseLock(printLock);
		money[1][i] = currentMoney;
	}
}

/*
 * function is for management of the TT queue by the Manager
 * */
void queueTtManage(int debugId)
{      
    int i=0, j=0, k=0;
    for( i=0; i<mtCb.numTT; i++)
    {
        int currentCustCount = getTotalCustCount(debugId, &mtCb.queue[TT_QUEUE][i]);
        if( currentCustCount < 3 && currentCustCount !=0 && getTtState(debugId, i) != ON_BREAK )
        { /*we have fewer than 3 people in line and ticket taker is not on break*/
            k=0;
            for( j=0; j<mtCb.numTT; j++)
            {
                if(j != i && ( getTtState(debugId, j) == ON_BREAK || mtCb.tt[j].msgByMan==GO_ON_BREAK))
                    k++;/*number of other TTs on break or who've have been asked by manager to go on break*/
            }
            if(k != (mtCb.numTT - 1) && checkChances(debugId, 20))
            {/*if not everyone else is on break, we check chances if this taker can go on break - and, if he can then we...*/
                mtCb.tt[i].msgByMan=GO_ON_BREAK;
			AcquireLock(printLock);/*PRINT*/
                Print("Manager has told TicketTaker %d to go on break.\n", i, -1, -1);
			ReleaseLock(printLock);
            }
        }
        if( currentCustCount > 5 )
        {/*if this employee has more than 5 customers to handlle, we need to get someone off from break*/
            k=0;
            for( j=0; j<mtCb.numTT; j++)
            {/*we are looking for other TTs*/
                if(j != i && getTtState(debugId, j) == ON_BREAK)
                {/*we found one on a break - so we get him off break and wake him up*/
                    signalToTtLock(debugId, j);
                    break;
                }
            }
        }
    }
}

/*
 * this is for manager to manage ticket clerk queue
 * */
void queueTcManage(int debugId)
{       
	int i=0, j=0, k=0;
	for( i=0; i<mtCb.numTC; i++)
	{
		int currentCustCount = getTotalCustCount(debugId, &mtCb.queue[TC_QUEUE][i]);
		if( currentCustCount < 3 && currentCustCount !=0 && getTcState(debugId, i) != ON_BREAK )
		{ /*we have fewer than 3 people in line and ticket clerk is not on break*/
			k=0;
			for( j=0; j<mtCb.numTC; j++)
			{
				if(j != i && ( getTcState(debugId, j) == ON_BREAK || mtCb.tc[j].msgByMan==GO_ON_BREAK))
					k++; /*number of other ticket clerks on break or who've have been asked by manager to go on break*/

			}
			if(k != (mtCb.numTC - 1) && checkChances(debugId, 20))
			{/*if not everyone else is on break, we check chances if this clerk can go on break - and, if he can then we...*/
				mtCb.tc[i].msgByMan=GO_ON_BREAK;
				AcquireLock(printLock);/*PRINT*/
				Print("Manager has told TicketClerk %d to go on break.\n", i, -1, -1);
				ReleaseLock(printLock);
			}
		}
		if( currentCustCount > 5 )
		{/*if this employee has more than 5 customers to handlle, we need to get someone off from break*/
			k=0;
			for( j=0; j<mtCb.numTC; j++)
			{/*we are looking for other TCs*/
				if(j != i && getTcState(debugId, j) == ON_BREAK)
				{ /*we found one on a break - so we get him off break and wake him up*/
					signalToTcLock(debugId, j);
					break;
				} 
			}
		}
	}
}

/*
 * function is for management of the concession clerk queue by the Manager
 * */
void queueCcManage(int debugId)
{      
    int i=0, j=0, k=0;
    for( i=0; i<mtCb.numCC; i++)
    {
        int currentCustCount = getTotalCustCount(debugId, &mtCb.queue[CC_QUEUE][i]);
        if( currentCustCount < 3 && currentCustCount !=0 && getCcState(debugId, i) != ON_BREAK )
        { /*we have fewer than 3 people in line and concession clerk is not on break*/
            k=0;
            for( j=0; j<mtCb.numCC; j++)
            {
                if(j != i && ( getCcState(debugId, j) == ON_BREAK || mtCb.cc[j].msgByMan==GO_ON_BREAK))
                    k++;/*number of other TCs on break or who've have been asked by manager to go on break*/
            }
            if(k != (mtCb.numCC - 1) && checkChances(debugId, 20))
            {/*if not everyone else is on break, we check chances if this conc clerk can go on break - and, if he can then we...*/
                mtCb.cc[i].msgByMan=GO_ON_BREAK;
			AcquireLock(printLock);/*PRINT*/
			Print("Manager has told ConcessionClerk %d to go on break.\n", i,-1,-1);
			ReleaseLock(printLock);
            }
        }
        if( currentCustCount > 5 )
        {/*if this employee has more than 5 customers to handlle, we need to get someone off from break*/
            k=0;
            for( j=0; j<mtCb.numCC; j++)
            {/*we are looking for other TCs*/

                if(j != i && getCcState(debugId, j) == ON_BREAK)
                {
                    signalToCcLock(debugId, j);
                    break;
                }
            }
        }
    }
}

/*
 * customer is paying the concession clerk
 * */
void transferMoneyFromCustToCc(int debugId, Cust *fromCust, int toCcId)
{
	
	fromCust->money = fromCust->money - mtCb.cc[toCcId].msgBuffer;
	mtCb.cc[toCcId].money = mtCb.cc[toCcId].money +  mtCb.cc[toCcId].msgBuffer;
}

/*
 * the concession clerk's main program
 * */
void ccMain()
{
    int selfId = 0;
    int debugId;
    int ret_val=0;
    int i=0;
    int currentState;
    CC *self;
    int popcorns, sodas;
    end_fork_thread(&selfId);



    self = &mtCb.cc[selfId];/*set up a pointer to itself*/
    currentState = getCcState(debugId, selfId);/*get the current state*/
    if( currentState == STARTED_BY_MAN)/*clerk has just been started */
    {
        addAddressToQueue(debugId, selfId, CC_QUEUE, selfId);/*we associate a concession clerk's queue with this clerk*/
        changeCcState(debugId, selfId, FREE_AND_SELLING_FOOD_BEING_FIRST);/*he is now ready to sell*/
    }


    acquireCcLock(debugId, selfId);/*he'll acquire the lock for itself*/


    while(1)
    {
	
        currentState = getCcState(debugId, selfId);
        if( self->msgByMan == GO_ON_BREAK)
        {/*we check if manager asked him to go on break*/

            changeCcState(debugId, selfId, ON_BREAK);/*change state to go on break*/
	    AcquireLock(printLock);/*PRINT*/
	    Print("ConcessionClerk %d is going on break.\n", selfId,-1,-1);
	    ReleaseLock(printLock);

	    while(getTotalCustCount(debugId, &mtCb.queue[CC_QUEUE][selfId]) > 0)
            {/*there are customers in the clerk's queue*/
                self->currentCust = queueRemoveCust(debugId, &mtCb.queue[CC_QUEUE][selfId]);/*remove a customer from queue*/
                if(self->currentCust != NULL)
                {
                     while(getCustState(debugId, self->currentCust->selfId, self->currentCust->grpId) != WAIT)/*if customer is not waiting for some CV (and thus, asleep), then we yield thread to take it to back of ready queue*/
			     YIELD();

                    changeCustState(debugId, self->currentCust->selfId, self->currentCust->grpId, SIGNAL); /*if the customer was on wait, he is ready to move on*/
		    while(getCustState(debugId, self->currentCust->selfId, self->currentCust->grpId) == SIGNAL)
			    YIELD();
                }
                self->currentCust = NULL;/*we clear data before beginning new iteration*/
            }
            broadcastAllCcLock(debugId, selfId);/*we'll wake up everyone waiting on that tc's lock*/
            self->msgByMan = INVALID_MSG; /*this is to ensure that the msg is dealt with only once*/
            waitOnCcLock(debugId, selfId);/*we now go to wait for lock*/
			AcquireLock(printLock);/*PRINT*/
			Print("ConcessionClerk %d is coming off break.\n", selfId,-1,-1);
			ReleaseLock(printLock);
            changeCcState(debugId, selfId, FREE_AND_SELLING_FOOD); /*back from break and ready to sell again  */

  
        }
        if( self->msgByMan == SELL_FOOD )
        {/*manager has asked to sell food*/
            changeCcState(debugId, selfId, FREE_AND_SELLING_FOOD);/*set state to selling food*/
            self->msgByMan = INVALID_MSG;/*message taken care of, so removing from memory*/
        }
        else if( currentState == FREE_AND_SELLING_FOOD ||
                currentState == FREE_AND_SELLING_FOOD_BEING_FIRST )
        {
            if(getTotalCustCount(debugId, &mtCb.queue[CC_QUEUE][selfId]) == 0 )
            {/*there aint nobody in line*/
		    AcquireLock(printLock);/*PRINT*/
		    Print("ConcessionClerk %d has no one in line. I am available for a customer.\n", selfId, -1,-1 );
		    ReleaseLock(printLock);
                changeCcState(debugId, selfId, FREE_AND_SELLING_FOOD_BEING_FIRST);/*ready for a new queue of customers*/
            }   
            else
            {   /*there are people in queue*/

                self->currentCust = queueRemoveCust(debugId, &mtCb.queue[CC_QUEUE][selfId]);/*we try and see if there's anyone in line*/
                if(self->currentCust != NULL)
                {/*we take out first customer from queue*/
                    while(getCustState(debugId, self->currentCust->selfId, self->currentCust->grpId) != WAIT)
			    YIELD();
				AcquireLock(printLock);/*PRINT*/
				Print("ConcessionClerk %d has a line length %d and is signalling a customer.\n", selfId, getTotalCustCount(debugId, &mtCb.queue[CC_QUEUE][selfId])+1,-1 );/*ticket clerk is calling up customer*/
				ReleaseLock(printLock);
                    changeCustState(debugId, self->currentCust->selfId, self->currentCust->grpId, SIGNAL);


                    waitOnCcLock(debugId, selfId);

		    popcorns =  self->currentCust->msgBuffer;
                    self->msgBuffer = self->currentCust->msgBuffer * PER_POPCORN_COST; /*computing cost for popcorn*/
                              


                    signalToCcLock(debugId, selfId); /*waking up customer to get him to pay money*/
                    waitOnCcLock(debugId, selfId); /*wait for customer to pay*/


		    sodas =  self->currentCust->msgBuffer;
                    self->msgBuffer = self->msgBuffer + self->currentCust->msgBuffer * PER_SODA_COST;  /*computing cost for soda*/
                    AcquireLock(printLock);/*PRINT*/
					Print("ConcessionClerk %d has an order for %d popcorn and %d soda. ", selfId, popcorns, sodas);
					Print("The cost is %d\n", self->msgBuffer,-1,-1);
					ReleaseLock(printLock);

                    transferFoodFromTo(debugId, self, self->currentCust); /*giving soda and popcorn*/

                    signalToCcLock(debugId, selfId);

                    waitOnCcLock(debugId, selfId);
                    AcquireLock(printLock);/*PRINT*/
                    Print("ConcessionClerk %d has been paid for the order.\n", selfId, -1,-1);
                    ReleaseLock(printLock);                   


					{/* Test 2 specific*/
						if(mtTest2 == 1)
						{
							mtTest2Called++;
							if(mtTest2Called >= 5)
							{
								AcquireLock(exitLock);/*PRINT*/
								changeCcState(debugId, selfId, WAIT_AS_OVER);    
								WaitCV(exitLock, exitCV);/*we now go to wait for lock*/
							}
						}
					}




                }
            }   
        }

        /*here finding out is there is any group at cc or not, if there is not any group left at cc, so cc should be returned.*/
        for (i=0; i<mtCb.numCustGrp; i++)
        {
            if( getCustGrpLocation(debugId, i) != TICKET_CLERK && getCustGrpLocation(debugId, i) != START && getCustGrpLocation(debugId, i) != CONCESSION_CLERK );
            else  break;
        }
        if( i == mtCb.numCustGrp)
	{

		AcquireLock(exitLock);/*PRINT*/
		changeCcState(debugId, selfId, WAIT_AS_OVER); 
		WaitCV(exitLock, exitCV);/*we now go to wait for lock*/

		/*Exit(0);
		  return;*/
	}
	/*end_check(debugId);*/
   
	YIELD();
    }   
}

/*
 * the ticket clerk's main program
 * */
void tcMain()
/*void tcMain(int achintya)*/
{
	int selfId = 0;
	int debugId;
	int ret_val=0;
	int i=0;
	TC *self;
	int currentState;
	end_fork_thread(&selfId);

	debugId = selfId+200;




	self = &mtCb.tc[selfId]; /*set up a pointer to itself*/

	currentState = getTcState(debugId, selfId); /*get the current state*/

	if(mtTest4 == 0)
	{
		if( currentState == STARTED_BY_MAN)/*clerk has just been started*/
		{
			addAddressToQueue(debugId, selfId, TC_QUEUE, selfId);/*we associate a ticket clerk's queue with this ticket clerk*/
			changeTcState(debugId, selfId, FREE_AND_SELLING_TICKET_BEING_FIRST);/*he is now ready to sell*/

		}
	}
	acquireTcLock(debugId, selfId);/*he'll acquire the lock for itsel*/

	while(1)
	{
	
		currentState = getTcState(debugId, selfId);
		if( self->msgByMan == GO_ON_BREAK)
		{/*manager asks the clerk to go on break*/

			changeTcState(debugId, selfId, ON_BREAK);/*change state to go on break*/
			AcquireLock(printLock);/*PRINT*/
			Print("TicketClerk %d is going on break.\n", selfId, -1, -1 );
			ReleaseLock(printLock);
			if(mtTest4 == 1)
			{ /*specific to test case 4*/
				mtCb.tc[1].queue = &mtCb.queue[TC_QUEUE][1];
				mtCb.tc[1].queue->queueType = TC_QUEUE;
				mtCb.tc[1].queue->queueAddress = (Employee)&mtCb.tc[1];

				waitOnTcLock(debugId, selfId);
				AcquireLock(printLock);/*PRINT*/
				Print("TicketClerk %d is coming off break.\n", selfId, -1,-1);
				ReleaseLock(printLock);
				mtTest4Called++;
				AcquireLock(exitLock);/*PRINT*/
				changeTcState(debugId, selfId, WAIT_AS_OVER);    /*back from break and ready to sell again*/
				WaitCV(exitLock, exitCV);/*we now go to wait for lock*/


				/*Exit(0);*/

			}
			else
			{
				while(getTotalCustCount(debugId, &mtCb.queue[TC_QUEUE][selfId]) > 0)
				{/*there are customers in the clerk's queue*/
					self->currentCust = queueRemoveCust(debugId, &mtCb.queue[TC_QUEUE][selfId]); /*remove a customer from queue*/

					if(self->currentCust != NULL)
					{
						while(getCustState(debugId, self->currentCust->selfId, self->currentCust->grpId) != WAIT) /*if customer is not waiting for some CV (and thus, asleep), then we yield thread to take it to back of ready queue*/
							YIELD();
						changeCustState(debugId, self->currentCust->selfId, self->currentCust->grpId, SIGNAL); /*if the customer was on wait, he is ready to move on*/
						while(getCustState(debugId, self->currentCust->selfId, self->currentCust->grpId) == SIGNAL)
							YIELD();
					}
					self->currentCust = NULL;/*we clear data before beginning new iteration*/
				}
				broadcastAllTcLock(debugId, selfId);/*we'll wake up everyone waiting on that tc's lock*/
				self->msgByMan = INVALID_MSG;/*this is to ensure that the msg is dealt with only once*/
				waitOnTcLock(debugId, selfId);/*we now go to wait for lock*/
				AcquireLock(printLock);/*PRINT*/
				Print("TicketClerk %d is coming off break.\n", selfId, -1,-1);
				ReleaseLock(printLock);
				changeTcState(debugId, selfId, FREE_AND_SELLING_TICKET);    /*back from break and ready to sell again*/
			}
		}
		if( self->msgByMan == SELL_TICKETS )
		{/*manager has asked to sell tickets*/
			changeTcState(debugId, selfId, FREE_AND_SELLING_TICKET);/*set state to selling tickets*/
			self->msgByMan = INVALID_MSG;
		}
		else if( currentState == FREE_AND_SELLING_TICKET ||
				currentState == FREE_AND_SELLING_TICKET_BEING_FIRST )
		{/*if tickets are currently being sold*/
			if(getTotalCustCount(debugId, &mtCb.queue[TC_QUEUE][selfId]) == 0 )
			{/*there aint nobody in line*/
				AcquireLock(printLock);/*PRINT*/
				Print("TicketClerk %d has no one in line. I am available for a customer.\n", selfId, -1,-1 );
				ReleaseLock(printLock);
				changeTcState(debugId, selfId, FREE_AND_SELLING_TICKET_BEING_FIRST);/*ready for a new queue of customers*/
			}	
			else
			{	
				self->currentCust = queueRemoveCust(debugId, &mtCb.queue[TC_QUEUE][selfId]);/*we try and see if there's anyone in line*/
				if(self->currentCust != NULL)
				{/*we take out first customer from queue*/
					while(getCustState(debugId, self->currentCust->selfId, self->currentCust->grpId) != WAIT)
						YIELD();
/*until customer and clerk are ready to interact we go on wait*/

					AcquireLock(printLock);/*PRINT*/
					Print("TicketClerk %d has a line length %d and is signalling a customer.\n", selfId, getTotalCustCount(debugId, &mtCb.queue[TC_QUEUE][selfId])+1 , -1);/*ticket clerk is calling up customer*/
					ReleaseLock(printLock);


					changeCustState(debugId, self->currentCust->selfId, self->currentCust->grpId, SIGNAL); /*customer is now awoken from wait*/
					AcquireLock(printLock);/*PRINT*/
					Print("Customer %d in Group %d is walking up to TicketClerk %d ", self->currentCust->selfId, self->currentCust->grpId, selfId);
					Print("to buy %d tickets\n", mtCb.custGrp[self->currentCust->grpId].numCust,-1,-1);
					ReleaseLock(printLock);



					self->msgBuffer = mtCb.custGrp[self->currentCust->grpId].numCust * PER_TICKET_COST;/*computing cost*/

					AcquireLock(printLock);/*PRINT*/
					Print("TicketClerk %d has an order for %d and the cost is %d\n", selfId, mtCb.custGrp[self->currentCust->grpId].numCust, self->msgBuffer );
					ReleaseLock(printLock);


					signalToTcLock(debugId, selfId);/*waking up customer to get him to pay money*/



					waitOnTcLock(debugId, selfId);/*wait for customer to pay*/



					transferTicketFromTo(debugId, self, self->currentCust);/*giving ticket*/


					signalToTcLock(debugId, selfId);
					waitOnTcLock(debugId, selfId);


					{/* Test 3 specific*/
						if(mtTest3 == 1)
						{
							mtTest3Called++;
							if(mtTest3Called >= 5)
							{
								AcquireLock(exitLock);/*PRINT*/
								changeTcState(debugId, selfId, WAIT_AS_OVER);    /*back from break and ready to sell again*/
								WaitCV(exitLock, exitCV);/*we now go to wait for lock*/

							}
						}
					}
				}
			}	
		}
		if(mtTest2 == 1)
		{	
			if(mtTest2Called == 5)
			{
				AcquireLock(exitLock);/*PRINT*/
				changeTcState(debugId, selfId, WAIT_AS_OVER);    /*back from break and ready to sell again*/
				WaitCV(exitLock, exitCV);/*we now go to wait for lock*/
			}
		}	
		/*here finding out is there is any group at tc or not, if there is not any group left at tc, so tc should be returned.*/
		for (i=0; i<mtCb.numCustGrp; i++)
		{
			if( getCustGrpLocation(debugId, i) != TICKET_CLERK && getCustGrpLocation(debugId, i) != START );
			else  break;
		}
		if( i == mtCb.numCustGrp)
		{
                   
			AcquireLock(exitLock);/*PRINT*/
			changeTcState(debugId, selfId, WAIT_AS_OVER);    /*back from break and ready to sell again*/
			WaitCV(exitLock, exitCV);/*we now go to wait for lock*/

			/*Exit(0);
			  return;*/
		}
		/*end_check(debugId);*/
	
		YIELD();

	}	
}

/*
 * The Ticket Taker Main Program
 * */
void ttMain()
/*void ttMain(int achintya)*/
{
	int selfId = 0;
	int debugId;
	int ret_val=0;
	int i=0;
	TT *self; /*set up a pointer to itself*/
	int currentState;
	int groupInLobby=0;
	end_fork_thread(&selfId);

	debugId = selfId+400;




	self = &mtCb.tt[selfId]; /*set up a pointer to itself*/


	currentState = getTtState(debugId, selfId); /*get the current state*/
	if( currentState == STARTED_BY_MAN)/*ticket taker has just been started by manage*/
	{
		addAddressToQueue(debugId, selfId, TT_QUEUE, selfId);;/*we associate a ticket taker's queue with this ticket taker*/
		changeTtState(debugId, selfId, FREE_AND_TAKING_TICKET_BEING_FIRST);
	}


	acquireTtLock(debugId, selfId);/*get lock before modifying*/
	while(1)
	{
		currentState = getTtState(debugId, selfId);
		if( currentState == MAN_STARTING_MOVIE )
		{ /*manager asked for movie to start*/
			changeTtState(debugId, selfId, ON_BREAK);
			AcquireLock(printLock);/*PRINT*/
			Print("TicketTaker %d is going on break.\n", selfId, -1,-1);
			ReleaseLock(printLock);

			while(getTotalCustCount(debugId, &mtCb.queue[TT_QUEUE][selfId]) > 0)
			{/*if there is somebody in queue*/
				self->currentCust = queueRemoveCust(debugId, &mtCb.queue[TT_QUEUE][selfId]);/*take someone from queue*/
				if(self->currentCust != NULL)
				{
					while(getCustState(debugId, self->currentCust->selfId, self->currentCust->grpId) != WAIT)
						YIELD();
					changeCustState(debugId, self->currentCust->selfId, self->currentCust->grpId, SIGNAL); /*if the customer was  on wait, he is ready to move on*/
					while(getCustState(debugId, self->currentCust->selfId, self->currentCust->grpId) == SIGNAL)
						YIELD();
				}
				self->currentCust = NULL;/*we clear data before beginning new iteration*/
			}
			broadcastAllTtLock(debugId, selfId); /*we'll wake up everyone waiting on that tt's lock*/
			self->msgByMan = INVALID_MSG;/*this is to ensure that the msg is dealt with only once*/
			waitOnTtLock(debugId, selfId);/*we now go to wait for lock*/
			AcquireLock(printLock);/*PRINT*/
			Print("TicketTaker %d is coming off break.\n", selfId, -1,-1);
			ReleaseLock(printLock);
			changeTtState(debugId, selfId, FREE_AND_TAKING_TICKET);  /*back from break to take tickets again */
		}
		else if( self->msgByMan == GO_ON_BREAK)
		{/*has been asked to go on break*/
			self->msgToMan = INVALID_MSG;
			changeTtState(debugId, selfId, ON_BREAK);/*change state to being on break*/
			self->msgByMan = INVALID_MSG;
			AcquireLock(printLock);/*PRINT*/
			Print("TicketTaker %d is going on break.\n", selfId, -1,-1);
			ReleaseLock(printLock);
			while(getTotalCustCount(debugId, &mtCb.queue[TT_QUEUE][selfId]) > 0)
			{/*if queue wasnt empty when he was asked to go on a break*/
				self->currentCust = queueRemoveCust(debugId, &mtCb.queue[TT_QUEUE][selfId]);
				if(self->currentCust != NULL)
				{/* if there was somebody in queue*/
					while(getCustState(debugId, self->currentCust->selfId, self->currentCust->grpId) != WAIT)
						YIELD();
					changeCustState(debugId, self->currentCust->selfId, self->currentCust->grpId, SIGNAL); /*wake up customer if he was sleeping*/
					while(getCustState(debugId, self->currentCust->selfId, self->currentCust->grpId) == SIGNAL)
						YIELD();
				}
				self->currentCust = NULL;
			}
			broadcastAllTtLock(debugId, selfId);/*wake up all those who were waiting for the ticket taker's lock*/
			self->msgByMan = INVALID_MSG;
			waitOnTtLock(debugId, selfId);;/*wait for its own lock*/
			AcquireLock(printLock);/*PRINT*/
			Print("TicketTaker %d is coming off break.\n", selfId, -1, -1);
			ReleaseLock(printLock);
			changeTtState(debugId, selfId, FREE_AND_TAKING_TICKET);
		}
		else if( self->msgByMan == FILL_CUST )
		{/*manager has asked taker to start taking tickets*/

			changeTtState(debugId, selfId, FREE_AND_TAKING_TICKET);/*setting state of ticket taker as ready to take the first ticket*/
			self->msgByMan = INVALID_MSG;;/*message was received - now we dissolve it to ensure it is not taken care of again*/
		}
		if( currentState == FREE_AND_TAKING_TICKET ||
				currentState == FREE_AND_TAKING_TICKET_BEING_FIRST )
		{/*if the taker is currently taking tickets*/
			if(getTotalCustCount(debugId, &mtCb.queue[TT_QUEUE][selfId]) == 0 )
			{/*if no one is in queue*/
				AcquireLock(printLock);/*PRINT*/
				Print("TicketTaker %d has no one in line. I am available for a customer.\n", selfId,  -1, -1);
				ReleaseLock(printLock);
				changeTtState(debugId, selfId, FREE_AND_TAKING_TICKET_BEING_FIRST);/* we set state as ready to take first ticket again*/
			}
			else
			{/*there is some one in queue - we take them out*/
				self->currentCust = queueRemoveCust(debugId, &mtCb.queue[TT_QUEUE][selfId]);   
				if(self->currentCust != NULL)
				{
					while(getCustState(debugId, self->currentCust->selfId, self->currentCust->grpId) != WAIT)
						YIELD();

					/*wait for customers to be in WAIT state before wqking them up to interact with them*/
					AcquireLock(printLock);/*PRINT*/
					Print("TicketTaker %d has a line length %d and is signalling a customer.\n", selfId, getTotalCustCount(debugId, &mtCb.queue[TT_QUEUE][selfId])+1, -1);
					ReleaseLock(printLock);


					if( mtCb.custGrp[self->currentCust->grpId].numCust <= (MAX_SEATS - getNumOfTicketTaken(debugId)) )
					{/*movie room has sufficient seats for this group*/
						self->msgToCust = YES;
					}
					else
					{	/*movie room diesnt have sufficient seats for this group*/
						AcquireLock(printLock);/*PRINT*/
						Print("TicketTaker %d is not allowing the group into the theater. The number of taken tickets is %d", selfId,  getNumOfTicketTaken(debugId), -1);
						Print(" and the group size is %d.\n", mtCb.custGrp[self->currentCust->grpId].numCust, -1, -1);
						ReleaseLock(printLock);
						self->msgToCust = NO;
					}
					changeCustState(debugId, self->currentCust->selfId, self->currentCust->grpId, SIGNAL);/*now we awake the customer who was waiting for tt to check*/
					signalToTtLock(debugId, selfId);/*tt now let others get access to its CV*/
					waitOnTtLock(debugId, selfId);/* it now goes on wait   */
				}
			}  
		}
		groupInLobby=0;

		/*here finding out if there is any group in the lobby. If there is any then if its size < than the number of seats left in the movieroom, then it shuold be first accomodated. Else movie should be started.*/
		for (i=0; i<mtCb.numCustGrp; i++)
		{
			if(
					mtCb.custGrp[i].location == START ||
					mtCb.custGrp[i].location == TICKET_CLERK ||
					mtCb.custGrp[i].location == CONCESSION_CLERK ||
					mtCb.custGrp[i].location == LOBBY
			  )
			{
				groupInLobby=1;/*there is someone in lobby*/
				if( ( mtCb.custGrp[i].numCust <= (MAX_SEATS - getNumOfTicketTaken(debugId)) ))
				{
					changeTtState(debugId, selfId, FREE_AND_TAKING_TICKET);
					break;
				}
			}
		}
		if(i == mtCb.numCustGrp && groupInLobby==1)
		{/*we have checked all groups in lobby and accomodated them if possible*/
			self->msgToMan = START_MOVIE;
			AcquireLock(printLock);/*PRINT*/
			Print("TicketTaker %d has stopped taking tickets\n", selfId, -1, -1);
			ReleaseLock(printLock);
			changeTtState(debugId, selfId, MAN_STARTING_MOVIE);

		}

		/*here finding out is there is any group in lobby or movieroom, if there is not any group left in lobby and movieroom then tt should Exit the theater - his job is over as everyone of customers who bought tickets have shown him tickets.*/
		for (i=0; i<mtCb.numCustGrp; i++)
		{
			if(
					getCustGrpLocation(debugId, i) != START &&
					getCustGrpLocation(debugId, i) != TICKET_CLERK &&
					getCustGrpLocation(debugId, i) != CONCESSION_CLERK &&
					getCustGrpLocation(debugId, i) != LOBBY &&
					getCustGrpLocation(debugId, i) != MOVIEROOM
			  );
			else  break;
		}
		if( i == mtCb.numCustGrp)
		{/*all tickets taken by ticket clerk - everyone's been to movie room*/
			AcquireLock(printLock);/*PRINT*/
			Print("TicketTaker %d has stopped taking tickets\n", selfId,-1,-1);
			ReleaseLock(printLock);

			AcquireLock(exitLock);/*PRINT*/
			changeTtState(debugId, selfId, WAIT_AS_OVER);    /*back from break and ready to sell again*/
			WaitCV(exitLock, exitCV);/*we now go to wait for lock*/

			/*Exit(0);
			return;*/
		}

		/*end_check(debugId);*/

		/*here finding out, is there is any group in lobby or not, if there is not any group left in lobby then movie should be started.*/
		for (i=0; i<mtCb.numCustGrp; i++)
		{
			int grpLoc = getCustGrpLocation(debugId, i);
			if(
					grpLoc != START &&
					grpLoc != TICKET_CLERK &&
					grpLoc != CONCESSION_CLERK &&
					grpLoc != LOBBY
			  );
			else  break;
		}
		if( i == mtCb.numCustGrp)
		{
			self->msgToMan = START_MOVIE;
			AcquireLock(printLock);/*PRINT*/
			Print("TicketTaker %d has stopped taking tickets\n", selfId,-1,-1);
			ReleaseLock(printLock);
			changeTtState(debugId, selfId, MAN_STARTING_MOVIE);
		}
		YIELD();
	}   
}

/*
 * Movie Technician Main's Program
 * */
void mtMain()
{
	int selfId = 0;
	int ret_val=0;
	int debugId;
	MT *self;/*creating pointer to self*/
	Cust *curentCust;
	int currentState;
	int i=0, j=0, num=0;
	int custState;
	int movieDuration;

	end_fork_thread(&selfId);

	debugId = selfId+500;

	self = &mtCb.mt;/*creating pointer to self*/

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

			AcquireLock(exitLock);/*PRINT*/
			/*changeCcState(debugId, selfId, WAIT_AS_OVER);*/ 
			WaitCV(exitLock, exitCV);/*we now go to wait for lock*/

			/*return;*/
		}
		if((getMtState(debugId) == MOVIE_IS_PLAYING) && (mtCb.man.msgToAll == MOVIE_RUNNING))
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
					Cust *currentCust = (Cust *)mtCb.mvRoom.seat[i][j].cust;
					if( currentCust!=NULL )
					while(getCustState(debugId, currentCust->selfId, currentCust->grpId) != SEAT_TAKEN)
					{
						YIELD();
					}
				}
			}
			AcquireLock(printLock);/*PRINT*/
			Print("The MovieTechnician has started the movie\n",-1,-1,-1);
			ReleaseLock(printLock);
	
			for(i=0;i<movieDuration;i++)
			{/*the movie is beingplayed by calling thread->yields				*/
				YIELD();
			}	
			AcquireLock(printLock);/*PRINT*/
			Print("The MovieTechnician has ended the movie\n",-1,-1,-1);
			ReleaseLock(printLock);
			/*The movie is now over.. Have to tell manager*/
			/*The movie is now over so changing back status of ticket owners to READY_TO_LEAVE_MOVIE_ROOM*/
			for(i=0; i<MAX_ROWS; i++)
			{
				for(j=0; j<MAX_COLS; j++) /*These customers have seen the movie they now ought to leave*/
				{
					Cust *currentCust = (Cust *)mtCb.mvRoom.seat[i][j].cust;
					if( currentCust!=NULL && currentCust->seatTaken == 1 && getCustState(debugId, currentCust->selfId, currentCust->grpId) == SEAT_TAKEN)
					{
						changeCustState(debugId, currentCust->selfId, currentCust->grpId, READY_TO_LEAVE_MOVIE_ROOM);    /*customers shall be awoken here - SIGNAL them*/
					}
				}
			}
			AcquireLock(printLock);/*PRINT*/
			Print("The MovieTechnician has told customers to leave the theater room\n",-1,-1,-1);
			ReleaseLock(printLock);
			changeMtState(debugId, MOVIE_IS_NOT_PLAYING);

			YIELD();
		}
		YIELD();
		
	}	
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
	if(queueTcLock!=NULL) AcquireLock(queueTcLock);
        state = getTcState(debugId, queueId);
        num_ele=mtCb.numTC;
    }
    else if(queueType == CC_QUEUE)
    {
	if(queueCcLock!=NULL) AcquireLock(queueCcLock);
        num_ele=mtCb.numCC;
    }
    else if(queueType == TT_QUEUE)
    {
	if(queueTtLock!=NULL) AcquireLock(queueTtLock);
        num_ele=mtCb.numTT;
    }
    for(queueId=0; queueId < num_ele; queueId++)
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
        if(getTotalCustCount(debugId, &mtCb.queue[queueType][queueId]) < numCust)
        {
            if(mtCb.queue[queueType][queueId].queueAddress != NULL)
            {
                if(state != ON_BREAK)
                {/*this is to manage if a queue has too many ppl - we get someone off from break*/
                    numCust = getTotalCustCount(debugId, &mtCb.queue[queueType][queueId]);
                    reqdQueueId = queueId;
                }
            }
        }
    }
    if(reqdQueueId == -1)
    {
	    if(queueType == TC_QUEUE)
	    {
		    if(queueTcLock!=NULL) ReleaseLock(queueTcLock);
	    }
	    else if(queueType == CC_QUEUE)
	    {
		    if(queueCcLock!=NULL) ReleaseLock(queueCcLock);
	    }
	    else if(queueType == TT_QUEUE)
	    {
		    if(queueTtLock!=NULL) ReleaseLock(queueTtLock);
	    }
	    return NOK;
    }
    if(queueType == TC_QUEUE)
    {
        tcState = getTcState(debugId, reqdQueueId);
        if(tcState != ON_BREAK)
        {
	    if(mtTest1 == 1)
	    {
		    AcquireLock(printLock);
		    Print("Customer %d in Group %d is getting in TicketClerk line %d\n", self->selfId, self->grpId, reqdQueueId );
		    ReleaseLock(printLock);
		    mtCb.queue[TC_QUEUE][reqdQueueId].numCust++;
		    if(queueTcLock!=NULL) ReleaseLock(queueTcLock);
		    changeCustState(debugId, self->selfId, self->grpId, WAIT);
	    }
	    if(mtTest4 == 1)
	    {
		    AcquireLock(printLock);
		    Print("Customer %d in Group %d is getting in TicketClerk line %d\n", self->selfId, self->grpId, reqdQueueId );
		    ReleaseLock(printLock);
		    mtCb.queue[TC_QUEUE][reqdQueueId].numCust++;
		    if(queueTcLock!=NULL) ReleaseLock(queueTcLock);
		    mtTest4Called++;
		    changeCustState(debugId, self->selfId, self->grpId, WAIT_AS_OVER);
	    }
            ret=queueAddCust(debugId, &mtCb.queue[queueType][reqdQueueId], self);/*attempting to add customer to queue of queueType*/
            if(ret != NO_ADDRESS )
            {/*there was a line ot get into*/
				AcquireLock(printLock);
                Print("Customer %d in Group %d is getting in TicketClerk line %d\n", self->selfId, self->grpId, reqdQueueId );
				ReleaseLock(printLock);

                self->queue = &mtCb.queue[queueType][reqdQueueId];
		if(queueTcLock != NULL) ReleaseLock(queueTcLock);
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
		    if(queueTcLock != NULL) ReleaseLock(queueTcLock);
		    return NOK;
            }
        }
        else if(tcState == ON_BREAK)
        {/* we see clerk on lock*/
	    if(queueTcLock != NULL) ReleaseLock(queueTcLock);/* we let go of lock for queue*/
			AcquireLock(printLock);/*PRINT*/
            Print("Customer %d in Group %d sees TicketClerk %d is on break\n", self->selfId, self->grpId, reqdQueueId );
			ReleaseLock(printLock);
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
				AcquireLock(printLock);/*PRINT*/
                Print("Customer %d in Group %d is getting in ConcessionClerk line %d\n", self->selfId, self->grpId, reqdQueueId );
				ReleaseLock(printLock);
                self->queue = &mtCb.queue[queueType][reqdQueueId];
		if(queueCcLock != NULL) ReleaseLock(queueCcLock);
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
		    if(queueCcLock != NULL) ReleaseLock(queueCcLock);
		    return NOK;
            }
        }
        else if (ccState == ON_BREAK)
        {
		if(queueCcLock != NULL) ReleaseLock(queueCcLock);
			AcquireLock(printLock);/*PRINT*/
            Print("Customer %d in Group %d sees ConcessionClerk %d is on break\n", self->selfId, self->grpId, reqdQueueId );
			ReleaseLock(printLock);
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
				AcquireLock(printLock);/*PRINT*/
                Print("Customer %d in Group %d is getting in TicketTaker line %d\n", self->selfId, self->grpId, reqdQueueId );
				ReleaseLock(printLock);

                self->queue = &mtCb.queue[queueType][reqdQueueId];
		if(queueTtLock != NULL) ReleaseLock(queueTtLock);
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
		    if(queueTtLock != NULL) ReleaseLock(queueTtLock);
                return NOK;
            }
        }
        else if(ttState == ON_BREAK)
        {
	    if(queueTtLock != NULL) ReleaseLock(queueTtLock);
			AcquireLock(printLock);/*PRINT*/
            Print("Customer %d in Group %d sees TicketTaker %d is on break\n", self->selfId, self->grpId, reqdQueueId );
			ReleaseLock(printLock);
        }           
    }
    return ROK;
}


void transferMoneyFromCustToTc(int debugId, Cust *fromCust, int toTcId)
{ /*showing that customer is pay for his buys - to ticket clerk*/
	fromCust->money = fromCust->money - mtCb.tc[toTcId].msgBuffer;
	mtCb.tc[toTcId].money = mtCb.tc[toTcId].money +  mtCb.tc[toTcId].msgBuffer;
}


int askPopcorn(int debugId, Cust *self)
{/*the group head is asking others if they need popcorn*/
	int i=0;
	int numPopcorn=0;
	for(i=0; i < mtCb.custGrp[self->grpId].numCust; i++)
	{/*there is a 75% probability that popcorn will be needed - this function checkChances will give 1 if that is the case*/
		mtCb.custGrp[self->grpId].cust[i].takePopcorn = checkChances(debugId, 75);
		if(mtCb.custGrp[self->grpId].cust[i].takePopcorn)
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
	for(i=0; i < mtCb.custGrp[self->grpId].numCust; i++)
	{
		mtCb.custGrp[self->grpId].cust[i].takeSoda = checkChances(debugId, 75);/*there is a 75% probability that soda will be needed - this function checkChances will give 1 if that is the case*/
		if(mtCb.custGrp[self->grpId].cust[i].takeSoda)
		{
			numSoda++;/*total number of soda for that group enhanced by 1*/
		}
	}
	return numSoda;
}

/*
 * Customer Main Program
 * */
void custMain()
{
	int selfId = 0;
	int popcorns, sodas;
	int debugId, i, j, k, seatsTaken, seatTook;
	int ret_val=0;
	Cust *self;
	CustGrp *selfGrp;
	int useDuration;
	end_fork_thread(&selfId);

	debugId = selfId+1000;

/*initializing some values before beginning interactions*/
	i=selfId / 10;
	j=selfId % 10;
	k=0;
	seatsTaken = 0;
	seatTook = 0;
	selfId=j;
	self = &mtCb.custGrp[i].cust[j];
	selfGrp = &mtCb.custGrp[i];
	

	while(1)
	{
		if(getCustState(debugId, selfId, self->grpId) == STARTED_BY_MAIN)/*customers have been just created*/
		{
			if(self->IAMTicketBuyer)/*if this is the group head*/
			{	
				AcquireLock(printLock);/*PRINT*/
				Print("Customer %d in Group %d has entered the movie theater.\n", selfId, self->grpId, -1);
				ReleaseLock(printLock);
				for(i=1; i< mtCb.custGrp[self->grpId].numCust; i++)
				{   /*these are other customers in the group - they all wait for him to get back from ticket clerk*/
					while(getCustState(debugId, i, self->grpId) != WAIT_FOR_TICKET_BUYER_FROM_TC)
					{ YIELD(); }
				}
				changeCustState(debugId, self->selfId, self->grpId, LOOKING_FOR_TC_QUEUE);/*it will now look to enter queue*/
				changeCustGrpLocation(debugId, self->grpId, TICKET_CLERK);/*the group is now at ticket clerk*/
			}
			else
			{	/*this aint group head*/
				AcquireLock(printLock);/*PRINT*/
				Print("Customer %d of Group %d is waiting for HeadCustomer\n", self->selfId, self->grpId, -1);
				ReleaseLock(printLock);
				changeCustState(debugId, self->selfId, self->grpId, WAIT_FOR_TICKET_BUYER_FROM_TC);/*non group heading customers are waiting for him*/
			}
		}
		if(getCustState(debugId, self->selfId, self->grpId) == LOOKING_FOR_TC_QUEUE)
		{ /*if this thread is waiting for a place in a tc queue (will only happen if this grp head)*/
			if(mtTest6==1) /*specific to test case 6*/
			{
				AcquireLock(printLock);/*PRINT*/
				Print("Customer %d in Group %d is getting in TicketClerk line 0\n", self->selfId, self->grpId,-1 );
				Print("Customer %d in Group %d is leaving TicketClerk 0\n", self->selfId, self->grpId,-1);
				ReleaseLock(printLock);
				changeCustState(debugId, self->selfId, self->grpId, GOT_TICKET);
				AcquireLock(printLock);/*PRINT*/
				Print("HeadCustomer %d of Group %d has told the group to proceed\n", self->selfId, self->grpId,-1);
				ReleaseLock(printLock);
				changeCustGrpState(debugId, self->grpId, GOT_TICKET);	
			}
			else
			{
				if(selectAndAddInQueue(debugId, self, TC_QUEUE)==ROK)
				{/*found a queue*/
					if(getCustState(debugId, self->selfId, self->grpId) == SIGNAL)
					{/*the clerk signalled this customer*/
						AcquireLock(printLock);/*PRINT*/
						Print("Customer %d in Group %d in TicketClerk line %d is paying ", self->selfId, self->grpId, self->queue->queueId );
						Print("%d for tickets\n", mtCb.tc[self->queue->queueId].msgBuffer, -1,-1);
						ReleaseLock(printLock);
						transferMoneyFromCustToTc(debugId, self, self->queue->queueId);	
/*giving money and taking tickets*/

						signalToTcLock(debugId,  self->queue->queueId);
/*get tc off lock on CV by giving him money*/

						waitOnTcLock(debugId,  self->queue->queueId);/*wait for him to give me tickets*/
						AcquireLock(printLock);/*PRINT*/
						Print("Customer %d in Group %d is leaving TicketClerk %d\n", self->selfId, self->grpId, self->queue->queueId );
						ReleaseLock(printLock);
						releaseTcLock(debugId,  self->queue->queueId);/*done with ticket clerk so dont need to him lock him up anymore - other customers can use him now*/
						signalToTcLock(debugId,  self->queue->queueId);



						changeCustState(debugId, self->selfId, self->grpId, GOT_TICKET);/*have tickets - let's go*/
						Print("HeadCustomer %d of Group %d has told the group to proceed\n", self->selfId, self->grpId,-1);
						ReleaseLock(printLock);
						changeCustGrpState(debugId, self->grpId, GOT_TICKET);	
					}
				}
			}
		}














		/*now onto concession clerk*/
		if(getCustState(debugId, self->selfId, self->grpId) == GOT_TICKET)
		{
			{/* Test 3 specific*/
				if(mtTest3 == 1)
				{
					changeCustState(debugId, self->selfId, self->grpId, WAIT_AS_OVER);
					/*return;*/
				}
			}

			if(self->IAMTicketBuyer)
			{   /*if this is the group head*/
				for(i=1; i< mtCb.custGrp[self->grpId].numCust; i++)
				{/*these are other customers in the group - they all wait for him to get back from concession clerk*/
					while(getCustState(debugId, i, self->grpId) != WAIT_FOR_TICKET_BUYER_FROM_CC)
					{ YIELD(); }
				}
				changeCustState(debugId, self->selfId, self->grpId, LOOKING_FOR_CC_QUEUE);/*it will now look to enter queue*/
				changeCustGrpLocation(debugId, self->grpId, CONCESSION_CLERK);/*the group is now at concession clerk*/
			}
			else
			{/*this aint group head*/
				AcquireLock(printLock);/*PRINT*/
				Print("Customer %d in Group %d has entered the movie theater.\n", selfId, self->grpId, -1);
				Print("Customer %d of Group %d is waiting for HeadCustomer\n", self->selfId, self->grpId, -1 );/*non group heading customers are waiting for him*/
				ReleaseLock(printLock);
				changeCustState(debugId, self->selfId, self->grpId, WAIT_FOR_TICKET_BUYER_FROM_CC);
			}
		}

		if(getCustState(debugId, self->selfId, self->grpId) == LOOKING_FOR_CC_QUEUE)
		{/*if this thread is waiting for a place in a cc queue (will only happen if this grp head)*/
			if(mtTest6==1) /*Test 6 specific*/
			{
				AcquireLock(printLock);
				Print("Customer %d in Group %d is getting in ConcessionClerk line 0\n", self->selfId, self->grpId, -1 );
				Print("Customer %d in Group %d is leaving ConcessionClerk 0\n", self->selfId, self->grpId, -1 );
				changeCustState(debugId, self->selfId, self->grpId, GOT_FOOD);
				Print("HeadCustomer %d of Group %d has told the group to proceed\n", self->selfId, self->grpId, -1 );
				ReleaseLock(printLock);
				changeCustGrpState(debugId, self->grpId, GOT_FOOD);	
			}
			else
			{
				popcorns = askPopcorn(debugId, self);/*asking itself and other grp members how many popcorns the group needs*/
				sodas = askSoda(debugId, self);/*find gorup's soda requirement*/
				for(i=0; i< mtCb.custGrp[self->grpId].numCust; i++)
				{
					if(mtCb.custGrp[self->grpId].cust[i].takePopcorn || mtCb.custGrp[self->grpId].cust[i].takeSoda)
					{
						AcquireLock(printLock);
						Print("Customer %d in Group %d has %d popcorn and ", self->selfId, self->grpId, mtCb.custGrp[self->grpId].cust[i].takePopcorn );
						Print("%d soda request from a group member\n", mtCb.custGrp[self->grpId].cust[i].takeSoda, -1,-1 );
						ReleaseLock(printLock);
					}
				}


				if(selectAndAddInQueue(debugId, self, CC_QUEUE)==ROK)
				{/*found a line*/

					if(getCustState(debugId, self->selfId, self->grpId) == SIGNAL)
					{/*the clerk signalled this customer*/
						self->msgBuffer = popcorns;/*noting the number needed*/

						signalToCcLock(debugId,  self->queue->queueId);

						waitOnCcLock(debugId,  self->queue->queueId);/*waiting for opportunity to reach cc*/






						self->msgBuffer = sodas;/*noting requirement*/
/*will now go and get it form concession clerk*/
						AcquireLock(printLock);
						Print("Customer %d in Group %d is walking up to ConcessionClerk %d ", self->selfId, self->grpId, self->queue->queueId );
						Print("to buy %d popcorn and %d soda\n", popcorns, sodas, -1 );
						ReleaseLock(printLock);
						signalToCcLock(debugId,  self->queue->queueId);/*wake up cc to get him to give food to me*/

						waitOnCcLock(debugId,  self->queue->queueId);/*wait for him to get back to me with food and then give him money*/





						transferMoneyFromCustToCc(debugId, self, self->queue->queueId);   /*giving money*/
						AcquireLock(printLock);/*PRINT*/
						Print("Customer %d in Group %d in ConcessionClerk line %d ", self->selfId, self->grpId, self->queue->queueId);
						Print("is paying %d for food\n", mtCb.cc[self->queue->queueId].msgBuffer,-1,-1 );
						Print("Customer %d in Group %d is leaving ConcessionClerk %d\n", self->selfId, self->grpId, self->queue->queueId);
						ReleaseLock(printLock);

/*everything for food is done - let's go*/

						releaseCcLock(debugId,  self->queue->queueId);
						signalToCcLock(debugId,  self->queue->queueId);



						changeCustState(debugId, self->selfId, self->grpId, GOT_FOOD);
						AcquireLock(printLock);/*PRINT*/
						Print("HeadCustomer %d of Group %d has told the group to proceed\n", self->selfId, self->grpId, -1); /*head customer asks others to get  move on*/
						ReleaseLock(printLock);

						changeCustGrpState(debugId, self->grpId, GOT_FOOD);   
					}
				}
			}
		}



		if(mtTest2 ==1) /*test2 specific*/
		{
			changeCustState(debugId, self->selfId, self->grpId, WAIT_AS_OVER);
		}









		if(getCustState(debugId,  self->selfId, self->grpId) == GOT_FOOD)
		{
/*food done - now lets try to get into movie room*/
			if(self->IAMTicketBuyer)
			{/*this is group head*/

				for(i=1; i< mtCb.custGrp[self->grpId].numCust; i++)
				{/*these are other customers in the group - they all wait for him to get back from ticket taker*/
					while(getCustState(debugId, i, self->grpId) != WAIT_FOR_TICKET_BUYER_FROM_TT)
					{ YIELD(); }
				}
				changeCustGrpLocation(debugId, self->grpId, LOBBY);/*evryone is in lobby*/
				changeCustState(debugId, self->selfId, self->grpId, LOOKING_FOR_TT);/*the group head is looking for ticket taker*/
			}
			else
			{/*others are waiting for grp head to get back ticket taker*/
				AcquireLock(printLock);/*PRINT*/
				Print("Customer %d of Group %d is waiting for HeadCustomer\n", self->selfId, self->grpId, -1 );
				ReleaseLock(printLock);
				changeCustState(debugId, self->selfId, self->grpId, WAIT_FOR_TICKET_BUYER_FROM_TT);
			}
		}

		if(getCustState(debugId,  self->selfId, self->grpId) == LOOKING_FOR_TT)
		{/*if this thread is waiting for a place in a tt queue (will only happen if this grp head)*/
			if(mtTest6==1) /*specific to test 6*/
			{
				AcquireLock(printLock);/*PRINT*/
				Print("Customer %d in Group %d is getting in TicketTaker line 0\n", self->selfId, self->grpId, -1 );
				Print("Customer %d in Group %d is leaving TicketTaker 0\n", self->selfId, self->grpId, -1 );
				Print("HeadCustomer %d of Group %d has told the group to proceed\n", self->selfId, self->grpId, -1 );
				ReleaseLock(printLock);
				changeCustState(debugId, self->selfId, self->grpId, SEATS_READY);
				changeCustGrpState(debugId, self->grpId, SEATS_READY);
			}
			else
			{
				if(selectAndAddInQueue(debugId, self, TT_QUEUE)==ROK)
				{/*found a line*/
					if(getCustState(debugId, self->selfId, self->grpId) == SIGNAL)
					{    /*the ticket taker signalled this customer      */

						changeTtState(debugId, self->queue->queueId, BUSY_WITH_CUST); /*the tt is now busy with this customer*/
						mtCb.tt[self->queue->queueId].currentCust = NULL;
						if(mtCb.tt[self->queue->queueId].msgToCust == YES)
						{/*ticket taker says there is place inside for this group*/
							if(increaseNumOfTicketTaken(debugId, mtCb.custGrp[self->grpId].numCust ) == ROK)
							{/*add tickets in this group to total tickets with customer*/
								AcquireLock(printLock);/*PRINT*/
								Print("Customer %d in Group %d is walking upto TicketTaker %d to give", self->selfId, self->grpId, self->queue->queueId);
								Print(" %d tickets.\n", mtCb.custGrp[self->grpId].numCust, -1, -1);
								Print("TicketTaker %d is allowing the group into the theater. The number of tickets taken is %d. \n", self->queue->queueId, mtCb.custGrp[self->grpId].numCust, -1);
								ReleaseLock(printLock);
								changeCustGrpLocation(debugId, self->grpId, MOVIEROOM);/*the grp will now move to movie room*/
								signalToTtLock(debugId, self->queue->queueId);
								AcquireLock(printLock);/*PRINT*/
								Print("Customer %d in Group %d is leaving TicketTaker %d\n", self->selfId, self->grpId, self->queue->queueId);/* they have now gone past ticket taker*/
								ReleaseLock(printLock);	
								releaseTtLock(debugId, self->queue->queueId);/*release the ticket taker's lock*/
							}
							else
							{/*the taker is not taking tickets - so the grp head releases its locks and moves back to lobby to his other group mates*/
								signalToTtLock(debugId, self->queue->queueId);
								releaseTtLock(debugId, self->queue->queueId);
								AcquireLock(printLock);/*PRINT*/
								Print("Customer %d in Group %d sees TicketTaker %d is no longer taking tickets. Going to the lobby\n", self->selfId, self->grpId, self->queue->queueId);
								ReleaseLock(printLock);
								changeCustState(debugId, self->selfId, self->grpId, WAIT_IN_LOBBY_AS_TICKET_NOT_TAKEN_BY_TT);
								changeCustState(debugId, self->selfId, self->grpId, LOOKING_FOR_TT);/*he must now look for a working ticket taker again*/
							}
						}   
						else
						{/*ticket taker says sorry no space inside*/
							signalToTtLock(debugId, self->queue->queueId);
							releaseTtLock(debugId, self->queue->queueId);
							AcquireLock(printLock);/*PRINT*/
							Print("Customer %d in Group %d sees TicketTaker %d is no longer taking tickets. Going to the lobby\n", 
							self->selfId, self->grpId, self->queue->queueId);
							ReleaseLock(printLock);
							changeCustState(debugId, self->selfId, self->grpId, WAIT_IN_LOBBY_AS_TICKET_NOT_TAKEN_BY_TT);
							changeCustState(debugId, self->selfId, self->grpId, LOOKING_FOR_TT);/*he must now look for a working ticket taker again*/
						}

					}

				}

				/*once we make it to movie room*/

				if(getCustGrpLocation(debugId, self->grpId) == MOVIEROOM)
				{
					changeCustState(debugId, self->selfId, self->grpId, TICKET_TAKEN);/*We have tickets!!*/
					AcquireLock(printLock);/*PRINT*/
					Print("HeadCustomer %d of Group %d has told the group to proceed\n", self->selfId, self->grpId, -1 );
					ReleaseLock(printLock);
					changeCustGrpState(debugId, self->grpId, TICKET_TAKEN); 
				}
				YIELD();/*now we wait for others for do their work*/
				if(mtCb.man.msgToAll == MOVIE_RUNNING && getCustGrpLocation(debugId, self->grpId) == LOBBY)
				{/*manager has said that movie is running - ie ticket taker is no longer taking tickets.. but grp is in lobby - so it waits till next movie*/
					changeCustState(debugId, self->selfId, self->grpId, WAIT_IN_LOBBY_AS_TICKET_NOT_TAKEN_BY_TT);
					changeCustState(debugId, self->selfId, self->grpId, LOOKING_FOR_TT);

				}
				YIELD();
				if(getCustGrpLocation(debugId, self->grpId) == LOBBY && mtCb.custGrp[self->grpId].numCust > (MAX_SEATS - getNumOfTicketTaken(debugId)))
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
				for(i=1; i< mtCb.custGrp[self->grpId].numCust; i++)
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
			while(!seatsTaken)/*we arent sat yet*/
			{	/*looking for seats in same row*/
				for(i=0; i<MAX_ROWS; i++)
				{
					if(!seatsTaken)
					for(j=0; j<MAX_COLS; j++)
					{
						if(mtCb.mvRoom.seat[i][j].cust == NULL )
						{/*thers's nobody here on this seat yet*/
							if((j+mtCb.custGrp[self->grpId].numCust ) > MAX_COLS)
							break;

							for(k=j+1; k<(j+mtCb.custGrp[self->grpId].numCust); k++)
							{
								if(mtCb.mvRoom.seat[i][k].cust != NULL )
								break;
								
							}
							if(k==(j+mtCb.custGrp[self->grpId].numCust))
							{/*allocating seats*/
								mtCb.mvRoom.seat[i][j].cust = self; 
								self->seat =  &mtCb.mvRoom.seat[i][j];
								for(k=j+1; k<(j+mtCb.custGrp[self->grpId].numCust); k++)
								{
									mtCb.mvRoom.seat[i][k].cust = (Cust *)&mtCb.custGrp[self->grpId].cust[k-j]; 
									mtCb.custGrp[self->grpId].cust[k-j].seat =  &mtCb.mvRoom.seat[i][k];
									AcquireLock(printLock);/*PRINT*/
									Print("Customer %d in Group %d has found the following seat: row %d and ", (k-j), self->grpId, i);
									Print("seat %d\n", k, -1,-1);
									ReleaseLock(printLock);
								}
								seatsTaken = 1;
							}
							break;
						}
						
					}
				}
				/*looking for seats in consecutive rows*/
				k=0;
				if(!seatsTaken)
				{
					while(k!=mtCb.custGrp[self->grpId].numCust)
					{
						seatTook =0;
						for(i=0; i<MAX_ROWS; i++)
						{
							for(j=0; j<MAX_COLS; j++)
							{
								if(mtCb.mvRoom.seat[i][j].cust == NULL )
								{ /*nobody is sat here - we take the first seat we see empty*/
									mtCb.mvRoom.seat[i][j].cust = (Cust *)&mtCb.custGrp[self->grpId].cust[k];
									mtCb.custGrp[self->grpId].cust[k].seat =  &mtCb.mvRoom.seat[i][j];
									AcquireLock(printLock);/*PRINT*/
									Print("Customer %d in Group %d has found the following seat: row %d and ", k, self->grpId, i);
									Print("seat %d\n", j, -1,-1);
									ReleaseLock(printLock);									
									k++; 
									seatTook = 1; 
									break;
								}
							}
							if(seatTook) break;
						}
					}
				}seatsTaken = 1;
			}
			ReleaseLock(seats);
		}
		if(seatsTaken)
		{/*people have selected seats*/

			changeCustState(debugId, self->selfId, self->grpId, SEATS_READY);
			changeCustGrpState(debugId, self->grpId, SEATS_READY);
		}
		if(getCustState(debugId,  self->selfId, self->grpId) == SEATS_READY)
		{
			if(mtTest6==1)/* specific to test 6*/
			{
				if(self->IAMTicketBuyer)
				{
					for(i=1; i< mtCb.custGrp[self->grpId].numCust; i++)
					{
						while( getCustState(debugId, i, self->grpId) != READY_TO_LEAVE_MOVIE_ROOM )
						{ YIELD(); }
					}
				}
				AcquireLock(printLock);/*PRINT*/
				Print("Customer %d in Group %d is sitting in a theater room seat\n", self->selfId, self->grpId,-1);
				ReleaseLock(printLock);
				changeCustState(debugId, self->selfId, self->grpId, READY_TO_LEAVE_MOVIE_ROOM);
			}
			else
			{

				self->seat->isTaken = 1;
				self->seatTaken = 1;
				if(self->IAMTicketBuyer)
				{
					for(i=1; i< mtCb.custGrp[self->grpId].numCust; i++)
					{/*wait for evryone in group to sit down*/
						while( getCustState(debugId, i, self->grpId) != SEAT_TAKEN )
						{ YIELD(); }
					}
				}
				AcquireLock(printLock);
				Print("Customer %d in Group %d is sitting in a theater room seat\n", self->selfId, self->grpId,-1);
				ReleaseLock(printLock);
				changeCustState(debugId, self->selfId, self->grpId, SEAT_TAKEN);
			}
		}





		/*time to go after movie*/
		if((getCustState(debugId,  self->selfId, self->grpId) == READY_TO_LEAVE_MOVIE_ROOM) )
		{
			if(mtTest6==0) /*test 6 specific*/
			{
				self->seat->isTaken = 0;
				self->seat->cust = NULL;
				self->seatTaken = 0;
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
				AcquireLock(printLock);/*PRINT*/
				Print("HeadCustomer %d of Group %d has told the group to proceed\n", self->selfId, self->grpId, -1 );
				ReleaseLock(printLock);
			}
			AcquireLock(printLock);/*PRINT*/
			Print("Customer %d in Group %d is getting out of a theater room seat\n", self->selfId, self->grpId, -1);
			ReleaseLock(printLock);
			changeCustState(debugId, self->selfId, self->grpId, LEFT_MOVIE_ROOM_AFTER_MOVIE);
		}

		if(getCustState(debugId,  self->selfId, self->grpId) == LEFT_MOVIE_ROOM_AFTER_MOVIE )
		{
			if(self->IAMTicketBuyer)
			{   /*I am grp head - getting evryone out of movie room*/
				changeCustGrpLocation(debugId, self->grpId, OUT_OF_MOVIEROOM);
				for(i=1; i< mtCb.custGrp[self->grpId].numCust; i++)
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
				AcquireLock(printLock);/*PRINT*/
				Print("Customer %d in Group %d is going to the bathroom.\n", self->selfId, self->grpId,-1);
				ReleaseLock(printLock);
				changeCustState(debugId, self->selfId, self->grpId, USING_BATHROOM);

				useDuration = Random();
				useDuration %= 10;
				useDuration += 5;
				/*we assume once a peson goes to bathroom, he returns after doing his business in some Random time between 5 and 15 thread yields*/
				for(i=0; i<useDuration; i++)
				{				
					YIELD();
				}
				AcquireLock(printLock);/*PRINT*/
				Print("Customer %d in Group %d is leaving the bathroom.\n", self->selfId, self->grpId, -1);
				ReleaseLock(printLock);
			}
			changeCustState(debugId, self->selfId, self->grpId, READY_TO_GO_OUT_OF_MT);
		}
		if(getCustState(debugId,  self->selfId, self->grpId) == READY_TO_GO_OUT_OF_MT)
		{ /*ready to go out of theater*/
			AcquireLock(printLock);/*PRINT*/
			Print("Customer %d in Group %d is in the lobby\n", self->selfId, self->grpId,-1);
			ReleaseLock(printLock);
			for(i=0; i< mtCb.custGrp[self->grpId].numCust; i++)
			{  /*checking if people in my group needed tto use bathroom or not*/
				while(
						getCustState(debugId, i, self->grpId) != CUSTOMER_SIMULATION_COMPLETED &&
						getCustState(debugId, i, self->grpId) != WAIT_AS_OVER &&
						getCustState(debugId, i, self->grpId) != READY_TO_GO_OUT_OF_MT
				     )
				{ YIELD(); }/*we wait for everyone to leave theater and simulation completes*/
			}
			AcquireLock(printLock);/*PRINT*/
			Print("Customer %d in Group %d is leaving the lobby\n", self->selfId, self->grpId, -1);
			ReleaseLock(printLock);

			AcquireLock(printLock);/*PRINT*/
			Print("Customer %d in Group %d has left the movie theater.\n", self->selfId, self->grpId,-1);
			ReleaseLock(printLock);
			
			changeCustState(debugId, self->selfId, self->grpId, CUSTOMER_SIMULATION_COMPLETED);/*once everyone has left theater - the simulation is ove*/
			if(self->IAMTicketBuyer)
			{ /*the grp head waits for everyone to leave theater and simulation ending*/
				for(i=1; i< mtCb.custGrp[self->grpId].numCust; i++)
				{
					while(
							getCustState(debugId, i, self->grpId) != WAIT_AS_OVER &&
							getCustState(debugId, i, self->grpId) != CUSTOMER_SIMULATION_COMPLETED
					     )
					{ YIELD(); }
				}
				changeCustGrpLocation(debugId, self->grpId, OUT_OF_MOVIETHEATURE);
			}
			mtTest6Called++;
			AcquireLock(exitLock);/*PRINT*/
			self->state = WAIT_AS_OVER; 
			WaitCV(exitLock, exitCV);/*we now go to wait for lock*/

			/*changeCustState(debugId, self->selfId, self->grpId, WAIT_AS_OVER);*/
			/*Exit(0);
			return;*/
		}	
		if(getCustState(debugId, self->selfId, self->grpId) == WAIT_AS_OVER)
		{
			changeCustState(debugId, self->selfId, self->grpId, WAIT_AS_OVER);
		}
		YIELD();
	}
}

/*
 * Starting Customer Threads
 * */
void startCustThreads(int debugId )
{
	int i=0, j=0, num=0;
	/*Thread *cust;*/
	/*char *threadName;*/

	for(i=0; i<mtCb.numCustGrp; i++)
	{  /*we have i groups and j people in i group*/
		for(j=0; j<mtCb.custGrp[i].numCust; j++)
		{
			/*threadName = (char *)malloc(sizeof(100));*/
			/*sPrint(threadName, "CUST_%d_%d\0",i,j);*/
			/*mtCb.custGrp[i].cust[j].selfThread = new Thread(mtCb.custGrp[i].cust[j].name);*/
			changeCustState(debugId, j, i, STARTED_BY_MAIN);
			num = (10*i)+j;
			/*mtCb.custGrp[i].cust[j].selfThread->Fork((VoidFunctionPtr)custMain, num);*/
			start_fork_thread(mtCb.custGrp[i].cust[j].name, custMain, num);
		}
	}
}

int SeatsTaken(int debugId)/*This function will be used to know how many seats are available at a time*/
{
	int i=0,seatsCount=0;
	int j=0;
	for (i = 0; i < MAX_ROWS; i++)/*rows in movie room*/
	{	
		for (j=0; j<MAX_COLS ; j++)/*cols in movie room*/

		{
			if(mtCb.mvRoom.seat[i][j].cust != NULL && mtCb.mvRoom.seat[i][j].isTaken == 1) 
			{
				Cust *cust = (Cust *) mtCb.mvRoom.seat[i][j].cust;
				seatsCount++; 
			}

		}
	}
	return seatsCount;
}

/*
 * management of movie playing by the Manager
 * */
void movieManage(int debugId)
{
	int i=0;
	for(i=0; i<mtCb.numTT; i++)
	{
		if( mtCb.tt[i].msgToMan == START_MOVIE )
		{/*once movie is begun, manager asks ticket takers to go on break*/
			mtCb.tt[i].msgByMan = GO_ON_BREAK;
			AcquireLock(printLock);/*PRINT*/
			Print("Manager has told TicketTaker %d to go on break.\n", i,-1,-1);
			ReleaseLock(printLock);
			mtCb.man.msgToAll = MOVIE_RUNNING;
			mtCb.tt[i].msgToMan = INVALID_MSG;
		}
	}

	if (mtCb.man.msgToAll == MOVIE_RUNNING)
	{/*manager tells everyone that movie is running*/
		for(i=0; i<mtCb.numTT; i++)
		{
			if( ( getTtState(debugId, i) == FREE_AND_TAKING_TICKET ||
						getTtState(debugId, i) == FREE_AND_TAKING_TICKET_BEING_FIRST) && mtCb.tt[i].currentCust == NULL)
			{
				acquireTtLock(debugId, i);/*aquire its loc*/
				signalToTtLock(debugId, i);/*signal it to wake up*/
				releaseTtLock(debugId, i);/*let go of lock*/
				YIELD();/*manager yields to let others run*/
			}
				while(getTtState(debugId, i) != ON_BREAK && getTtState(debugId, i) != WAIT_AS_OVER)
				{ /*the ticket taker is working but there is no customer at him*/
					YIELD();
				}
		}

		if (getMtState(debugId) == MOVIE_IS_NOT_PLAYING )/*We havent checked here if all 25 have to be in*/
		{	
			while( !IsSeatsOccupied && SeatsTaken(debugId) != 0) /*seats taken by custs should be equal to zero b4 next movie starts*/
			{ 
				YIELD(); /*manager waits for seats to be taken*/
			}
			for (i=0; i<mtCb.numCustGrp; i++)
			{/*checking if everyone has seen movie*/
				if( 
						getCustGrpLocation(debugId, i) != START && 
						getCustGrpLocation(debugId, i) != TICKET_CLERK && 
						getCustGrpLocation(debugId, i) != CONCESSION_CLERK && 
						getCustGrpLocation(debugId, i) != LOBBY && 
						getCustGrpLocation(debugId, i) != MOVIEROOM 
				  );
				else  break;
			}
			if( i == mtCb.numCustGrp)
			{
				changeMtState(debugId, NO_MOVIE_REQD);
				IsSeatsOccupied = 1;
				return;
			}

			if(!IsSeatsOccupied)
			{/*seats are taken so ticket takers list of tickets taken is to be completed*/
				resetNumOfTicketTaken(debugId);
				mtCb.man.msgToAll = MOVIE_NOT_RUNNING;
			/*sending msg to the Ticket Taker to take tickets and fill seats	*/
				for(i=0; i<mtCb.numTT; i++)
				{/*we set ticket taker to initial values*/
					acquireTtLock(debugId, i);
					mtCb.tt[i].msgByMan = FILL_CUST;
					mtCb.tt[i].msgToMan = INVALID_MSG;
					mtCb.tt[i].currentCust = NULL;
					signalToTtLock(debugId, i);

					releaseTtLock(debugId, i);
				}
			}
			IsSeatsOccupied = 1;
			if(mtCb.man.msgToAll == MOVIE_NOT_RUNNING)
			{  /*if movie is not running, tickets can be given to ticket taker*/
				for (i=0; i<mtCb.numCustGrp; i++)
				{
					if( getCustGrpLocation(debugId, i) == LOBBY && getCustState(debugId, 0, i) == WAIT_IN_LOBBY_AS_TICKET_NOT_TAKEN_BY_TT)
					{
						changeCustState(debugId, 0, i, NEW_MOVIE_STARTING_TICKET_MAY_BE_TAKEN);
					}
				}
			}
		}
	}
	/*condition that TT said that everyone has been told to get into seats*/
	/*We havent checked here if all 25 have to be in*/
	if (mtCb.man.msgToAll == MOVIE_RUNNING)
	{
		int sTaken = SeatsTaken(debugId);
		int numTcktTaken = getNumOfTicketTaken(debugId);
		if(sTaken == numTcktTaken && sTaken != 0 && (getMtState(debugId) == MOVIE_IS_NOT_PLAYING || getMtState(debugId) == STARTED_BY_MAN))
		{ /*we see if seats have been taken*/
			/*Ask the Technician to start movie;*/
			mtCb.mt.msgByMan = START_MOVIE;
			AcquireLock(printLock);/*PRINT*/
			Print("Manager is telling the MovieTechnician to start the movie\n",-1,-1,-1);
			ReleaseLock(printLock);
			changeMtState(debugId, MOVIE_IS_PLAYING);	
			IsSeatsOccupied = 0;
		}

	}
}

/*
 * Manager Main's Program
 * */
void manMain()
/*void manMain(int achintya)*/
{
	int a = 0;
	int debugId = 2;
	int ret_val=0;
	int i=0, j=0;
	Manager *self = &mtCb.man;
	
	end_fork_thread(&a);

	/*DEBUG_INIT("manager.log", &debugId);*/
	IsSeatsOccupied = 0;

	self = &mtCb.man;
	/*Thread *tcThread, *ccThread, *ttThread, *mtThread;*/
	/*char *threadName;*/

	mtCb.man.msgToAll = MOVIE_NOT_RUNNING; /*we intialize states of all workers to be MOVIE_NOT_RUNNING*/

	for(i=0; i<mtCb.numTC; i++)
	{/*the manager will start threads of the ticket clerks*/
		mtCb.tc[i].queueType = TC_QUEUE; 
		mtCb.tc[i].queue = &mtCb.queue[TC_QUEUE][i];
		mtCb.tc[i].location = TICKET_CLERK;
		if(mtTest2 == 0)
		{
			changeTcState(debugId, i, STARTED_BY_MAN);
		}
		start_fork_thread(mtCb.tc[i].name, tcMain, i);
		/*mtCb.tc[i].selfThread->Fork(tcMain, i); //the ticket clerk is now beginning to work*/
		if(mtTest4 == 1) break; /*test 4 specific*/
	}
	if(mtTest4 == 0)  /*if test 4  not running as these are not required in test 4.*/
	{
		for(i=0; i<mtCb.numCC; i++)
		{	/*the manager will now start concession clerks*/
			mtCb.cc[i].queueType = CC_QUEUE; 
			mtCb.cc[i].queue = &mtCb.queue[CC_QUEUE][i];
			mtCb.cc[i].location = CONCESSION_CLERK;
			if(mtTest2 == 0)
			{
				changeCcState(debugId, i, STARTED_BY_MAN);
			}
			start_fork_thread(mtCb.cc[i].name, ccMain, i);
		}
		if(mtTest2 == 0) /*if test 2  not running as these are not required in test 2.*/
		{
			for(i=0; i<mtCb.numTT; i++)
			{/*the manager will now start ticket takers*/
				mtCb.tt[i].location = MOVIEROOM;
				changeTtState(debugId, i, STARTED_BY_MAN);
				start_fork_thread(mtCb.tt[i].name, ttMain, i);
			}

			/*setting up the movie technician*/
			mtCb.mt.location = MOVIEROOM;
			changeMtState(debugId, STARTED_BY_MAN);
			start_fork_thread(mtCb.mt.name, mtMain, 0);
		}
	}
	while(1)
	{
		if(mtTest2 == 0)  /*if test 2 not running these are not required in test 2.*/
		{
			queueTcManage(debugId);
			if(mtTest4 == 0)  /*if test 4 not running these are not required in test 4.*/
			{
				queueCcManage(debugId);
				queueTtManage(debugId);
			}
		}
		if(mtTest4 == 0)  /*if test 4 not running these are not required in test 4.*/
		{
			moneyTcManage(debugId);
			moneyCcManage(debugId);
			AcquireLock(printLock);/*PRINT*/
			Print("Total money made by office = %d\n", mtCb.man.money,-1,-1);
			ReleaseLock(printLock);
			if(mtTest2 == 0)  /*if test 2 not running these are not required in test 2.*/

			{
				movieManage(debugId);	

				for (i=0; i<mtCb.numCustGrp; i++)
				{
					if(getCustGrpLocation(debugId, i) == OUT_OF_MOVIETHEATURE);
					/*if(getCustGrpLocation(debugId, i) == LOBBY);*/
					else break;
				}
				if( i == mtCb.numCustGrp )
				{
					for(i=0; i<mtCb.numTC; i++)
					{
						AcquireLock(exitLock);
						while(
								getTcState(debugId, i) != WAIT_AS_OVER &&
								getTcState(debugId, i) != ON_BREAK
						     )
						{ 
							ReleaseLock(exitLock);
							YIELD(); 
						}
						ReleaseLock(exitLock);
					}
					for(i=0; i<mtCb.numCC; i++)
					{
						AcquireLock(exitLock);
						while(
								getCcState(debugId, i) != WAIT_AS_OVER &&
								getCcState(debugId, i) != ON_BREAK
						     )
						{ 
							ReleaseLock(exitLock);
							YIELD(); 
						}
						ReleaseLock(exitLock);
					}
					for(i=0; i<mtCb.numTT; i++)
					{
						AcquireLock(exitLock);
						while(
								getTtState(debugId, i) != WAIT_AS_OVER &&
								getTtState(debugId, i) != ON_BREAK
						     )
						{ 
							ReleaseLock(exitLock);
							YIELD(); 
						}
						ReleaseLock(exitLock);
					}
					for(i=0; i<mtCb.numCustGrp; i++)
						for(j=0; j<mtCb.custGrp[i].numCust; j++)
						{
							AcquireLock(exitLock);
							while(mtCb.custGrp[i].cust[j].state != WAIT_AS_OVER)
							{ 
								ReleaseLock(exitLock);
								YIELD(); 
							}
							/*while(getCustState(debugId,j, i) != WAIT_AS_OVER)
							  { 
							  YIELD(); 
							  }*/
							ReleaseLock(exitLock);

						}
					Exit(0);
					/*for(i=0; i<mtCb.numTC; i++)
					  {
					  SignalCV(tcLock[i],tcCondVar[i]);
					  }
					  for(i=0; i<mtCb.numCC; i++)
					  {
					  SignalCV(ccLock[i],ccCondVar[i]);
					  }
					  for(i=0; i<mtCb.numTT; i++)
					  {
					  SignalCV(ttLock[i],ttCondVar[i]);
					  }
					  SignalCV(mtLock,mtCondVar);
					Halt();*/

				}
			}

			if(mtTest2 == 1)  /*/if test 2 not running these are not required in test 2.*/
			{
				if(mtTest2Called >= 5)
				{
					for(i=0; i<mtCb.numTC; i++)
					{
						AcquireLock(exitLock);
						while(getTcState(debugId, i) != WAIT_AS_OVER)
						{ 
							ReleaseLock(exitLock);
							YIELD(); 
						}
						ReleaseLock(exitLock);
					}
					for(i=0; i<mtCb.numCC; i++)
					{
						AcquireLock(exitLock);
						while(getCcState(debugId, i) != WAIT_AS_OVER)
						{ 
							ReleaseLock(exitLock);
							YIELD(); 
						}
						ReleaseLock(exitLock);
					}
					for(i=0; i<mtCb.numCustGrp; i++)
					for(j=0; j<mtCb.custGrp[i].numCust; j++)
					{
						while(getCustState(debugId,j, i) != WAIT_AS_OVER)
						{ 
							YIELD(); 
						}

					}
					Exit(0);
				}
			}
		}
		if(mtTest4 == 1)
		{
			if(mtTest4Called >= 7)
			{
				AcquireLock(exitLock);
				while(getTcState(debugId, 0) != WAIT_AS_OVER)
				{ 
					ReleaseLock(exitLock);
					YIELD(); 
				}
				ReleaseLock(exitLock);
				for(i=0; i<mtCb.numCustGrp; i++)
					for(j=0; j<mtCb.custGrp[i].numCust; j++)
					{
						while(getCustState(debugId,j, i) != WAIT_AS_OVER)
						{ 
							YIELD(); 
						}
					}
				Exit(0);
			}
		}
		YIELD();
		YIELD();
		YIELD();
	}	
}

/*
 * Manager Thread is started in it.
 * */
void startManThread(int debugId )
{
	start_fork_thread(mtCb.man.name, manMain, 0);
}

/*
 * This is the first function that is called 
 * when the customised simulation is run.
 * */
void movieTheatureMain()
{
	/*Setting up debugging information*/
	char dbg[10], logFileName[100];
	int i=0,ret_val=0;

	int debugId=1;

	initializeAllValues(debugId); /*intializing entities that shall be part of simulation*/
	initializeAllQueues(debugId);/*initializing all different queues that need to be formed*/
	initializeAllLocks(debugId);/*set up required locks*/
	startManThread(debugId);/*start the manager*/
	startCustThreads(debugId);/*bring in the customers*/

	/*DEBUG_CLOSE(debugId);*/
}

/*
 * This is to initailize all values
 * required for the simulation with 
 * 40 customers, 5 Ticket Clerks, 5
 * Concession Clerks, 3 Ticket Clerks and
 * other employees. This is a complete simulation 
 * Initialization
 * */
void initializeAllValues_Test7(int debugId)
{
	int i=0, j=0;

	mtCb.numTC = 5;
	for(i=0; i<mtCb.numTC; i++)
	{
		copy_string(mtCb.queue[TC_QUEUE][i].name, 20,"QQ", 2, 0, i);
		copy_string(mtCb.tc[i].name, 20,"TC", 2, -1, i);
	}

	mtCb.numCC = 5;
	for(i=0; i<mtCb.numCC; i++)
	{
		copy_string(mtCb.queue[CC_QUEUE][i].name, 20,"QQ", 2, 1, i);
		copy_string(mtCb.cc[i].name, 20,"CC", 2, -1, i);
	}

	mtCb.numTT = 3;
	for(i=0; i<mtCb.numTT; i++)
	{
		copy_string(mtCb.queue[TT_QUEUE][i].name, 20,"QQ", 2, 2, i);
		copy_string(mtCb.tt[i].name, 20,"TT", 2, -1, i);
	}

	copy_string(mtCb.man.name, 20,"MA", 2, -1, 1);
	copy_string(mtCb.mt.name, 20,"MT", 2, -1, 1);
	
	mtCb.numCust = 40;

	mtCb.numCustGrp=0;	
	i=mtCb.numCust;
	while(i > 0)
	{
		int randomNum = Random(); 
		randomNum = (randomNum % 5) + 1;
		if(i <= 5)
		{
			randomNum = i;
		}
		i = i- randomNum;
				
		mtCb.custGrp[mtCb.numCustGrp].grpId = mtCb.numCustGrp;
		mtCb.custGrp[mtCb.numCustGrp].location = START;
		mtCb.custGrp[mtCb.numCustGrp].numCust = randomNum; 
		copy_string(mtCb.custGrp[mtCb.numCustGrp].name, 20,"CG", 2, -1, mtCb.numCustGrp);
		for(j=0; j<mtCb.custGrp[mtCb.numCustGrp].numCust; j++)
		{
			mtCb.custGrp[mtCb.numCustGrp].cust[j].selfId = j;
			mtCb.custGrp[mtCb.numCustGrp].cust[j].grpId = mtCb.numCustGrp;
			mtCb.custGrp[mtCb.numCustGrp].cust[j].money = CUST_MONEY;
			mtCb.custGrp[mtCb.numCustGrp].cust[j].IAMTicketBuyer = isCustTicketBuyer(debugId, mtCb.numCustGrp,j);
			copy_string(mtCb.custGrp[mtCb.numCustGrp].cust[j].name, 20,"CU", 2, mtCb.numCustGrp, j);
		}
		mtCb.numCustGrp++;

	}


	AcquireLock(printLock);/*PRINT*/
	Print("Number Of Customers = %d \n", mtCb.numCust, -1, -1);
	Print("Number Of Groups = %d \n", mtCb.numCustGrp, -1, -1);
	Print("Number Of TicketClerks = %d \n", mtCb.numTC, -1, -1);
	Print("Number Of ConcessionClerks = %d \n", mtCb.numCC, -1, -1);
	Print("Number Of TicketTakers = %d \n", mtCb.numTT, -1, -1);
	ReleaseLock(printLock);

}

/*
 * Here is the first function to be called 
 * for the complete simulation with 40 customers.
 * This option is given the test suite.
void movieTheatureMain_Test7()
 * */
void main()
{

	/*char *dbg = NULL, logFileName[100];*/
	char dbg[10], logFileName[100];
	int i=0,ret_val=0;

	int debugId=1;
	/*DEBUG_INIT("main.log", &debugId);*/

	initializeForkMec();

	initializeAllValues_Test7(debugId);

	initializeAllQueues(debugId);

	initializeAllLocks(debugId);

	startManThread(debugId);
	startCustThreads(debugId);
	Exit(0);
	/*DEBUG_CLOSE(debugId);*/
}

/*
 * This is to initialize 
 * 6 customers simulation and 
 * one of each employee type.
 * */
void initializeAllValues_Test8(int debugId)
{
	int i=0, j=0;
	int randomNum;

	mtCb.numTC = 1;
	for(i=0; i<mtCb.numTC; i++)
	{
		copy_string(mtCb.queue[TC_QUEUE][i].name, 20,"QQ", 2, 0, i);
		copy_string(mtCb.tc[i].name, 20,"TC", 2, -1, i);
/*		mtCb.tc[i] = (TC *)malloc(sizeof(TC));*/
	}

	mtCb.numCC = 1;
	for(i=0; i<mtCb.numCC; i++)
	{
		copy_string(mtCb.queue[CC_QUEUE][i].name, 20,"QQ", 2, 1, i);
		copy_string(mtCb.cc[i].name, 20,"CC", 2, -1, i);
/*		mtCb.cc[i] = (CC *)malloc(sizeof(CC));*/
	}

	mtCb.numTT = 1;
	for(i=0; i<mtCb.numTT; i++)
	{
		copy_string(mtCb.queue[TT_QUEUE][i].name, 20,"QQ", 2, 2, i);
		copy_string(mtCb.tt[i].name, 20,"TT", 2, -1, i);
/*		mtCb.tt[i] = (TT *)malloc(sizeof(TT));*/
	}

	copy_string(mtCb.man.name, 20,"MA", 2, -1, 1);
	copy_string(mtCb.mt.name, 20,"MT", 2, -1, 1);
/*	mtCb.mvRoom = (MvRoom *)malloc(sizeof(MvRoom));*/
/*	mtCb.mt = (MT *)malloc(sizeof(MT));*/
	
	mtCb.numCust = 6;

	mtCb.numCustGrp=0;	
	i=mtCb.numCust;
	while(i > 0)
	{
		randomNum = Random(); 

		randomNum = (randomNum % 5) + 1;
		if(i <= 5)
		{
			randomNum = i;
		}
		i = i- randomNum;
				
/*		mtCb.custGrp[mtCb.numCustGrp] = (CustGrp *)malloc(sizeof(CustGrp));*/
		mtCb.custGrp[mtCb.numCustGrp].grpId = mtCb.numCustGrp;
		mtCb.custGrp[mtCb.numCustGrp].location = START;
		mtCb.custGrp[mtCb.numCustGrp].numCust = randomNum; 
		copy_string(mtCb.custGrp[mtCb.numCustGrp].name, 20,"CG", 2, -1, mtCb.numCustGrp);
		for(j=0; j<mtCb.custGrp[mtCb.numCustGrp].numCust; j++)
		{
/*			mtCb.custGrp[mtCb.numCustGrp].cust[j] = (Cust *)malloc(sizeof(Cust));*/
			mtCb.custGrp[mtCb.numCustGrp].cust[j].selfId = j;
			mtCb.custGrp[mtCb.numCustGrp].cust[j].grpId = mtCb.numCustGrp;
			mtCb.custGrp[mtCb.numCustGrp].cust[j].money = CUST_MONEY;
			mtCb.custGrp[mtCb.numCustGrp].cust[j].IAMTicketBuyer = isCustTicketBuyer(debugId, mtCb.numCustGrp,j);
			copy_string(mtCb.custGrp[mtCb.numCustGrp].cust[j].name, 20,"CU", 2, mtCb.numCustGrp, j);
		}
		mtCb.numCustGrp++;

	}

	AcquireLock(printLock);/*PRINT*/
	Print("Number Of Customers = %d \n", mtCb.numCust, -1, -1);
	Print("Number Of Groups = %d \n", mtCb.numCustGrp, -1, -1);
	Print("Number Of TicketClerks = %d \n", mtCb.numTC, -1, -1);
	Print("Number Of ConcessionClerks = %d \n", mtCb.numCC, -1, -1);
	Print("Number Of TicketTakers = %d \n", mtCb.numTT, -1, -1);
	ReleaseLock(printLock);

}

/*
 * This is a simulation start-up
 * for 6 customers option. Option 
 * is given in our test suite
 * */
void movieTheatureMain_Test8()
{

	/*char *dbg = NULL, logFileName[100];*/
	char dbg[10], logFileName[100];
	int i=0,ret_val=0;

	int debugId=1;
	/*DEBUG_INIT("main.log", &debugId);*/
	initializeForkMec();

	initializeAllValues_Test8(debugId);

	initializeAllQueues(debugId);

	initializeAllLocks(debugId);

	startManThread(debugId);
	startCustThreads(debugId);
	/*Exit(0);*/
	/*DEBUG_CLOSE(debugId);*/
}

/*
 * This is the initialization step
 * to for test case 1. Number of 
 * customers groups initailzed are 10 
 * with 1 customer in each group. 
 * */
void initializeAllValues_Test1(int debugId)
{
	int i=0, j=0;

	mtCb.numTC = 3;
	mtCb.numCC = 0;
	mtCb.numTT = 0;
	for(i=0; i<mtCb.numTC; i++)
	{
/*		mtCb.tc[i] = (TC *)malloc(sizeof(TC));*/
		copy_string(mtCb.queue[TC_QUEUE][i].name, 20,"QQ", 2, 0, i);
		copy_string(mtCb.tc[i].name, 20,"TC", 2, -1, i);
		mtCb.tc[i].state = FREE_AND_SELLING_FOOD_BEING_FIRST;
	}

	mtCb.numCust = 10;

	mtCb.numCustGrp=0;	
	i=mtCb.numCust;
	while(i > 0)
	{
		i = i- 1;

/*		mtCb.custGrp[mtCb.numCustGrp] = (CustGrp *)malloc(sizeof(CustGrp));*/
		mtCb.custGrp[mtCb.numCustGrp].grpId = mtCb.numCustGrp;
		mtCb.custGrp[mtCb.numCustGrp].location = START;
		mtCb.custGrp[mtCb.numCustGrp].numCust = 1; 
		copy_string(mtCb.custGrp[mtCb.numCustGrp].name, 20,"CG", 2, -1, mtCb.numCustGrp);
/*		mtCb.custGrp[mtCb.numCustGrp].cust[0] = (Cust *)malloc(sizeof(Cust));*/
		mtCb.custGrp[mtCb.numCustGrp].cust[0].selfId = j;
		mtCb.custGrp[mtCb.numCustGrp].cust[0].grpId = mtCb.numCustGrp;
		mtCb.custGrp[mtCb.numCustGrp].cust[0].money = CUST_MONEY;
		mtCb.custGrp[mtCb.numCustGrp].cust[0].IAMTicketBuyer = isCustTicketBuyer(debugId, mtCb.numCustGrp,j);
		copy_string(mtCb.custGrp[mtCb.numCustGrp].cust[0].name, 20,"CU", 2, mtCb.numCustGrp, 0);
		mtCb.numCustGrp++;

	}

	AcquireLock(printLock);/*PRINT*/
	Print("Number Of Customers = 10 \n",-1,-1,-1);
	Print("Number Of Groups = 10 \n",-1,-1,-1);
	Print("Number Of TicketClerks = 3 \n",-1,-1,-1);
	Print("Number Of ConcessionClerks = 0 \n",-1,-1,-1);
	Print("Number Of TicketTakers = 0 \n",-1,-1,-1);
	ReleaseLock(printLock);
}



/*
 * This initializes queues 
 * pertaining to test case 1
 * */
void initializeAllQueues_Test1(int debugId )
{

	int i=0;
	/*char *queueName;*/
	for(i=0; i<mtCb.numTC; i++)
	{
/*		mtCb.queue[TC_QUEUE][i] = (mtQueue *)malloc(sizeof(mtQueue));*/
		initializeQueue(debugId, &mtCb.queue[TC_QUEUE][i]);
		mtCb.queue[TC_QUEUE][i].queueId = i;
		mtCb.queue[TC_QUEUE][i].queueType = TC_QUEUE;
		/*queueName = (char *)malloc(sizeof(100));*/
		/*sPrint(queueName, "QUEUE_%d_%d\0",TC_QUEUE,i);*/
		queueLock[TC_QUEUE][i] = CreateLock(mtCb.tc[i].name);
		queueCondVar[TC_QUEUE][i] = CreateCV(mtCb.tc[i].name);
	}
	/*queueName = (char *)malloc(sizeof(100));*/
	/*sPrint(queueName, "QUEUE_TC\0");*/
	queueTcLock = CreateLock(mtCb.tc[0].name);
}

/*
 * This adds address to queue
 * pertaining to test case one
 * */
void addAddressToQueue_Test1(int debugId)
{
	int i=0;
	for(i=0; i<mtCb.numTC; i++)
	{
		mtCb.tc[i].queue = &mtCb.queue[TC_QUEUE][i];
		mtCb.tc[i].queue->queueType = TC_QUEUE;
		mtCb.tc[i].queue->queueAddress = (Employee)&mtCb.tc[i];
	}
}

/*
 * This adds address to queue
 * pertaining to test case 2
 * */
void addAddressToQueue_Test2(int debugId)
{
	int i=0;
	for(i=0; i<mtCb.numTC; i++)
	{
		mtCb.tc[i].queue = &mtCb.queue[TC_QUEUE][i];
		mtCb.tc[i].queue->queueType = TC_QUEUE;
		mtCb.tc[i].queue->queueAddress = (Employee)&mtCb.tc[i];
	}
	for(i=0; i<mtCb.numCC; i++)
	{
		mtCb.cc[i].queue = &mtCb.queue[CC_QUEUE][i];
		mtCb.cc[i].queue->queueType = CC_QUEUE;
		mtCb.cc[i].queue->queueAddress = (Employee)&mtCb.cc[i];
	}

}

/*
 * This is to initialze locks
 * pertaining to test case 1.
 *
 * */
void initializeAllLocks_Test1(int debugId)
{
	int i=0, j=0; /*char *lockName;*/

	for(i=0; i<mtCb.numCustGrp; i++)
	{
		for(j=0; j<mtCb.custGrp[i].numCust; j++)
		{
			/*lockName = (char *)malloc(sizeof(100));*/
			/*sPrint(lockName, "CUST_%d_%d\0",i,j);*/
			custLock[i][j]=CreateLock(mtCb.custGrp[i].cust[j].name);
			custCondVar[i][j]=CreateCV(mtCb.custGrp[i].cust[j].name);
		}
	}

	for(i=0; i<mtCb.numTC; i++)
	{
		/*lockName = (char *)malloc(sizeof(100));*/
		/*sPrint(lockName, "TC_%d\0",i);*/
		tcLock[i]=CreateLock(mtCb.tc[i].name);
		tcCondVar[i]=CreateCV(mtCb.tc[i].name);
	}
}


/*
 * This is the main function 
 * pertaining to test case 1 which
 * proves Customers always take 
 * the shortest line, but no 2 
 * customers ever choose the same 
 * shortest line at the same time 
 * */
void movieTheatureMain_Test1()
{
	/*char *dbg = NULL, logFileName[100];*/
	char dbg[10], logFileName[100];
	int i=0,ret_val=0;

	int debugId=1;
	/*DEBUG_INIT("main.log", &debugId);*/
	mtTest1 = 1;
	initializeAllValues_Test1(debugId);
	initializeAllQueues_Test1(debugId);
	initializeAllLocks_Test1(debugId);
	addAddressToQueue_Test1(debugId);
	startCustThreads(debugId);
	
	for(i=0; i< mtCb.numCustGrp; i++)
	{
		/*while(mtCb.custGrp[i].cust[0].state != WAIT)*/
		while(getCustState(debugId, 0, i) != WAIT)
		{ 
			YIELD(); 
		}
	}
	Exit(0);

	/*DEBUG_CLOSE(debugId);*/
}

/*
 * This is to initialize values
 * for test case 3 in it we have 
 * taken 5 customers and 5 groups.
 * and 1 ticket clerks.
 * */
void initializeAllValues_Test3(int debugId)
{
	int i=0, j=0;

	mtCb.numTC = 1;
	mtCb.numCC = 0;
	mtCb.numTT = 0;
	for(i=0; i<mtCb.numTC; i++)
	{
/*		mtCb.tc[i] = (TC *)malloc(sizeof(TC));*/
		copy_string(mtCb.queue[TC_QUEUE][i].name, 20,"QQ", 2, 0, i);
		copy_string(mtCb.tc[i].name, 20,"TC", 2, -1, i);
		mtCb.tc[i].state = FREE_AND_SELLING_TICKET_BEING_FIRST;
	}

	mtCb.numCust = 5;

	mtCb.numCustGrp=0;	
	i=mtCb.numCust;
	while(i > 0)
	{
		i = i- 1;

/*		mtCb.custGrp[mtCb.numCustGrp] = (CustGrp *)malloc(sizeof(CustGrp));*/
		mtCb.custGrp[mtCb.numCustGrp].grpId = mtCb.numCustGrp;
		mtCb.custGrp[mtCb.numCustGrp].location = START;
		mtCb.custGrp[mtCb.numCustGrp].numCust = 1; 
		copy_string(mtCb.custGrp[mtCb.numCustGrp].name, 20,"CG", 2, -1, mtCb.numCustGrp);
/*		mtCb.custGrp[mtCb.numCustGrp].cust[0] = (Cust *)malloc(sizeof(Cust));*/
		mtCb.custGrp[mtCb.numCustGrp].cust[0].selfId = j;
		mtCb.custGrp[mtCb.numCustGrp].cust[0].grpId = mtCb.numCustGrp;
		mtCb.custGrp[mtCb.numCustGrp].cust[0].money = CUST_MONEY;
		mtCb.custGrp[mtCb.numCustGrp].cust[0].IAMTicketBuyer = isCustTicketBuyer(debugId, mtCb.numCustGrp,j);
		copy_string(mtCb.custGrp[mtCb.numCustGrp].cust[0].name, 20,"CU", 2, mtCb.numCustGrp, 0);
		mtCb.numCustGrp++;

	}

	AcquireLock(printLock);/*PRINT*/
	Print("Number Of Customers = 5 \n",-1,-1,-1);
	Print("Number Of Groups = 5 \n",-1,-1,-1);
	Print("Number Of TicketClerks = 1 \n",-1,-1,-1);
	Print("Number Of ConcessionClerks = 0 \n",-1,-1,-1);
	Print("Number Of TicketTakers = 0 \n",-1,-1,-1);
	ReleaseLock(printLock);

}

/*
 * This is to initialize queues
 * pertaining to test case 3.
 *
 * */
void initializeAllQueues_Test3(int debugId )
{

	int i=0;
	/*char *queueName;*/
	for(i=0; i<mtCb.numTC; i++)
	{
/*		mtCb.queue[TC_QUEUE][i] = (mtQueue *)malloc(sizeof(mtQueue));*/
		initializeQueue(debugId, &mtCb.queue[TC_QUEUE][i]);
		mtCb.queue[TC_QUEUE][i].queueId = i;
		mtCb.queue[TC_QUEUE][i].queueType = TC_QUEUE;
		/*queueName = (char *)malloc(sizeof(100));*/
		/*sPrint(queueName, "QUEUE_%d_%d\0",TC_QUEUE,i);*/
		queueLock[TC_QUEUE][i] = CreateLock(mtCb.queue[TC_QUEUE][i].name);
		queueCondVar[TC_QUEUE][i] = CreateCV(mtCb.queue[TC_QUEUE][i].name);
	}
}

/*
 * This is to initialize all
 * locks pertaining to test case
 * 3.
 * */
void initializeAllLocks_Test3(int debugId)
{
	int i=0, j=0; /*char *lockName;*/

	for(i=0; i<mtCb.numCustGrp; i++)
	{
		for(j=0; j<mtCb.custGrp[i].numCust; j++)
		{
			/*lockName = (char *)malloc(sizeof(100));*/
			/*sPrint(lockName, "CUST_%d_%d\0",i,j);*/
			custLock[i][j]=CreateLock(mtCb.custGrp[i].cust[j].name);
			custCondVar[i][j]=CreateCV(mtCb.custGrp[i].cust[j].name);
		}
	}

	for(i=0; i<mtCb.numTC; i++)
	{
		/*lockName = (char *)malloc(sizeof(100));*/
		/*sPrint(lockName, "TC_%d\0",i);*/
		tcLock[i]=CreateLock(mtCb.tc[i].name);
		tcCondVar[i]=CreateCV(mtCb.tc[i].name);
	}

}

/*
 * This is the first step 
 * pertaining to test case 3
 * which proves Customers do not
 *  leave a Clerk, or TicketTaker, 
 *  until they are told to do so. Clerks 
 *  and TicketTakers do not start 
 *  with another Customer until they 
 *  know the current Customer has left. 
 *  customer until they know that the 
 *  last Customer has left their area
 * */
void movieTheatureMain_Test3()
{
	char dbg[10], logFileName[100];
	int i=0,j=0,ret_val=0;

	int debugId=1;
	/*DEBUG_INIT("main.log", &debugId);*/

	/*char *threadName;*/
	initializeAllValues_Test3(debugId);
	initializeAllQueues_Test3(debugId);
	initializeAllLocks_Test3(debugId);
	mtTest3=1;
	startCustThreads(debugId);

	for(i=0; i<mtCb.numTC; i++)
	{
		/*threadName = (char *)malloc(sizeof(100));*/
		/*sPrint(threadName, "TC_%d\0",i);*/
		/*mtCb.tc[i].selfThread = new Thread(mtCb.tc[i].name);*/
		mtCb.tc[i].queueType = TC_QUEUE; 
		mtCb.tc[i].queue = &mtCb.queue[TC_QUEUE][i];
		mtCb.tc[i].location = TICKET_CLERK;
		changeTcState(debugId, i, STARTED_BY_MAN);
		start_fork_thread(mtCb.tc[i].name, tcMain, i);
		/*mtCb.tc[i].selfThread->Fork(tcMain, i); */
	}

	while(mtTest3Called != 5)
	{ 
		YIELD(); 
	}
	for(i=0; i<mtCb.numTC; i++)
	{
		AcquireLock(exitLock);
		while(getTcState(debugId, i) != WAIT_AS_OVER)
		{ 
			ReleaseLock(exitLock);
			YIELD(); 
		}
		ReleaseLock(exitLock);
	}
	for(i=0; i<mtCb.numCustGrp; i++)
		for(j=0; j<mtCb.custGrp[i].numCust; j++)
		{
			while(getCustState(debugId,j, i) != WAIT_AS_OVER)
			{ 
				YIELD(); 
			}

		}
	Exit(0);

	/*DEBUG_CLOSE(debugId);*/
}

/*
 * This is the intialization
 * step pertaining to test case 2
 * we have taken 2 ticket clerks.
 * 1 concession clerk and 5 customer
 * groups with 1 customer in each group.
 * */
void initializeAllValues_Test2(int debugId)
{
	int i=0, j=0;

	mtCb.numTC = 2;
	mtCb.numCC = 1;
	mtCb.numTT = 0;
	for(i=0; i<mtCb.numTC; i++)
	{
/*		mtCb.tc[i] = (TC *)malloc(sizeof(TC));*/
		copy_string(mtCb.queue[TC_QUEUE][i].name, 20,"QQ", 2, 0, i);
		copy_string(mtCb.tc[i].name, 20,"TC", 2, -1, i);
		mtCb.tc[i].state = FREE_AND_SELLING_TICKET_BEING_FIRST;
	}
	for(i=0; i<mtCb.numCC; i++)
	{
/*		mtCb.cc[i] = (CC *)malloc(sizeof(CC));*/
		copy_string(mtCb.queue[CC_QUEUE][i].name, 20,"QQ", 2, 1, i);
		copy_string(mtCb.cc[i].name, 20,"CC", 2, -1, i);
		mtCb.cc[i].state = FREE_AND_SELLING_FOOD_BEING_FIRST;
	}


	mtCb.numCust = 5;

	mtCb.numCustGrp=0;	
	i=mtCb.numCust;
	while(i > 0)
	{
		i = i- 1;

/*		mtCb.custGrp[mtCb.numCustGrp] = (CustGrp *)malloc(sizeof(CustGrp));*/
		mtCb.custGrp[mtCb.numCustGrp].grpId = mtCb.numCustGrp;
		mtCb.custGrp[mtCb.numCustGrp].location = START;
		mtCb.custGrp[mtCb.numCustGrp].numCust = 1; 
		copy_string(mtCb.custGrp[mtCb.numCustGrp].name, 20,"CG", 2, -1, mtCb.numCustGrp);
/*		mtCb.custGrp[mtCb.numCustGrp].cust[0] = (Cust *)malloc(sizeof(Cust));*/
		mtCb.custGrp[mtCb.numCustGrp].cust[0].selfId = j;
		mtCb.custGrp[mtCb.numCustGrp].cust[0].grpId = mtCb.numCustGrp;
		mtCb.custGrp[mtCb.numCustGrp].cust[0].money = CUST_MONEY;
		mtCb.custGrp[mtCb.numCustGrp].cust[0].IAMTicketBuyer = isCustTicketBuyer(debugId, mtCb.numCustGrp,j);
		copy_string(mtCb.custGrp[mtCb.numCustGrp].cust[0].name, 20,"CU", 2, mtCb.numCustGrp, 0);
		mtCb.numCustGrp++;

	}

	AcquireLock(printLock);/*PRINT*/
	Print("Number Of Customers = 5 \n",-1,-1,-1);
	Print("Number Of Groups = 5 \n",-1,-1,-1);
	Print("Number Of TicketClerks = 2 \n",-1,-1,-1);
	Print("Number Of ConcessionClerks = 1 \n",-1,-1,-1);
	Print("Number Of TicketTakers = 0 \n",-1,-1,-1);
	ReleaseLock(printLock);
}

/*
 * This is to initialze all
 * queues pertaining to 
 * test case 2.
 *
 * */
void initializeAllQueues_Test2(int debugId )
{

	int i=0;
	/*char *queueName;*/
	for(i=0; i<mtCb.numTC; i++)
	{
/*		mtCb.queue[TC_QUEUE][i] = (mtQueue *)malloc(sizeof(mtQueue));*/
		initializeQueue(debugId, &mtCb.queue[TC_QUEUE][i]);
		mtCb.queue[TC_QUEUE][i].queueId = i;
		mtCb.queue[TC_QUEUE][i].queueType = TC_QUEUE;
		/*queueName = (char *)malloc(sizeof(100));*/
		/*sPrint(queueName, "QUEUE_%d_%d\0",TC_QUEUE,i);*/
		queueLock[TC_QUEUE][i] = CreateLock(mtCb.tc[i].name);
		queueCondVar[TC_QUEUE][i] = CreateCV(mtCb.tc[i].name);
	}
	/*queueName = (char *)malloc(sizeof(100));*/
	/*sPrint(queueName, "QUEUE_TC\0");*/
	queueTcLock = CreateLock(mtCb.tc[0].name);
	for(i=0; i<mtCb.numCC; i++)
	{
/*		mtCb.queue[CC_QUEUE][i] = (mtQueue *)malloc(sizeof(mtQueue));*/
		initializeQueue(debugId, &mtCb.queue[CC_QUEUE][i]);
		mtCb.queue[CC_QUEUE][i].queueId = i;
		mtCb.queue[CC_QUEUE][i].queueType = CC_QUEUE;
		/*queueName = (char *)malloc(sizeof(100));*/
		/*sPrint(queueName, "QUEUE_%d_%d\0",CC_QUEUE,i);*/
		queueLock[CC_QUEUE][i] = CreateLock(mtCb.cc[i].name);
		queueCondVar[CC_QUEUE][i] = CreateCV(mtCb.cc[i].name);
	}
}

/*
 * This is to initialize 
 * all locks pertaining to test
 * case 2.
 * */
void initializeAllLocks_Test2(int debugId)
{
	int i=0, j=0; /*char *lockName;*/

	for(i=0; i<mtCb.numCustGrp; i++)
	{
		for(j=0; j<mtCb.custGrp[i].numCust; j++)
		{
			/*lockName = (char *)malloc(sizeof(100));*/
			/*sPrint(lockName, "CUST_%d_%d\0",i,j);*/
			custLock[i][j]=CreateLock(mtCb.custGrp[i].cust[j].name);
			custCondVar[i][j]=CreateCV(mtCb.custGrp[i].cust[j].name);
		}
	}

	for(i=0; i<mtCb.numTC; i++)
	{
		/*lockName = (char *)malloc(sizeof(100));*/
		/*sPrint(lockName, "TC_%d\0",i);*/
		tcLock[i]=CreateLock(mtCb.tc[i].name);
		tcCondVar[i]=CreateCV(mtCb.tc[i].name);
	}
	for(i=0; i<mtCb.numCC; i++)
	{
		/*lockName = (char *)malloc(sizeof(100));*/
		/*sPrint(lockName, "CC_%d\0",i);*/
		ccLock[i]=CreateLock(mtCb.cc[i].name);
		ccCondVar[i]=CreateCV(mtCb.cc[i].name);
	}
}

/*
 * this function is main program 
 * for test cases 2 which proves 
 * Managers only read one from one 
 * Clerk's total money received, 
 * at a time. 
 * */
void movieTheatureMain_Test2()
{
	char dbg[10], logFileName[100];
	int i=0,ret_val=0;

	int debugId=1;
	/*DEBUG_INIT("main.log", &debugId);*/

	/*char *threadName;*/
	initializeAllValues_Test2(debugId);
	initializeAllQueues_Test2(debugId);
	initializeAllLocks_Test2(debugId);
	addAddressToQueue_Test2(debugId);
	mtTest2=1;
	startManThread(debugId);
	startCustThreads(debugId);

	/*DEBUG_CLOSE(debugId);*/
}

/*
 * This function is the main 
 * program for the test case 5
 * which proves Total sales never 
 * suffers from a race condition 
 * */
void movieTheatureMain_Test5()
{
	char dbg[10], logFileName[100];
	int i=0,ret_val=0;

	int debugId=1;
	/*DEBUG_INIT("main.log", &debugId);*/

	/*char *threadName;*/
	initializeAllValues_Test2(debugId);
	initializeAllQueues_Test2(debugId);
	initializeAllLocks_Test2(debugId);
	addAddressToQueue_Test2(debugId);
	mtTest2=1;
	startManThread(debugId);
	startCustThreads(debugId);

	/*DEBUG_CLOSE(debugId);*/
}

/*
 * This is to initialize all
 * values pertaining to test case 
 * 6. in it, we have 3 customer group.
 * */
void initializeAllValues_Test6(int debugId)
{
	int i=0, j=0;

	mtCb.numTC = 0;
	mtCb.numCC = 0;
	mtCb.numTT = 0;

	mtCb.numCust = 3;

	mtCb.numCustGrp=1;	
	for(i=0; i<mtCb.numCustGrp; i++)
	{
/*		mtCb.custGrp[i] = (CustGrp *)malloc(sizeof(CustGrp));*/
		mtCb.custGrp[i].grpId = i;
		mtCb.custGrp[i].location = START;
		mtCb.custGrp[i].numCust = 3; 
		copy_string(mtCb.custGrp[i].name, 20,"CG", 2, -1, i);
		for(j=0; j<3; j++)
		{
/*			mtCb.custGrp[i].cust[j] = (Cust *)malloc(sizeof(Cust));*/
			mtCb.custGrp[i].cust[j].selfId = j;
			mtCb.custGrp[i].cust[j].grpId = i;
			mtCb.custGrp[i].cust[j].money = CUST_MONEY;
			mtCb.custGrp[i].cust[j].IAMTicketBuyer = isCustTicketBuyer(debugId, i,j);
			copy_string(mtCb.custGrp[i].cust[j].name, 20,"CU", 2, i, j);
		}
	}

	AcquireLock(printLock);/*PRINT*/
	Print("Number Of Customers = 3 \n",-1,-1,-1);
	Print("Number Of Groups = 1 \n",-1,-1,-1);
	Print("Number Of TicketClerks = 0 \n",-1,-1,-1);
	Print("Number Of ConcessionClerks = 0 \n",-1,-1,-1);
	Print("Number Of TicketTakers = 0 \n",-1,-1,-1);
	ReleaseLock(printLock);

}

/*
 * This initializes all
 * locks pertaining to test case
 * 6
 * */
void initializeAllLocks_Test6(int debugId)
{
	int i=0, j=0; /*char *lockName;*/

	for(i=0; i<mtCb.numCustGrp; i++)
	{
		for(j=0; j<mtCb.custGrp[i].numCust; j++)
		{
			/*lockName = (char *)malloc(sizeof(100));*/
			/*sPrint(lockName, "CUST_%d_%d\0",i,j);*/
			custLock[i][j]=CreateLock(mtCb.custGrp[i].cust[j].name);
			custCondVar[i][j]=CreateCV(mtCb.custGrp[i].cust[j].name);
		}
	}
}

/*
 * This is the main function
 * pertaining to test case 6 
 * which proves Customer groups 
 * always move together through the
 * theater. This requires explicit 
 * synchronization that you implement
 * */
void movieTheatureMain_Test6()
{
	char dbg[10], logFileName[100];
	int i=0,ret_val=0;

	int debugId=1;
	/*DEBUG_INIT("main.log", &debugId);*/
	initializeForkMec();

	/*char *threadName;*/
	initializeAllValues_Test6(debugId);
	initializeAllLocks_Test6(debugId);
	mtTest6=1;
	startCustThreads(debugId);
	while(mtTest6Called != 3)
	{ 
		YIELD(); 
	}
	for(i=0; i<mtCb.custGrp[0].numCust; i++)
	{
		while(getCustState(debugId,i, 0) != WAIT_AS_OVER)
		{ 
			YIELD(); 
		}
	}
	Exit(0);

	/*DEBUG_CLOSE(debugId);*/
}

/*
 * This is the initialization step
 * pertaining to test case 4.
 * In it, we have taken 2 ticket clerks
 * and 6 customer groups with 1 customer
 *  in each one.
 * */
void initializeAllValues_Test4(int debugId)
{
	int i=0, j=0;

	mtCb.numTC = 2;
	mtCb.numCC = 0;
	mtCb.numTT = 0;
	for(i=0; i<mtCb.numTC; i++)
	{
/*		mtCb.tc[i] = (TC *)malloc(sizeof(TC));*/
		mtCb.tc[i].state = FREE_AND_SELLING_FOOD_BEING_FIRST;
		copy_string(mtCb.queue[TC_QUEUE][i].name, 20,"QQ", 2, 0, i);
		copy_string(mtCb.tc[i].name, 20,"TC", 2, -1, i);
	}
	mtCb.tc[0].state = ON_BREAK;
	mtCb.tc[0].msgByMan = GO_ON_BREAK;

	mtCb.numCust = 6;

	mtCb.numCustGrp=0;	
	i=mtCb.numCust;
	while(i > 0)
	{
		i = i- 1;

/*		mtCb.custGrp[mtCb.numCustGrp] = (CustGrp *)malloc(sizeof(CustGrp));*/
		mtCb.custGrp[mtCb.numCustGrp].grpId = mtCb.numCustGrp;
		mtCb.custGrp[mtCb.numCustGrp].location = START;
		mtCb.custGrp[mtCb.numCustGrp].numCust = 1; 
		copy_string(mtCb.custGrp[mtCb.numCustGrp].name, 20,"CG", 2, -1, mtCb.numCustGrp);
/*		mtCb.custGrp[mtCb.numCustGrp].cust[0] = (Cust *)malloc(sizeof(Cust));*/
		mtCb.custGrp[mtCb.numCustGrp].cust[0].selfId = j;
		mtCb.custGrp[mtCb.numCustGrp].cust[0].grpId = mtCb.numCustGrp;
		mtCb.custGrp[mtCb.numCustGrp].cust[0].money = CUST_MONEY;
		mtCb.custGrp[mtCb.numCustGrp].cust[0].IAMTicketBuyer = isCustTicketBuyer(debugId, mtCb.numCustGrp,j);
		copy_string(mtCb.custGrp[mtCb.numCustGrp].cust[0].name, 20,"CU", 2, mtCb.numCustGrp, 0);
		mtCb.numCustGrp++;

	}

	AcquireLock(printLock);/*PRINT*/
	Print("Number Of Customers = 6 \n",-1,-1,-1);
	Print("Number Of Groups = 6 \n",-1,-1,-1);
	Print("Number Of TicketClerks = 2 \n",-1,-1,-1);
	Print("Number Of ConcessionClerks = 0 \n",-1,-1,-1);
	Print("Number Of TicketTakers = 0 \n",-1,-1,-1);
	ReleaseLock(printLock);

}

/*
 * This is to initialze all
 * queues pertaining to test 
 * case 4.
 * */
void initializeAllQueues_Test4(int debugId )
{

	int i=0;
	/*char *queueName;*/
	for(i=0; i<mtCb.numTC; i++)
	{
/*		mtCb.queue[TC_QUEUE][i] = (mtQueue *)malloc(sizeof(mtQueue));*/
		initializeQueue(debugId, &mtCb.queue[TC_QUEUE][i]);
		mtCb.queue[TC_QUEUE][i].queueId = i;
		mtCb.queue[TC_QUEUE][i].queueType = TC_QUEUE;
		/*queueName = (char *)malloc(sizeof(100));*/
		/*sPrint(queueName, "QUEUE_%d_%d\0",TC_QUEUE,i);*/
		queueLock[TC_QUEUE][i] = CreateLock(mtCb.tc[i].name);
		queueCondVar[TC_QUEUE][i] = CreateCV(mtCb.tc[i].name);
	}

}

/*
 * This is to initialze all
 * locks pertaing to test case
 * 4
 * */
void initializeAllLocks_Test4(int debugId)
{
	int i=0, j=0; /*char *lockName;*/

	for(i=0; i<mtCb.numCustGrp; i++)
	{
		for(j=0; j<mtCb.custGrp[i].numCust; j++)
		{
			/*lockName = (char *)malloc(sizeof(100));*/
			/*sPrint(lockName, "CUST_%d_%d\0",i,j);*/
			custLock[i][j]=CreateLock(mtCb.custGrp[i].cust[j].name);
			custCondVar[i][j]=CreateCV(mtCb.custGrp[i].cust[j].name);
		}
	}


	for(i=0; i<mtCb.numTC; i++)
	{
		/*lockName = (char *)malloc(sizeof(100));*/
		/*sPrint(lockName, "TC_%d\0",i);*/
		tcLock[i]=CreateLock(mtCb.tc[i].name);
		tcCondVar[i]=CreateCV(mtCb.tc[i].name);
	}
}


/*
 * This is the main program 
 * pertaining to test case 4
 * which proves Managers get 
 * Clerks off their break when 
 * lines get too long
 * */
void movieTheatureMain_Test4()
{
	/*char *dbg = NULL, logFileName[100];*/
	char dbg[10], logFileName[100];
	int i=0,ret_val=0;

	int debugId=1;
	/*DEBUG_INIT("main.log", &debugId);*/
	mtTest4=1;

	initializeAllValues_Test4(debugId);
	initializeAllQueues_Test4(debugId);
	initializeAllLocks_Test4(debugId);

	startCustThreads(debugId);
	startManThread(debugId);
	
	
	/*DEBUG_CLOSE(debugId);*/
}

/*
 * This is the foremost 
 * function of the whole project
 * it gives the whole test quite
 * the simulation. All choices 
 * are described in it.
 *
 * */
void project1()
{
	int choice=-1;
	AcquireLock(printLock);/*PRINT*/
	Print("\n\n\t\t-------- CSCI 402: Assignment 1 (Made By Group 3)--------\n\n\n",-1,-1,-1);
	Print("Please enter the corresponding number from the options below to run desired simulation or testcases: \n\n\n",-1,-1,-1);
	Print("\t1.  Run the given testsuite for Part 1\n\n",-1,-1,-1);
	Print("\t2.  Run the test for Part 1 made by us \n\t\t: It shows that order of acquire and release is mantained across several cycles\n\n",-1,-1,-1);
	Print("\t3.  Run the Test 1 for Part 2 \n\t\t: It shows that Customers always take the shortest line, but no 2 customers ever choose the same shortest line at the same time \n\n",-1,-1,-1);
	Print("\t4.  Run the Test 2 for Part 2 \n\t\t: It shows that Managers only read one from one Clerk's total money received, at a time \n\n",-1,-1,-1);
	Print("\t5.  Run the Test 3 for Part 2 \n\t\t: It shows that Customers do not leave a Clerk, or TicketTaker, until they are told to do so. Clerks and TicketTakers do not start with another Customer until they know the current Customer has left. customer until they know that the last Customer has left their area \n\n",-1,-1,-1);
	Print("\t6.  Run the Test 4 for Part 2 \n\t\t: It shows that Managers get Clerks off their break when lines get too long \n\n",-1,-1,-1);
	Print("\t7.  Run the Test 5 for Part 2 \n\t\t: It shows that Total sales never suffers from a race condition \n\n",-1,-1,-1);
	Print("\t8.  Run the Test 6 for Part 2 \n\t\t: It shows that Customer groups always move together through the theater. This requires explicit synchronization that you implement. \n\n",-1,-1,-1);
	Print("\t9. Run complete simulation with 6 customers and 1 employee of each type\n\n",-1,-1,-1);
	Print("\t10. Run complete simulation with 40 customers and maximum allowable employees of each type\n\n",-1,-1,-1);
	Print("\t11. Run complete customised simulation (with user inputs)\n\n",-1,-1,-1);
	ReleaseLock(printLock);

	/*scanf("%d",&choice);*/
	/*choice = Scan(&choice);*/

	switch (choice)
	{
		case 1 :
			AcquireLock(printLock);/*PRINT*/
			Print("You selected %d. The given testsuite for part 1 will now run...\n\n", choice,-1,-1);
			ReleaseLock(printLock);
			/*ThreadTest_ForPart1();*/
			break;
		case 2 :
			AcquireLock(printLock);/*PRINT*/
			Print("You selected %d. Our self made test for Part 1 will now run...\n\n", choice,-1,-1);
			ReleaseLock(printLock);
			/*SelfTest1_ForPart1();*/
			break;
		case 3 :
			AcquireLock(printLock);/*PRINT*/
			Print("You selected %d. The Test 1 for Part 2 will now run...\n\n", choice,-1,-1);
			ReleaseLock(printLock);
			/*movieTheatureMain_Test1();*/
			break;
		case 4 :
			AcquireLock(printLock);/*PRINT*/
			Print("You selected %d. The Test 2 for Part 2 will now run...\n\n", choice,-1,-1);
			ReleaseLock(printLock);
			/*movieTheatureMain_Test2();*/
			break;
		case 5 :
			AcquireLock(printLock);/*PRINT*/
			Print("You selected %d. The Test 3 for Part 2 will now run...\n\n", choice,-1,-1);
			ReleaseLock(printLock);
			/*movieTheatureMain_Test3();*/
			break;
		case 6 :
			AcquireLock(printLock);/*PRINT*/
			Print("You selected %d. The Test 4 for Part 2 will now run...\n\n", choice,-1,-1);
			ReleaseLock(printLock);
			/*movieTheatureMain_Test4();*/
			break;
		case 7 :
			AcquireLock(printLock);/*PRINT*/
			Print("You selected %d. The Test 5 for Part 2 will now run...\n\n", choice,-1,-1);
			ReleaseLock(printLock);
			movieTheatureMain_Test5();
			break;
		case 8 :
			AcquireLock(printLock);/*PRINT*/
			Print("You selected %d. The Test 6 for Part 2 will now run...\n\n", choice,-1,-1);
			ReleaseLock(printLock);
			/*movieTheatureMain_Test6();*/
			break;
		case 9 :
			AcquireLock(printLock);/*PRINT*/
			Print("You selected %d. A complete simulation with 6 customers and 1 employee of each type will now run...\n\n", choice,-1,-1);
			ReleaseLock(printLock);
			/*movieTheatureMain_Test8();*/
			break;
		case 10 :
			AcquireLock(printLock);/*PRINT*/
			Print("You selected %d. A complete simulation, with 40 customers and maximum allowable employees of each type, will now run...\n\n", choice,-1,-1);
			ReleaseLock(printLock);
			/*movieTheatureMain_Test7();*/
			break;
		case 11 :
			AcquireLock(printLock);/*PRINT*/
			Print("You selected %d. A complete simulation with customised inputs will now run...\n\n", choice,-1,-1);
			ReleaseLock(printLock);
			movieTheatureMain();
			break;
		default:
			AcquireLock(printLock);/*PRINT*/
			Print("You gave invalid input. The program will Exit now...\n\n",-1,-1,-1);
			ReleaseLock(printLock);
	}
	return;
}

