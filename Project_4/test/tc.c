#include "syscall.h"
#include "movietheature_main.h"

/*extern void ThreadTest_ForPart1();*/
/*extern void SelfTest1_ForPart1();*/

/*struct DebugControlBlock debugCb;*/

int gDebugFileNo=1;
char *printLock;
char *tcNumLock;
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
int tc_id = 0;
char *tcLock[MAX_TC];
char *custLock[MAX_CUST_GRP][MAX_CUST];
char *queueLock[MAX_QUEUE_TYPE][MAX_NUM_QUEUE];
char *queueTcLock;
int tc_number;
char *tcCondVar[MAX_TC];
char *custCondVar[MAX_CUST_GRP][MAX_CUST];
char *queueCondVar[MAX_QUEUE_TYPE][MAX_NUM_QUEUE];
MV tc_num;
char print_lockName[25];

/* Movie Theature control 
 * block
 * */
MtCb mtCb;

/*
 * wrapper functions for
 * monitor variables
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

/*
 * Wrapper for Yield
 * */
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
	if( queue->queueType == TC_QUEUE )
	{
		while(getMv(&mtCb.tc[queue->queueId].msgFromCust) != CUST_REMOVED)
		{ 
			YIELD();
		}
		
		setMv(&mtCb.tc[queue->queueId].msgFromCust, INVALID_MSG);
		currCustGrp = getMv(&mtCb.tc[queue->queueId].currentCustGrpId);
		currCust = getMv(&mtCb.tc[queue->queueId].currentCustId);
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
	/*Print("tc Going to acquire lock queue type=%d id=%d lockName=%s\n",queueType, queueId, queueLock[queueType][queueId] );*/
	if(queueType == TC_QUEUE)  
	{ /*associating ticket clerk with queue*/

		mtCb.tc[id].queue = &mtCb.queue[queueType][queueId];
		mtCb.tc[id].queue->queueType = queueType;
		mtCb.tc[id].queue->queueAddress = (Employee)&mtCb.tc[queueId];
	}
	/*Print("tc Going to release lock queue type=%d id=%d lockName=%s\n",queueType, queueId, queueLock[queueType][queueId] );*/
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

	mtCb.queue[TC_QUEUE][tc_number].queueId = tc_number;
	mtCb.queue[TC_QUEUE][tc_number].queueType = TC_QUEUE;
	initializeQueue(debugId, &mtCb.queue[TC_QUEUE][tc_number]);

	/*Print("tc lock b4 created queuelock=%s type=%d id=%d\n", queueLock[TC_QUEUE][tc_number], TC_QUEUE, tc_number );*/

	queueLock[TC_QUEUE][tc_number] = mtCb.queue[TC_QUEUE][tc_number].name;
	CreateLock(queueLock[TC_QUEUE][tc_number]);

	/*Print("tc lock created queuelock=%s type=%d id=%d\n", queueLock[TC_QUEUE][tc_number], TC_QUEUE, tc_number );*/



	/*Print("tc lock b4 created queuelock=%s type=%d id=%d\n", queueCondVar[TC_QUEUE][tc_number], TC_QUEUE, tc_number );*/
	queueCondVar[TC_QUEUE][tc_number] = mtCb.queue[TC_QUEUE][tc_number].name;
	CreateCV(queueCondVar[TC_QUEUE][tc_number]);
	/*Print("tc lock created queuecondvar=%s type=%d id=%d\n", queueCondVar[TC_QUEUE][tc_number], TC_QUEUE, tc_number );*/

	addAddressToQueue(debugId, tc_number, TC_QUEUE, tc_number);
}

/*
 * this will initialize the locks and condition variables that we created for each queues, manager, ticker clerks, concession clerks, tikcet takers, customers
 * */
void initializeAllLocks(int debugId)
{
	int i,j;
	i=0, j=0; /*char *lockName;*/

	for(i=(tc_number * 2) ; i <= (tc_number * 2) + 1; i++)
	{ /*we start with the customers and create locks and CVs for them*/
		j=0;
		{
			custLock[i][j] = mtCb.custGrp[i].cust[j].name;
			CreateLock(mtCb.custGrp[i].cust[j].name);
			/*Print("tc lock created custlock=%s grp=%d cust=%d\n", custLock[i][j], i, j );*/
			
			custCondVar[i][j]= mtCb.custGrp[i].cust[j].name;
			CreateCV(mtCb.custGrp[i].cust[j].name);
			/*Print("tc lock created condvar=%s grp=%d cust=%d\n", custCondVar[i][j], i, j );*/
		}
	}

	{ /*lock and CV for ticket clerk*/
		tcLock[tc_number] = mtCb.tc[tc_number].name;
		CreateLock(mtCb.tc[tc_number].name);
		/*Print("tc lock created tclock=%s tc=%d \n", tcLock[tc_number], tc_number, -1 );*/
		
		tcCondVar[tc_number] = mtCb.tc[tc_number].name;
		CreateCV(mtCb.tc[tc_number].name);
		/*Print("tc lock created condvar=%s tc=%d \n", tcCondVar[tc_number], tc_number, -1 );*/
	}
}

/*
 * we'll be initializing all the values
 * , assign ids to the identities and
 * and allocating memory to these.
 * */
void initializeAllValues(int debugId)
{
	int i=0, j=0;

	int ID;
	ID = GetId();
	/*tc_id = (tc_id - 2) % 16;*/
	tc_id = (ID - 2) / 16 ;

	tc_number = tc_id;




	copy_string(mtCb.queue[TC_QUEUE][tc_number].name, 20,"QQ", 2, 0, tc_number);
	copy_string(mtCb.tc[tc_number].name, 20,"TC", 2, -1, tc_number);

	mtCb.tc[tc_number].money.name[0] = 'M';
	mtCb.tc[tc_number].money.name[1] = 'T';
	mtCb.tc[tc_number].money.name[2] = 'C';
	mtCb.tc[tc_number].money.name[3] = '0' + tc_number;
	mtCb.tc[tc_number].money.name[4] = '\0';
	mtCb.tc[tc_number].state.name[0] = 'S';
	mtCb.tc[tc_number].state.name[1] = 'T';
	mtCb.tc[tc_number].state.name[2] = 'C';
	mtCb.tc[tc_number].state.name[3] = '0' + tc_number;
	mtCb.tc[tc_number].state.name[4] = '\0';
	mtCb.tc[tc_number].currentCustId.name[0] = 'C';
	mtCb.tc[tc_number].currentCustId.name[1] = 'C';
	mtCb.tc[tc_number].currentCustId.name[2] = 'U';
	mtCb.tc[tc_number].currentCustId.name[3] = '0' + tc_number;
	mtCb.tc[tc_number].currentCustId.name[4] = 'T';
	mtCb.tc[tc_number].currentCustId.name[5] = '\0';
	mtCb.tc[tc_number].currentCustGrpId.name[0] = 'C';
	mtCb.tc[tc_number].currentCustGrpId.name[1] = 'C';
	mtCb.tc[tc_number].currentCustGrpId.name[2] = 'G';
	mtCb.tc[tc_number].currentCustGrpId.name[3] = '0' + tc_number;
	mtCb.tc[tc_number].currentCustGrpId.name[4] = 'T';
	mtCb.tc[tc_number].currentCustGrpId.name[5] = '\0';
	mtCb.tc[tc_number].msgFromCust.name[0] = 'M';
	mtCb.tc[tc_number].msgFromCust.name[1] = 'C';
	mtCb.tc[tc_number].msgFromCust.name[2] = 'U';
	mtCb.tc[tc_number].msgFromCust.name[3] = '0' + tc_number;
	mtCb.tc[tc_number].msgFromCust.name[4] = 'T';
	mtCb.tc[tc_number].msgFromCust.name[5] = '\0';
	mtCb.tc[tc_number].msgBuffer.name[0] = 'M';
	mtCb.tc[tc_number].msgBuffer.name[1] = 'B';
	mtCb.tc[tc_number].msgBuffer.name[2] = '0' + tc_number;
	mtCb.tc[tc_number].msgBuffer.name[3] = 'T';
	mtCb.tc[tc_number].msgBuffer.name[4] = '\0';
	mtCb.tc[tc_number].msgByMan.name[0] = 'M';
	mtCb.tc[tc_number].msgByMan.name[1] = 'B';
	mtCb.tc[tc_number].msgByMan.name[2] = 'M';
	mtCb.tc[tc_number].msgByMan.name[3] = '0' + tc_number;
	mtCb.tc[tc_number].msgByMan.name[4] = 'T';
	mtCb.tc[tc_number].msgByMan.name[5] = '\0';
	createMv(&mtCb.tc[tc_number].state);
	createMv(&mtCb.tc[tc_number].money);
	createMv(&mtCb.tc[tc_number].currentCustId);
	createMv(&mtCb.tc[tc_number].currentCustGrpId);
	createMv(&mtCb.tc[tc_number].msgFromCust);
	createMv(&mtCb.tc[tc_number].msgByMan);
	createMv(&mtCb.tc[tc_number].msgBuffer);

	setMv(&mtCb.tc[tc_number].msgByMan, INVALID_MSG);
	setMv(&mtCb.tc[tc_number].money, 0);
	setMv(&mtCb.tc[tc_number].msgByMan, INVALID_MSG);
	setMv(&mtCb.tc[tc_number].currentCustId, -1);
	setMv(&mtCb.tc[tc_number].currentCustGrpId, -1);
	setMv(&mtCb.tc[tc_number].msgFromCust, INVALID_MSG);
	setMv(&mtCb.tc[tc_number].msgBuffer, INVALID_MSG);

	mtCb.numCustGrp= tc_id * 2;	
	i = ( tc_id * 2 )+1;
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
		setMv(&mtCb.custGrp[mtCb.numCustGrp].location, START);
		mtCb.custGrp[mtCb.numCustGrp].state.name[0] = 'S';
		mtCb.custGrp[mtCb.numCustGrp].state.name[1] = 'C';
		mtCb.custGrp[mtCb.numCustGrp].state.name[2] = 'G';
		mtCb.custGrp[mtCb.numCustGrp].state.name[3] = '0' + mtCb.custGrp[mtCb.numCustGrp].grpId;
		mtCb.custGrp[mtCb.numCustGrp].state.name[4] = '\0';
		createMv(&mtCb.custGrp[mtCb.numCustGrp].state);


		j=0;
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
 * the ticket clerk's main program
 * */
void tcMain(int selfId)
{
	int debugId;
	int ret_val=0;
	int i=0;
	TC *self;
	int currentState;
	int locCustGrp;

	debugId = selfId+200;
	mtCb.tc[selfId].queueType = TC_QUEUE;
	mtCb.tc[selfId].queue = &mtCb.queue[TC_QUEUE][selfId];
	mtCb.tc[i].location = TICKET_CLERK;

	self = &mtCb.tc[selfId]; /*set up a pointer to itself*/

	currentState = getTcState(debugId, selfId); /*get the current state*/
	addAddressToQueue(debugId, selfId, TC_QUEUE, selfId);/*we associate a ticket clerk's queue with this ticket clerk*/
	setMv(&mtCb.queue[TC_QUEUE][selfId].state, NO_CUSTOMER);
	changeTcState(debugId, selfId, FREE_AND_SELLING_TICKET_BEING_FIRST);/*he is now ready to sell*/


	acquireTcLock(debugId, selfId);/*he'll acquire the lock for itsel*/

	while(1)
	{
	
		currentState = getTcState(debugId, selfId);
		if( currentState == FREE_AND_SELLING_TICKET ||
				currentState == FREE_AND_SELLING_TICKET_BEING_FIRST )
		{/*if tickets are currently being sold*/
			if(getTotalCustCount(debugId, &mtCb.queue[TC_QUEUE][selfId]) == 0 )
			{/*there aint nobody in line*/
				Print("TicketClerk %d has no one in line. I am available for a customer.\n", selfId, -1,-1 );
				changeTcState(debugId, selfId, FREE_AND_SELLING_TICKET_BEING_FIRST);/*ready for a new queue of customers*/
			}	
			else
			{	
				Print("TicketClerk %d is coming off break.\n", selfId, -1,-1);
				self->currentCust = queueRemoveCust(debugId, &mtCb.queue[TC_QUEUE][selfId]);/*we try and see if there's anyone in line*/
				if(self->currentCust != NULL)
				{/*we take out first customer from queue*/
					while(getCustState(debugId, self->currentCust->selfId, self->currentCust->grpId) != WAIT)
						YIELD();
/*until customer and clerk are ready to interact we go on wait*/

					Print("TicketClerk %d has a line length %d and is signalling a customer.\n", selfId, getTotalCustCount(debugId, &mtCb.queue[TC_QUEUE][selfId])+1 , -1);/*ticket clerk is calling up customer*/


					changeCustState(debugId, self->currentCust->selfId, self->currentCust->grpId, SIGNAL); /*customer is now awoken from wait*/
					Print("Customer %d in Group %d is walking up to TicketClerk %d ", self->currentCust->selfId, self->currentCust->grpId, selfId);
					Print("to buy %d tickets\n", mtCb.custGrp[self->currentCust->grpId].numCust,-1,-1);



					setMv(&self->msgBuffer,mtCb.custGrp[self->currentCust->grpId].numCust * PER_TICKET_COST);/*computing cost*/

					Print("TicketClerk %d has an order for %d and the cost is %d\n", selfId, mtCb.custGrp[self->currentCust->grpId].numCust, 30 );


					signalToTcLock(debugId, selfId);/*waking up customer to get him to pay money*/



					waitOnTcLock(debugId, selfId);/*wait for customer to pay*/

					setMv(&self->money, self->money.value +  (5 * PER_TICKET_COST));

					transferTicketFromTo(debugId, self, self->currentCust);/*giving ticket*/

					signalToTcLock(debugId, selfId);
					waitOnTcLock(debugId, selfId);
					Print("TicketClerk %d is going on break.\n", selfId, -1, -1 );
				}
			}	
		}
		/*here finding out is there is any group at tc or not, if there is not any group left at tc, so tc should be returned.*/
		for (i= (tc_id * 2) ; i <= (tc_id * 2) +1 ; i++)
		{
			locCustGrp = getCustGrpLocation(debugId, i);
			if( locCustGrp != TICKET_CLERK && locCustGrp != START );
			else  break;
		}
		if( i == (tc_id * 2) + 2 )
		{
			return;
		}

		/*locCustGrp = getCustGrpLocation(debugId, 0);
		if( locCustGrp != TICKET_CLERK && locCustGrp != START )
		return;
   		*/

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
	initializeAllQueues(debugId);/*initializing all different queues that need to be formed*/
	initializeAllLocks(debugId);/*set up required locks*/
	tcMain(tc_number);/*start the manager*/
	Exit(0);
}
