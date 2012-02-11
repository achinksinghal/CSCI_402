#include "syscall.h"
#include "movietheature_main.h"

/*extern void ThreadTest_ForPart1();*/
/*extern void SelfTest1_ForPart1();*/

/*struct DebugControlBlock debugCb;*/

int gDebugFileNo=1;
char * printLock;
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
char * manLock;
char * ticketTaking;
char * tcLock[MAX_TC];
char * ttLock[MAX_TT];
char * ccLock[MAX_CC];
char * seats;
char * mtLock;
char * custLock[MAX_CUST_GRP][MAX_CUST];
char * queueLock[MAX_QUEUE_TYPE][MAX_NUM_QUEUE];
int sim = 1;
MV simu;
char * manCondVar;
char * mtCondVar;
char * tcCondVar[MAX_TC];
char * ttCondVar[MAX_TT];
char * ccCondVar[MAX_CC];
char * custCondVar[MAX_CUST_GRP][MAX_CUST];
char * queueCondVar[MAX_QUEUE_TYPE][MAX_NUM_QUEUE];
/*
 * This are certain variables
 * required for specific test 
 * cases
 * */
int IsSeatsOccupied =0;
int money[2][5];
char print_lockName[25];

/* Movie Theature control 
 * block
 * */
MtCb mtCb;

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


void initialize()
{
	print_lockName[0]='P';
	print_lockName[1]='R';
	print_lockName[2]='\0';
	print_lockName[3]='\0';
	printLock = print_lockName;
	CreateLock(print_lockName);
	initializeMoney();
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
 * setting new state for ticket clerk
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
	state = getMv(&mtCb.custGrp[grpId].state);
	return state;
}

/*
 * finding value of location for a customer group
 * */
int getCustGrpLocation(int debugId, int grpId)
{
	return getMv(&mtCb.custGrp[grpId].location);
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
		WaitCV(mtLock,mtCondVar);
	}
	else if ( state == MOVIE_IS_PLAYING || state == NO_MOVIE_REQD )
	{
		SignalCV(mtLock,mtCondVar);		
		setMv(&mtCb.mt.msgToMan, STARTING_MOVIE);
	}
	
	ReleaseLock(mtLock); /*release the lock */
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
	setMv(&queue->numCust, 0);
	queue->state.name[0] = 'S';
	queue->state.name[1] = 'Q';
	queue->state.name[2] = '0'+queue->queueId;
	queue->state.name[3] = '0'+queue->queueType;
	queue->state.name[4] = '\0';
	createMv(&queue->state);
	setMv(&queue->state, NO_CUSTOMER_NO_ADDRESS);
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
	/*char *queueName;*/
	for(i=0; i<mtCb.numTC; i++) /*setting up requisite number of ticket clerk queues*/
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

	for(i=0; i<mtCb.numCC; i++) /*setting up requisite number of concession clerk queues*/
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

	for(i=0; i<mtCb.numTT; i++) /*setting up requisite number of ticket taker queues*/
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

	for(i=0; i<mtCb.numCustGrp; i++)
	{ /*we start with the customers and create locks and CVs for them*/
		for(j=0; j<mtCb.custGrp[i].numCust; j++)
		{
			custLock[i][j]=mtCb.custGrp[i].cust[j].name;
			CreateLock(mtCb.custGrp[i].cust[j].name);
			custCondVar[i][j]=mtCb.custGrp[i].cust[j].name;
			CreateCV(mtCb.custGrp[i].cust[j].name);
		}
	}

	for(i=0; i<mtCb.numTC; i++)
	{ /*locks and CVs for ticket clerks*/
		tcLock[i]=mtCb.tc[i].name;
		CreateLock(mtCb.tc[i].name);
		tcCondVar[i]=mtCb.tc[i].name;
		CreateCV(mtCb.tc[i].name);
	}
	for(i=0; i<mtCb.numCC; i++)
	{/* locks and CVs for concession clerks */

		ccLock[i]=mtCb.cc[i].name;
		CreateLock(mtCb.cc[i].name);
		ccCondVar[i]=mtCb.cc[i].name;
		CreateCV(mtCb.cc[i].name);
	}
	for(i=0; i<mtCb.numTT; i++)
	{	/* locks and CVs for ticket takers*/

		ttLock[i]=mtCb.tt[i].name;
		CreateLock(mtCb.tt[i].name);
		ttCondVar[i]=mtCb.tt[i].name;
		CreateCV(mtCb.tt[i].name);
	}

/*Now initialising individual locks and CVs for:*/
	/*---- manager*/
	manLock=mtCb.man.name;
	CreateLock(mtCb.man.name);
	manCondVar=mtCb.man.name;
	CreateCV(mtCb.man.name);

	/*--- seats*/
	mtCb.ticketTaking_name[0] = 'T';
	mtCb.ticketTaking_name[1] = 't';
	mtCb.ticketTaking_name[2] = '\0';
	mtCb.ticketTaking_name[3] = '\0';
	mtCb.seats_name[0] = 'S';
	mtCb.seats_name[1] = 't';
	mtCb.seats_name[2] = '\0';
	mtCb.seats_name[3] = '\0';
	ticketTaking=mtCb.ticketTaking_name;
	CreateLock(mtCb.ticketTaking_name);
	/*---ticket takers*/
	seats=mtCb.seats_name;
	CreateLock(mtCb.seats_name);
	/*---movie technician*/
	mtLock=mtCb.mt.name;
	CreateLock(mtCb.mt.name);
	mtCondVar=mtCb.mt.name;
	CreateCV(mtCb.mt.name);
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
 * we'll be initialising values, 
 * assigning ids to identities
 * and allocating memory to these.
 * */
void initializeAllValues(int debugId)
{
	int i=0, j=0;

	if(sim == 1 || sim == 3)	
	mtCb.numTC = 3;
	if(sim == 2)
	mtCb.numTC = 1;

	for(i=0; i<mtCb.numTC; i++)
	{
		copy_string(mtCb.queue[TC_QUEUE][i].name, 20,"QQ", 2, 0, i);
		copy_string(mtCb.tc[i].name, 20,"TC", 2, -1, i);

		mtCb.tc[i].money.name[0] = 'M';
		mtCb.tc[i].money.name[1] = 'T';
		mtCb.tc[i].money.name[2] = 'C';
		mtCb.tc[i].money.name[3] = '0' + i;
		mtCb.tc[i].money.name[4] = '\0';
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
		mtCb.tc[i].msgByMan.name[0] = 'M';
		mtCb.tc[i].msgByMan.name[1] = 'B';
		mtCb.tc[i].msgByMan.name[2] = 'M';
		mtCb.tc[i].msgByMan.name[3] = '0' + i;
		mtCb.tc[i].msgByMan.name[4] = 'T';
		mtCb.tc[i].msgByMan.name[5] = '\0';
		createMv(&mtCb.tc[i].state);
		createMv(&mtCb.tc[i].money);
		createMv(&mtCb.tc[i].currentCustId);
		createMv(&mtCb.tc[i].currentCustGrpId);
		createMv(&mtCb.tc[i].msgFromCust);
		createMv(&mtCb.tc[i].msgByMan);
		createMv(&mtCb.tc[i].msgBuffer);
		setMv(&mtCb.tc[i].money, 0);
		setMv(&mtCb.tc[i].msgByMan, INVALID_MSG);
		setMv(&mtCb.tc[i].currentCustId, -1);
		setMv(&mtCb.tc[i].currentCustGrpId, -1);
		setMv(&mtCb.tc[i].msgFromCust, INVALID_MSG);
		setMv(&mtCb.tc[i].msgBuffer, INVALID_MSG);
	}

	if(sim == 1 || sim == 3)	
	mtCb.numCC = 3;
	if(sim == 2)
	mtCb.numCC = 1;

	for(i=0; i<mtCb.numCC; i++)
	{
		copy_string(mtCb.queue[CC_QUEUE][i].name, 20,"QQ", 2, 1, i);
		copy_string(mtCb.cc[i].name, 20,"CC", 2, -1, i);

		mtCb.cc[i].money.name[0] = 'M';
		mtCb.cc[i].money.name[1] = 'C';
		mtCb.cc[i].money.name[2] = 'C';
		mtCb.cc[i].money.name[3] = '0' + i;
		mtCb.cc[i].money.name[4] = '\0';
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
		mtCb.cc[i].msgByMan.name[0] = 'M';
		mtCb.cc[i].msgByMan.name[1] = 'B';
		mtCb.cc[i].msgByMan.name[2] = 'M';
		mtCb.cc[i].msgByMan.name[3] = '0' + i;
		mtCb.cc[i].msgByMan.name[4] = 'C';
		mtCb.cc[i].msgByMan.name[5] = '\0';
		createMv(&mtCb.cc[i].state);
		createMv(&mtCb.cc[i].money);
		createMv(&mtCb.cc[i].currentCustId);
		createMv(&mtCb.cc[i].currentCustGrpId);
		createMv(&mtCb.cc[i].msgFromCust);
		createMv(&mtCb.cc[i].msgBuffer);
		createMv(&mtCb.cc[i].msgByMan);
		setMv(&mtCb.cc[i].money, 0);
		setMv(&mtCb.cc[i].msgByMan, INVALID_MSG);
		setMv(&mtCb.cc[i].currentCustId, -1);
		setMv(&mtCb.cc[i].currentCustGrpId, -1);
		setMv(&mtCb.cc[i].msgFromCust, INVALID_MSG);
		setMv(&mtCb.cc[i].msgBuffer, INVALID_MSG);
	}
	if(sim == 1 || sim == 3)	
	mtCb.numTT = 3;
	if(sim == 2)
	mtCb.numTT = 1;
	
	for(i=0; i<mtCb.numTT; i++)
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
		mtCb.tt[i].msgByMan.name[0] = 'M';
		mtCb.tt[i].msgByMan.name[1] = 'B';
		mtCb.tt[i].msgByMan.name[2] = 'M';
		mtCb.tt[i].msgByMan.name[3] = '0' + i;
		mtCb.tt[i].msgByMan.name[4] = 'D';
		mtCb.tt[i].msgByMan.name[5] = '\0';
		mtCb.tt[i].msgToMan.name[0] = 'M';
		mtCb.tt[i].msgToMan.name[1] = 'T';
		mtCb.tt[i].msgToMan.name[2] = 'M';
		mtCb.tt[i].msgToMan.name[3] = '0' + i;
		mtCb.tt[i].msgToMan.name[4] = 'D';
		mtCb.tt[i].msgToMan.name[5] = '\0';
		createMv(&mtCb.tt[i].state);
		createMv(&mtCb.tt[i].currentCustId);
		createMv(&mtCb.tt[i].currentCustGrpId);
		createMv(&mtCb.tt[i].msgFromCust);
		createMv(&mtCb.tt[i].msgByMan);
		createMv(&mtCb.tt[i].msgToMan);
		setMv(&mtCb.tt[i].msgByMan, INVALID_MSG);
		setMv(&mtCb.tt[i].msgToMan, INVALID_MSG);
		setMv(&mtCb.tt[i].currentCustId, -1);
		setMv(&mtCb.tt[i].currentCustGrpId, -1);
		setMv(&mtCb.tt[i].msgFromCust, INVALID_MSG);
	}

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
	setMv(&mtCb.man.msgToAll, INVALID_MSG);

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
	setMv(&mtCb.mt.msgByMan, INVALID_MSG);

	mtCb.numOfTicketsTaken.name[0] = 'N';
	mtCb.numOfTicketsTaken.name[1] = 'T';
	mtCb.numOfTicketsTaken.name[2] = 'T';
	mtCb.numOfTicketsTaken.name[3] = '\0';
	createMv(&mtCb.numOfTicketsTaken);
	setMv(&mtCb.numOfTicketsTaken, 0);


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
			setMv(&mtCb.mvRoom.seat[i][j].custId, -1);
			setMv(&mtCb.mvRoom.seat[i][j].custGrpId, -1);
			setMv(&mtCb.mvRoom.seat[i][j].isTaken, 0);
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
			setMv(&mtCb.custGrp[mtCb.numCustGrp].cust[j].seatI, -1);

			mtCb.custGrp[mtCb.numCustGrp].cust[j].seatJ.name[0] = 'S';
			mtCb.custGrp[mtCb.numCustGrp].cust[j].seatJ.name[1] = 'J';
			mtCb.custGrp[mtCb.numCustGrp].cust[j].seatJ.name[2] = 'C';
			mtCb.custGrp[mtCb.numCustGrp].cust[j].seatJ.name[3] = '0' + j;
			mtCb.custGrp[mtCb.numCustGrp].cust[j].seatJ.name[4] = '0' + mtCb.numCustGrp;
			mtCb.custGrp[mtCb.numCustGrp].cust[j].seatJ.name[5] = '\0';
			createMv(&mtCb.custGrp[mtCb.numCustGrp].cust[j].seatJ);
			setMv(&mtCb.custGrp[mtCb.numCustGrp].cust[j].seatJ, -1);

			mtCb.custGrp[mtCb.numCustGrp].cust[j].seatTaken.name[0] = 'S';
			mtCb.custGrp[mtCb.numCustGrp].cust[j].seatTaken.name[1] = 'E';
			mtCb.custGrp[mtCb.numCustGrp].cust[j].seatTaken.name[2] = 'T';
			mtCb.custGrp[mtCb.numCustGrp].cust[j].seatTaken.name[3] = '0' + j;
			mtCb.custGrp[mtCb.numCustGrp].cust[j].seatTaken.name[4] = '0' + mtCb.numCustGrp;
			mtCb.custGrp[mtCb.numCustGrp].cust[j].seatTaken.name[5] = '\0';
			createMv(&mtCb.custGrp[mtCb.numCustGrp].cust[j].seatTaken);
			setMv(&mtCb.custGrp[mtCb.numCustGrp].cust[j].seatTaken, 0);
		}
		mtCb.custGrp[mtCb.numCustGrp].cust[0].IAMTicketBuyer = 1;
		mtCb.numCustGrp++;
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
/*
 * Manager managing accounts with Ticket clerk
 * */
void moneyTcManage(int debugId)
{		
	int i=0;
	int currentMoney=0;	
	for( i=0; i<mtCb.numTC; i++)
	{	
		currentMoney =  getMv(&mtCb.tc[i].money);
		
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
		currentMoney =  getMv(&mtCb.cc[i].money);
		
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
                if(j != i && ( getTtState(debugId, j) == ON_BREAK || getMv(&mtCb.tt[j].msgByMan) == GO_ON_BREAK))
                    k++;/*number of other TTs on break or who've have been asked by manager to go on break*/
            }
            if(k != (mtCb.numTT - 1) && checkChances(debugId, 20))
            {/*if not everyone else is on break, we check chances if this taker can go on break - and, if he can then we...*/
                setMv(&mtCb.tt[i].msgByMan, GO_ON_BREAK);
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
				if(j != i && ( getTcState(debugId, j) == ON_BREAK || getMv(&mtCb.tc[j].msgByMan)==GO_ON_BREAK))
					k++; /*number of other ticket clerks on break or who've have been asked by manager to go on break*/

			}
			if(k != (mtCb.numTC - 1) && checkChances(debugId, 20))
			{/*if not everyone else is on break, we check chances if this clerk can go on break - and, if he can then we...*/
				setMv(&mtCb.tc[i].msgByMan, GO_ON_BREAK);
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
                if(j != i && ( getCcState(debugId, j) == ON_BREAK || getMv(&mtCb.cc[j].msgByMan)==GO_ON_BREAK))
                    k++;/*number of other TCs on break or who've have been asked by manager to go on break*/
            }
            if(k != (mtCb.numCC - 1) && checkChances(debugId, 20))
            {/*if not everyone else is on break, we check chances if this conc clerk can go on break - and, if he can then we...*/
		    setMv(&mtCb.cc[i].msgByMan, GO_ON_BREAK);
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


int SeatsTaken(int debugId)/*This function will be used to know how many seats are available at a time*/
{
	int i=0,seatsCount=0;
	int j=0;
	for (i = 0; i < MAX_ROWS; i++)/*rows in movie room*/
	{	
		for (j=0; j<MAX_COLS ; j++)/*cols in movie room*/
		{
			if(
				getMv(&mtCb.mvRoom.seat[i][j].custId) != -1 && 
				getMv(&mtCb.mvRoom.seat[i][j].custGrpId) != -1 && 
				getMv(&mtCb.mvRoom.seat[i][j].isTaken) == 1
			  ) 
			{
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
		if( getMv(&mtCb.tt[i].msgToMan) == START_MOVIE )
		{/*once movie is begun, manager asks ticket takers to go on break*/
			setMv(&mtCb.tt[i].msgByMan, GO_ON_BREAK);
			Print("Manager has told TicketTaker %d to go on break.\n", i,-1,-1);
			setMv(&mtCb.man.msgToAll, MOVIE_RUNNING);
			setMv(&mtCb.tt[i].msgToMan, INVALID_MSG);
		}
	}

	if (getMv(&mtCb.man.msgToAll) == MOVIE_RUNNING)
	{/*manager tells everyone that movie is running*/
		/*for(i=0; i<mtCb.numTT; i++)
		{
			if( (   getTtState(debugId, i) == FREE_AND_TAKING_TICKET ||
				getTtState(debugId, i) == FREE_AND_TAKING_TICKET_BEING_FIRST)
				 && 
				( getMv(&mtCb.tt[i].currentCustId) == -1 || 
				 getMv(&mtCb.tt[i].currentCustGrpId) == -1) )
			{
				acquireTtLock(debugId, i);*//*aquire its loc*/
				/*signalToTtLock(debugId, i);*//*signal it to wake up*/
				/*releaseTtLock(debugId, i);*//*let go of lock*/
				/*YIELD();*//*manager yields to let others run*/
			/*}
			while(getTtState(debugId, i) != ON_BREAK )
			{ *//*the ticket taker is working but there is no customer at him*/
				/*YIELD();
			}
		}*/

		if (getMtState(debugId) == MOVIE_IS_NOT_PLAYING )/*We havent checked here if all 25 have to be in*/
		{	
			while( !IsSeatsOccupied && SeatsTaken(debugId) != 0) /*seats taken by custs should be equal to zero b4 next movie starts*/
			{ 
				YIELD(); /*manager waits for seats to be taken*/
			}
			sim = getMv(&simu);
			if(sim == 1 || sim == 3)
			{
				int grp_loc = getCustGrpLocation(debugId, 5);
				if( 
						grp_loc != START && 
						grp_loc != TICKET_CLERK && 
						grp_loc != CONCESSION_CLERK && 
						grp_loc != LOBBY && 
						grp_loc != MOVIEROOM 
				  )
				{
					changeMtState(debugId, NO_MOVIE_REQD);
					return;
				}
	
			}
			else if(sim == 2)
			{
				int grp_loc = getCustGrpLocation(debugId, 1);
				if( 
						grp_loc != START && 
						grp_loc != TICKET_CLERK && 
						grp_loc != CONCESSION_CLERK && 
						grp_loc != LOBBY && 
						grp_loc != MOVIEROOM 
				  )
				{
					changeMtState(debugId, NO_MOVIE_REQD);
					return;
				}

			}

			if(!IsSeatsOccupied)
			{/*seats are taken so ticket takers list of tickets taken is to be completed*/
				resetNumOfTicketTaken(debugId);
				setMv(&mtCb.man.msgToAll, MOVIE_NOT_RUNNING);
			/*sending msg to the Ticket Taker to take tickets and fill seats	*/
				sim = getMv(&simu);
				if(sim == 2)
				{/*we set ticket taker to initial values*/
					i = 2;
					acquireTtLock(debugId, 2);
					setMv(&mtCb.tt[i].msgByMan, FILL_CUST);
					setMv(&mtCb.tt[i].msgToMan, INVALID_MSG);
					setMv(&mtCb.tt[i].currentCustId, -1);
					setMv(&mtCb.tt[i].currentCustGrpId, -1);
					signalToTtLock(debugId, 2);
					releaseTtLock(debugId, 2);
				}
				else if( sim == 3 || sim == 1)
				{/*we set ticket taker to initial values*/
					i = 2;
					acquireTtLock(debugId, 2);
					setMv(&mtCb.tt[i].msgByMan, FILL_CUST);
					setMv(&mtCb.tt[i].msgToMan, INVALID_MSG);
					setMv(&mtCb.tt[i].currentCustId, -1);
					setMv(&mtCb.tt[i].currentCustGrpId, -1);
					signalToTtLock(debugId, 2);
					releaseTtLock(debugId, 2);
				}
				
			}
			IsSeatsOccupied = 1;
			if(getMv(&mtCb.man.msgToAll) == MOVIE_NOT_RUNNING)
			{  /*if movie is not running, tickets can be given to ticket taker*/
				i = 5;
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
	if (getMv(&mtCb.man.msgToAll) == MOVIE_RUNNING)
	{
		int sTaken = SeatsTaken(debugId);
		int numTcktTaken = getNumOfTicketTaken(debugId);
		sim = getMv(&simu);
		if(((sim == 1 || sim == 3) && sTaken == 25) || ( sim == 2 && sTaken == 10) || (sim == 4  && sTaken == 5) )
		{

			if(sTaken == numTcktTaken && sTaken != 0 && (getMtState(debugId) == MOVIE_IS_NOT_PLAYING || getMtState(debugId) == STARTED_BY_MAN))
			{ /*we see if seats have been taken*/
				/*Ask the Technician to start movie;*/
				setMv(&mtCb.mt.msgByMan, START_MOVIE);
				AcquireLock(printLock);/*PRINT*/
				Print("Manager is telling the MovieTechnician to start the movie\n",-1,-1,-1);
				ReleaseLock(printLock);
				changeMtState(debugId, MOVIE_IS_PLAYING);	
				IsSeatsOccupied = 0;
			}
		}
	}
}

/*
 * Manager Main's Program
 * */
void manMain()
{
	int a = 0;
	int debugId = 2;
	int ret_val=0;
	int i=0, j=0;
	MV start;
	Manager *self = &mtCb.man;

	IsSeatsOccupied = 0;
	self = &mtCb.man;

	setMv(&mtCb.man.msgToAll, MOVIE_NOT_RUNNING); /*we intialize states of all workers to be MOVIE_NOT_RUNNING*/

	for(i=0; i<mtCb.numTC; i++)
	{/*the manager will start threads of the ticket clerks*/
		mtCb.tc[i].queueType = TC_QUEUE; 
		mtCb.tc[i].queue = &mtCb.queue[TC_QUEUE][i];
		mtCb.tc[i].location = TICKET_CLERK;
		changeTcState(debugId, i, STARTED_BY_MAN);
	}
	for(i=0; i<mtCb.numCC; i++)
	{	/*the manager will now start concession clerks*/
		mtCb.cc[i].queueType = CC_QUEUE; 
		mtCb.cc[i].queue = &mtCb.queue[CC_QUEUE][i];
		mtCb.cc[i].location = CONCESSION_CLERK;
		changeCcState(debugId, i, STARTED_BY_MAN);
	}
	for(i=0; i<mtCb.numTT; i++)
	{/*the manager will now start ticket takers*/
		mtCb.tt[i].queueType = TT_QUEUE; 
		mtCb.tt[i].queue = &mtCb.queue[TT_QUEUE][i];
		mtCb.tt[i].location = MOVIEROOM;
		changeTcState(debugId, i, STARTED_BY_MAN);
	}
	start.name[0]='Z';
	start.name[1]='\0';

	createMv(&start);
	setMv(&start, 1);

	/*setting up the movie technician*/
	mtCb.mt.location = MOVIEROOM;
	while(1)
	{
		moneyTcManage(debugId);
		moneyCcManage(debugId);
		Print("Total money made by office = %d\n", mtCb.man.money,-1,-1);
		movieManage(debugId);	
		sim = getMv(&simu);
		if(sim == 1 || sim == 3 || sim == 4)
		{
			i = 5;
			{
				if(getCustGrpLocation(debugId, i) == OUT_OF_MOVIETHEATURE)
				return;
			}
		}
		else if(sim == 2)
		{
			i=1;
			{
				if(getCustGrpLocation(debugId, i) == OUT_OF_MOVIETHEATURE)
				return;
			}
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
	char dbg[10], logFileName[100];
	int i=0,ret_val=0;

	int debugId=1;

	simu.name[0] = 'S';
        simu.name[1] = 'I';
        simu.name[2] = 'M';
        simu.name[4] = '\0';
        createMv(&simu);
	sim = getMv(&simu);


	initialize();
	initializeAllValues(debugId); /*intializing entities that shall be part of simulation*/
	initializeAllQueues(debugId);/*initializing all different queues that need to be formed*/
	initializeAllLocks(debugId);/*set up required locks*/
	manMain();/*start the manager*/
	Exit(0);

}

