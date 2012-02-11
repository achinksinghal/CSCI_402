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
char * ttLock[MAX_TT];
char * seats;
char * custLock[MAX_CUST_GRP][MAX_CUST];
char * queueLock[MAX_QUEUE_TYPE][MAX_NUM_QUEUE];
int tt_number;
int tt_id;

MV end_phase1;
MV end_phase2;
int ttNumLock;

char * manCondVar;
char * mtCondVar;
char * ttCondVar[MAX_TT];
char * custCondVar[MAX_CUST_GRP][MAX_CUST];
char * queueCondVar[MAX_QUEUE_TYPE][MAX_NUM_QUEUE];
/*
 * This are certain variables
 * required for specific test 
 * cases
 * */
int IsSeatsOccupied;
int money[2][5];
MV tt_num;
MV sim;
char print_lockName[25];

/* Movie Theature control 
 * block
 * */
MtCb mtCb;
/*
 * wrapper for Yield
 * */
void YIELD()
{
	Yield();
}

/*
 * Wrapper functions
 * for monitor variables
 * */
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

void initialize()
{
	print_lockName[0]='P';
	print_lockName[1]='R';
	print_lockName[2]='\0';
	print_lockName[3]='\0';
	printLock = print_lockName;
	CreateLock(print_lockName);
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
 * removing a customer from a queue
 * */
Cust* queueRemoveCust(int debugId, mtQueue *queue)
{
	/*We acquire a lock on the queue and then modify it. A customer is removed and is ready to be served.*/
	Cust *cust;
	int currCustGrp;
	int currCust;

	AcquireLock(queueLock[queue->queueType][queue->queueId]);
	SignalCV(queueLock[queue->queueType][queue->queueId],queueCondVar[queue->queueType][queue->queueId]);
	ReleaseLock(queueLock[queue->queueType][queue->queueId]);
	if( queue->queueType == TT_QUEUE )
	{
		while(getMv(&mtCb.tt[queue->queueId].msgFromCust) != CUST_REMOVED)
		{ 
			YIELD();
		}
		setMv(&mtCb.tt[queue->queueId].msgFromCust, INVALID_MSG);
		currCustGrp = getMv(&mtCb.tt[queue->queueId].currentCustGrpId);
		currCust = getMv(&mtCb.tt[queue->queueId].currentCustId);
		cust = &mtCb.custGrp[currCustGrp].cust[currCust];
	}

	AcquireLock(queueLock[queue->queueType][queue->queueId]);

	if(currCust >= 0 || currCustGrp >= 0)
	{/*if there were customers in line, we take them out and reduce count of number of customers in queue. After that, we release lock and return pointer to customer*/
		getMv(&queue->numCust);
		setMv(&queue->numCust, queue->numCust.value - 1);
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
	/*AcquireLock(queueLock[queueType][queueId]); */
	if(queueType == TT_QUEUE)  
	{ /*associating ticket taker with queue*/
		mtCb.tt[id].queue = &mtCb.queue[queueType][queueId];
		mtCb.tt[id].queue->queueType = queueType;
		mtCb.tt[id].queue->queueAddress = (Employee)&mtCb.tt[queueId];
	} 
	/*ReleaseLock(queueLock[queueType][queueId]); */
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
	mtCb.queue[TT_QUEUE][tt_number].queueId = tt_number;
	mtCb.queue[TT_QUEUE][tt_number].queueType = TT_QUEUE;
	initializeQueue(debugId, &mtCb.queue[TT_QUEUE][tt_number]);
	queueLock[TT_QUEUE][tt_number] = mtCb.queue[TT_QUEUE][tt_number].name;
	CreateLock(mtCb.queue[TT_QUEUE][tt_number].name);
	queueCondVar[TT_QUEUE][tt_number] = mtCb.queue[TT_QUEUE][tt_number].name;
	CreateCV(mtCb.queue[TT_QUEUE][tt_number].name);
	addAddressToQueue(debugId, tt_number, TT_QUEUE, tt_number);
}

/*
 * this will initialize the locks and condition variables that we created for each queues, manager, ticker clerks, concession clerks, tikcet takers, customers
 * */
void initializeAllLocks(int debugId)
{
	int i,j;
	i=0, j=0; /*char *lockName;*/

	for(i=(tt_number * 2) ; i <= (tt_number * 2) + 1; i++)
	{ /*we start with the customers and create locks and CVs for them*/
		j=0;
		{
			custLock[i][j]=mtCb.custGrp[i].cust[j].name;
			CreateLock(mtCb.custGrp[i].cust[j].name);
			custCondVar[i][j]=mtCb.custGrp[i].cust[j].name;
			CreateCV(mtCb.custGrp[i].cust[j].name);
		}
	}

	{	/* locks and CVs for ticket takers*/

		ttLock[tt_number]=mtCb.tt[tt_number].name;
		CreateLock(mtCb.tt[tt_number].name);
		ttCondVar[tt_number]=mtCb.tt[tt_number].name;
		CreateCV(mtCb.tt[tt_number].name);
	}
	mtCb.ticketTaking_name[0] = 'T';
        mtCb.ticketTaking_name[1] = 't';
        mtCb.ticketTaking_name[2] = '\0';
        mtCb.ticketTaking_name[3] = '\0';
        ticketTaking=mtCb.ticketTaking_name;
        CreateLock(mtCb.ticketTaking_name);


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
 * we'll be initializing values, 
 * assigning ids to all identities
 * and allocating memory to these.
 * */
void initializeAllValues(int debugId)
{
	int i=0, j=0;
	int ID;
	ID = GetId();
	tt_id = (ID - 14) / 16 ;
	tt_number = tt_id;

	sim.name[0] = 'S';
	sim.name[1] = 'I';
	sim.name[2] = 'M';
	sim.name[4] = '\0';
	createMv(&sim);
	
	copy_string(mtCb.queue[TT_QUEUE][tt_number].name, 20,"QQ", 2, 2, tt_number);
	copy_string(mtCb.tt[tt_number].name, 20,"TT", 2, -1, tt_number);

	mtCb.tt[tt_number].state.name[0] = 'S';
	mtCb.tt[tt_number].state.name[1] = 'T';
	mtCb.tt[tt_number].state.name[2] = 'T';
	mtCb.tt[tt_number].state.name[3] = '0' + tt_number;
	mtCb.tt[tt_number].state.name[4] = '\0';
	mtCb.tt[tt_number].currentCustId.name[0] = 'C';
	mtCb.tt[tt_number].currentCustId.name[1] = 'C';
	mtCb.tt[tt_number].currentCustId.name[2] = 'U';
	mtCb.tt[tt_number].currentCustId.name[3] = '0' + tt_number;
	mtCb.tt[tt_number].currentCustId.name[4] = 'D';
	mtCb.tt[tt_number].currentCustId.name[5] = '\0';
	mtCb.tt[tt_number].currentCustGrpId.name[0] = 'C';
	mtCb.tt[tt_number].currentCustGrpId.name[1] = 'C';
	mtCb.tt[tt_number].currentCustGrpId.name[2] = 'G';
	mtCb.tt[tt_number].currentCustGrpId.name[3] = '0' + tt_number;
	mtCb.tt[tt_number].currentCustGrpId.name[4] = 'D';
	mtCb.tt[tt_number].currentCustGrpId.name[5] = '\0';
	mtCb.tt[tt_number].msgFromCust.name[0] = 'M';
	mtCb.tt[tt_number].msgFromCust.name[1] = 'C';
	mtCb.tt[tt_number].msgFromCust.name[2] = 'U';
	mtCb.tt[tt_number].msgFromCust.name[3] = '0' + tt_number;
	mtCb.tt[tt_number].msgFromCust.name[4] = 'D';
	mtCb.tt[tt_number].msgFromCust.name[5] = '\0';
	mtCb.tt[tt_number].msgByMan.name[0] = 'M';
	mtCb.tt[tt_number].msgByMan.name[1] = 'B';
	mtCb.tt[tt_number].msgByMan.name[2] = 'M';
	mtCb.tt[tt_number].msgByMan.name[3] = '0' + tt_number;
	mtCb.tt[tt_number].msgByMan.name[4] = 'D';
	mtCb.tt[tt_number].msgByMan.name[5] = '\0';
	mtCb.tt[tt_number].msgToMan.name[0] = 'M';
	mtCb.tt[tt_number].msgToMan.name[1] = 'T';
	mtCb.tt[tt_number].msgToMan.name[2] = 'M';
	mtCb.tt[tt_number].msgToMan.name[3] = '0' + tt_number;
	mtCb.tt[tt_number].msgToMan.name[4] = 'D';
	mtCb.tt[tt_number].msgToMan.name[5] = '\0';
	mtCb.tt[tt_number].msgToCust.name[0] = 'M';
	mtCb.tt[tt_number].msgToCust.name[1] = 'T';
	mtCb.tt[tt_number].msgToCust.name[2] = 'C';
	mtCb.tt[tt_number].msgToCust.name[3] = '0' + tt_number;
	mtCb.tt[tt_number].msgToCust.name[4] = 'D';
	mtCb.tt[tt_number].msgToCust.name[5] = '\0';
	createMv(&mtCb.tt[tt_number].msgToMan);

	createMv(&mtCb.tt[tt_number].state);
	createMv(&mtCb.tt[tt_number].currentCustId);
	createMv(&mtCb.tt[tt_number].currentCustGrpId);
	createMv(&mtCb.tt[tt_number].msgFromCust);
	createMv(&mtCb.tt[tt_number].msgToCust);
	createMv(&mtCb.tt[tt_number].msgByMan);

	setMv(&mtCb.tt[tt_number].msgByMan, INVALID_MSG);
	setMv(&mtCb.tt[tt_number].currentCustId, -1);
	setMv(&mtCb.tt[tt_number].currentCustGrpId, -1);
	setMv(&mtCb.tt[tt_number].msgFromCust, INVALID_MSG);
	setMv(&mtCb.tt[tt_number].msgToCust, INVALID_MSG);
	setMv(&mtCb.tt[tt_number].msgToMan, INVALID_MSG);

	mtCb.numOfTicketsTaken.name[0] = 'N';
        mtCb.numOfTicketsTaken.name[1] = 'T';
        mtCb.numOfTicketsTaken.name[2] = 'T';
        mtCb.numOfTicketsTaken.name[3] = '\0';
        createMv(&mtCb.numOfTicketsTaken);



	mtCb.numCustGrp= tt_id * 2;	
	i = ( tt_id * 2 )+1;
	while( mtCb.numCustGrp <= i)
	{
			
		mtCb.custGrp[mtCb.numCustGrp].grpId = mtCb.numCustGrp;
		mtCb.custGrp[mtCb.numCustGrp].numCust = 5; 
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

		}
		mtCb.custGrp[mtCb.numCustGrp].cust[0].IAMTicketBuyer = 1;
		mtCb.numCustGrp++;
	}
}

/*
 * The Ticket Taker Main Program
 * */
void ttMain(int selfId)
{
	int debugId;
	int ret_val=0;
	int i=0;
	TT *self; /*set up a pointer to itself*/
	int currentState;
	int groupInLobby=0;
	int locCustGrp;
	int simu;
	debugId = selfId+400;


	mtCb.tt[selfId].queueType = TT_QUEUE;
	mtCb.tt[selfId].queue = &mtCb.queue[TT_QUEUE][selfId];
	mtCb.tt[i].location = MOVIEROOM;



	self = &mtCb.tt[selfId]; /*set up a pointer to itself*/


	currentState = getTtState(debugId, selfId); /*get the current state*/
	addAddressToQueue(debugId, selfId, TT_QUEUE, selfId);;/*we associate a ticket taker's queue with this ticket taker*/
	setMv(&mtCb.queue[TT_QUEUE][selfId].state, NO_CUSTOMER);
	changeTtState(debugId, selfId, FREE_AND_TAKING_TICKET_BEING_FIRST);


	acquireTtLock(debugId, selfId);/*get lock before modifying*/
	while(1)
	{
		currentState = getTtState(debugId, selfId);
		if( currentState == MAN_STARTING_MOVIE )
		{ /*manager asked for movie to start*/
			changeTtState(debugId, selfId, ON_BREAK);
			Print("TicketTaker %d is going on break.\n", selfId, -1,-1);

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
			
				setMv(&self->currentCustId, -1);/*we clear data before beginning new iteration*/
				setMv(&self->currentCustGrpId, -1);/*we clear data before beginning new iteration*/
			}
			/*broadcastAllTtLock(debugId, selfId); *//*we'll wake up everyone waiting on that tt's lock*/
			setMv(&self->msgByMan, INVALID_MSG);/*this is to ensure that the msg is dealt with only once*/
			waitOnTtLock(debugId, selfId);/*we now go to wait for lock*/
			Print("TicketTaker %d is coming off break.\n", selfId, -1,-1);
			changeTtState(debugId, selfId, FREE_AND_TAKING_TICKET);  /*back from break to take tickets again */
		}
		else if( getMv(&self->msgByMan) == GO_ON_BREAK)
		{/*has been asked to go on break*/
			setMv(&self->msgToMan, INVALID_MSG);
			changeTtState(debugId, selfId, ON_BREAK);/*change state to being on break*/
			setMv(&self->msgByMan, INVALID_MSG);
			Print("TicketTaker %d is going on break.\n", selfId, -1,-1);
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
				setMv(&self->currentCustId, -1);/*we clear data before beginning new iteration*/
				setMv(&self->currentCustGrpId, -1);/*we clear data before beginning new iteration*/
			}
			broadcastAllTtLock(debugId, selfId);/*wake up all those who were waiting for the ticket taker's lock*/
			setMv(&self->msgByMan, INVALID_MSG);
			waitOnTtLock(debugId, selfId);/*wait for its own lock*/
			Print("TicketTaker %d is coming off break.\n", selfId, -1, -1);
			changeTtState(debugId, selfId, FREE_AND_TAKING_TICKET);
		}
		else if( getMv(&self->msgByMan) == FILL_CUST )
		{/*manager has asked taker to start taking tickets*/

			changeTtState(debugId, selfId, FREE_AND_TAKING_TICKET);/*setting state of ticket taker as ready to take the first ticket*/
			setMv(&self->msgByMan, INVALID_MSG);/*message was received - now we dissolve it to ensure it is not taken care of again*/
		}
		if( currentState == FREE_AND_TAKING_TICKET ||
				currentState == FREE_AND_TAKING_TICKET_BEING_FIRST )
		{/*if the taker is currently taking tickets*/
			if(getTotalCustCount(debugId, &mtCb.queue[TT_QUEUE][selfId]) == 0 )
			{/*if no one is in queue*/
				Print("TicketTaker %d has no one in line. I am available for a customer.\n", selfId,  -1, -1);
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
					Print("TicketTaker %d has a line length %d and is signalling a customer.\n", selfId, getTotalCustCount(debugId, &mtCb.queue[TT_QUEUE][selfId])+1, -1);


					if( mtCb.custGrp[self->currentCust->grpId].numCust <= (MAX_SEATS - getNumOfTicketTaken(debugId)) )
					{/*movie room has sufficient seats for this group*/
						setMv(&self->msgToCust, YES);
					}
					else
					{	/*movie room diesnt have sufficient seats for this group*/
						Print("TicketTaker %d is not allowing the group into the theater. The number of taken tickets is %d", selfId,  getNumOfTicketTaken(debugId), -1);
						Print(" and the group size is %d.\n", mtCb.custGrp[self->currentCust->grpId].numCust, -1, -1);
						setMv(&self->msgToCust, NO);
					}
					changeCustState(debugId, self->currentCust->selfId, self->currentCust->grpId, SIGNAL);/*now we awake the customer who was waiting for tt to check*/
					/*signalToTtLock(debugId, selfId);tt now let others get access to its CV*/
					waitOnTtLock(debugId, selfId);/* it now goes on wait   */
				}
			}  
		}
		groupInLobby=0;

		/*here finding out if there is any group in the lobby. If there is any then if its size < than the number of seats left in the movieroom, then it shuold be first accomodated. Else movie should be started.*/
		for (i= (tt_id * 2) ; i <= (tt_id * 2) +1 ; i++)
		/*for (i=0; i<mtCb.numCustGrp; i++)*/
		{
			locCustGrp = getMv(&mtCb.custGrp[i].location);
			if(
					locCustGrp == START ||
					locCustGrp == TICKET_CLERK ||
					locCustGrp == CONCESSION_CLERK ||
					locCustGrp == LOBBY
			  )
			{
				groupInLobby=1;/*there is someone in lobby*/
				if( ( 5 <= (MAX_SEATS - getNumOfTicketTaken(debugId)) ))
				{
					changeTtState(debugId, selfId, FREE_AND_TAKING_TICKET);
					break;
				}
			}
		}
		/*if(i == mtCb.numCustGrp && groupInLobby==1)*/
		if( i == ((tt_id * 2) + 2) && groupInLobby==1)
		{/*we have checked all groups in lobby and accomodated them if possible*/
			setMv(&self->msgToMan, START_MOVIE);
			Print("TicketTaker %d has stopped taking tickets\n", selfId, -1, -1);
			changeTtState(debugId, selfId, MAN_STARTING_MOVIE);

		}

		/*here finding out is there is any group in lobby or movieroom, if there is not any group left in lobby and movieroom then tt should Exit the theater - his job is over as everyone of customers who bought tickets have shown him tickets.*/

		/*for (i=0; i<mtCb.numCustGrp; i++)*/
		for (i= (tt_id * 2) ; i <= (tt_id * 2) +1 ; i++)
		{
			locCustGrp = getMv(&mtCb.custGrp[i].location);
			if(
					locCustGrp != START &&
					locCustGrp != TICKET_CLERK &&
				     	locCustGrp != CONCESSION_CLERK &&
					locCustGrp != LOBBY &&
					locCustGrp != MOVIEROOM
			  );
			else  break;
		}
		/*if( i == mtCb.numCustGrp)*/
		if( i == ((tt_id * 2) + 2) )
		{/*all tickets taken by ticket clerk - everyone's been to movie room*/
			Print("TicketTaker %d has stopped taking tickets\n", selfId,-1,-1);

			return;
		}

		/*here finding out, is there is any group in lobby or not, if there is not any group left in lobby then movie should be started.*/
		/*for (i=0; i<mtCb.numCustGrp; i++)*/
		for (i= (tt_id * 2) ; i <= (tt_id * 2) +1 ; i++)
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
		/*if( i == mtCb.numCustGrp)
		if( i == (tt_id * 2) + 2 )*/
		simu = getMv(&sim);
		if( simu == 1 && selfId == 2 && i == 5)
		{
			setMv(&self->msgToMan, START_MOVIE);
			Print("TicketTaker %d has stopped taking tickets\n", selfId,-1,-1);
			setMv(&sim, 3);
			changeTtState(debugId, selfId, MAN_STARTING_MOVIE);
		}
		else if( simu == 1 && selfId == 1 && i==4)
		{
			Print("TicketTaker %d has stopped taking tickets\n", selfId,-1,-1);
			changeTtState(debugId, selfId, ON_BREAK);
			return;
		}
		else if( simu == 1 && selfId == 0 && i == 2)
		{
			Print("TicketTaker %d has stopped taking tickets\n", selfId,-1,-1);
			changeTtState(debugId, selfId, ON_BREAK);
			return;
		}
		else if( simu == 2 && (i == 2))
		{
			setMv(&self->msgToMan, START_MOVIE);
			Print("TicketTaker %d has stopped taking tickets\n", selfId,-1,-1);
			changeTtState(debugId, selfId, ON_BREAK);
			return;
		}
		else if(( simu == 3 || simu == 4) && (i == 4 || i == 2))
		{
			setMv(&self->msgToMan, START_MOVIE);
			Print("TicketTaker %d has stopped taking tickets\n", selfId,-1,-1);
			changeTtState(debugId, selfId, ON_BREAK);
			return;

		}
		else if( simu == 3 && (i == 6 ))
		{
			setMv(&sim, 4);
			setMv(&self->msgToMan, START_MOVIE);
			Print("TicketTaker %d has stopped taking tickets\n", selfId,-1,-1);
			changeTtState(debugId, selfId, ON_BREAK);
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
	int debugId=1;
	
	end_phase1.name[0]='E';
	end_phase1.name[1]='P';
	end_phase1.name[2]='1';
	end_phase1.name[3]='\0';
	createMv(&end_phase1);

	end_phase2.name[0]='E';
	end_phase2.name[1]='P';
	end_phase2.name[2]='2';
	end_phase2.name[3]='\0';
	createMv(&end_phase2);

	initialize();
	initializeAllValues(debugId); /*intializing entities that shall be part of simulation*/
	initializeAllQueues(debugId);/*initializing all different queues that need to be formed*/
	initializeAllLocks(debugId);/*set up required locks*/
	ttMain(tt_number);
	if( tt_id == 0 )
	{	
		setMv(&end_phase1, 1);
	}	
	if( tt_id == 1 )
	{	
		setMv(&end_phase2, 1);
	}	
	Exit(0);
}
