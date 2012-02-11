#include "syscall.h"
#include "movietheature_main.h"

/*extern void ThreadTest_ForPart1();*/
/*extern void SelfTest1_ForPart1();*/

/*struct DebugControlBlock debugCb;*/

int gDebugFileNo=1;
char * printLock;
int ccNumLock;
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
char * ccLock[MAX_CC];
char * custLock[MAX_CUST_GRP][MAX_CUST];
char * queueLock[MAX_QUEUE_TYPE][MAX_NUM_QUEUE];
int cc_id = 0;

char * ccCondVar[MAX_CC];
char * custCondVar[MAX_CUST_GRP][MAX_CUST];
char * queueCondVar[MAX_QUEUE_TYPE][MAX_NUM_QUEUE];
MV cc_num;
int cc_number;
char print_lockName[25];

/* Movie Theature control 
 * block
 * */
MtCb mtCb;
/*
 * wrapper functions
 * for monitor variables
 * */
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

void YIELD()
{
	Yield();
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

void initialize()
{
	print_lockName[0]='P';
	print_lockName[1]='R';
	print_lockName[2]='\0';
	print_lockName[3]='\0';
	printLock = print_lockName;
	CreateLock(print_lockName);
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
	if( queue->queueType == CC_QUEUE )
	{
		while(getMv(&mtCb.cc[queue->queueId].msgFromCust) != CUST_REMOVED)
		{ 
			YIELD();
		}
		
		setMv(&mtCb.cc[queue->queueId].msgFromCust, INVALID_MSG);
		currCustGrp = getMv(&mtCb.cc[queue->queueId].currentCustGrpId);
		currCust = getMv(&mtCb.cc[queue->queueId].currentCustId);
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
	if(queueType == CC_QUEUE)  
	{ /*associating concession clerk with queue*/

		mtCb.cc[id].queue = &mtCb.queue[queueType][queueId];
		mtCb.cc[id].queue->queueType = queueType;
		mtCb.cc[id].queue->queueAddress = (Employee)&mtCb.cc[queueId];
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
	/*setting up requisite number of concession clerk queues*/
	mtCb.queue[CC_QUEUE][cc_number].queueId = cc_number;
	mtCb.queue[CC_QUEUE][cc_number].queueType = CC_QUEUE;
	initializeQueue(debugId, &mtCb.queue[CC_QUEUE][cc_number]);
	queueLock[CC_QUEUE][cc_number] = mtCb.queue[CC_QUEUE][cc_number].name;
	CreateLock(mtCb.queue[CC_QUEUE][cc_number].name);
	queueCondVar[CC_QUEUE][cc_number] = mtCb.queue[CC_QUEUE][cc_number].name;
	CreateCV(mtCb.queue[CC_QUEUE][cc_number].name);
	addAddressToQueue(debugId, cc_number, CC_QUEUE, cc_number);
}

/*
 * this will initialize the locks and condition variables that we created for each queues, manager, ticker clerks, concession clerks, tikcet takers, customers
 * */
void initializeAllLocks(int debugId)
{
	int i,j;
	i=0, j=0; /*char *lockName;*/

	for(i=(cc_number * 2) ; i <= (cc_number * 2) + 1; i++)
	{ /*we start with the customers and create locks and CVs for them*/
		j=0;
		{
			custLock[i][j]=mtCb.custGrp[i].cust[j].name;
			CreateLock(mtCb.custGrp[i].cust[j].name);
			custCondVar[i][j]=mtCb.custGrp[i].cust[j].name;
			CreateCV(mtCb.custGrp[i].cust[j].name);
		}
	}

	{/* locks and CVs for concession clerks */

		ccLock[cc_number]=mtCb.cc[cc_number].name;
		CreateLock(mtCb.cc[cc_number].name);
		ccCondVar[cc_number]=mtCb.cc[cc_number].name;
		CreateCV(mtCb.cc[cc_number].name);
	}
}

/*
 * we'll be initialising values, 
 * assigning ids to all identities
 * and allocating memory to these.
 * */
void initializeAllValues(int debugId)
{
	int i=0, j=0;
	int ID;
	ID = GetId();
	cc_id = (ID - 13) / 16 ;

	cc_number = cc_id;



	copy_string(mtCb.queue[CC_QUEUE][cc_number].name, 20,"QQ", 2, 1, cc_number);
	copy_string(mtCb.cc[cc_number].name, 20,"CC", 2, -1, cc_number);

	mtCb.cc[cc_number].money.name[0] = 'M';
	mtCb.cc[cc_number].money.name[1] = 'C';
	mtCb.cc[cc_number].money.name[2] = 'C';
	mtCb.cc[cc_number].money.name[3] = '0' + cc_number;
	mtCb.cc[cc_number].money.name[4] = '\0';
	mtCb.cc[cc_number].state.name[0] = 'S';
	mtCb.cc[cc_number].state.name[1] = 'C';
	mtCb.cc[cc_number].state.name[2] = 'C';
	mtCb.cc[cc_number].state.name[3] = '0' + cc_number;
	mtCb.cc[cc_number].state.name[4] = '\0';
	mtCb.cc[cc_number].currentCustId.name[0] = 'C';
	mtCb.cc[cc_number].currentCustId.name[1] = 'C';
	mtCb.cc[cc_number].currentCustId.name[2] = 'U';
	mtCb.cc[cc_number].currentCustId.name[3] = '0' + cc_number;
	mtCb.cc[cc_number].currentCustId.name[4] = 'C';
	mtCb.cc[cc_number].currentCustId.name[5] = '\0';
	mtCb.cc[cc_number].currentCustGrpId.name[0] = 'C';
	mtCb.cc[cc_number].currentCustGrpId.name[1] = 'C';
	mtCb.cc[cc_number].currentCustGrpId.name[2] = 'G';
	mtCb.cc[cc_number].currentCustGrpId.name[3] = '0' + cc_number;
	mtCb.cc[cc_number].currentCustGrpId.name[4] = 'C';
	mtCb.cc[cc_number].currentCustGrpId.name[5] = '\0';
	mtCb.cc[cc_number].msgFromCust.name[0] = 'M';
	mtCb.cc[cc_number].msgFromCust.name[1] = 'C';
	mtCb.cc[cc_number].msgFromCust.name[2] = 'U';
	mtCb.cc[cc_number].msgFromCust.name[3] = '0' + cc_number;
	mtCb.cc[cc_number].msgFromCust.name[4] = 'C';
	mtCb.cc[cc_number].msgFromCust.name[5] = '\0';
	mtCb.cc[cc_number].msgBuffer.name[0] = 'M';
	mtCb.cc[cc_number].msgBuffer.name[1] = 'B';
	mtCb.cc[cc_number].msgBuffer.name[2] = '0' + cc_number;
	mtCb.cc[cc_number].msgBuffer.name[3] = 'C';
	mtCb.cc[cc_number].msgBuffer.name[4] = '\0';
	mtCb.cc[cc_number].msgByMan.name[0] = 'M';
	mtCb.cc[cc_number].msgByMan.name[1] = 'B';
	mtCb.cc[cc_number].msgByMan.name[2] = 'M';
	mtCb.cc[cc_number].msgByMan.name[3] = '0' + cc_number;
	mtCb.cc[cc_number].msgByMan.name[4] = 'C';
	mtCb.cc[cc_number].msgByMan.name[5] = '\0';
	createMv(&mtCb.cc[cc_number].state);
	createMv(&mtCb.cc[cc_number].money);
	createMv(&mtCb.cc[cc_number].currentCustId);
	createMv(&mtCb.cc[cc_number].currentCustGrpId);
	createMv(&mtCb.cc[cc_number].msgFromCust);
	createMv(&mtCb.cc[cc_number].msgByMan);
	createMv(&mtCb.cc[cc_number].msgBuffer);

	setMv(&mtCb.cc[cc_number].msgByMan, INVALID_MSG);
	setMv(&mtCb.cc[cc_number].money, 0);
	setMv(&mtCb.cc[cc_number].msgByMan, INVALID_MSG);
	setMv(&mtCb.cc[cc_number].currentCustId, -1);
	setMv(&mtCb.cc[cc_number].currentCustGrpId, -1);
	setMv(&mtCb.cc[cc_number].msgFromCust, INVALID_MSG);
	setMv(&mtCb.cc[cc_number].msgBuffer, INVALID_MSG);

	num_cust.name[0]='C';
	num_cust.name[1]='U';
	num_cust.name[2]='\0';

	mtCb.numCustGrp= cc_id * 2;	
	i = ( cc_id * 2 )+1;
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
 * the concession clerk's main program
 * */
void ccMain(int selfId)
{
    int debugId;
    int ret_val=0;
    int i=0;
    int currentState;
    int locCustGrp;
    CC *self;
    int popcorns, sodas;
 
    debugId = 0;
    mtCb.cc[selfId].queueType = CC_QUEUE;
    mtCb.cc[selfId].queue = &mtCb.queue[CC_QUEUE][selfId];
    mtCb.cc[i].location = CONCESSION_CLERK;

    self = &mtCb.cc[selfId];/*set up a pointer to itself*/
    currentState = getCcState(debugId, selfId);/*get the current state*/
    addAddressToQueue(debugId, selfId, CC_QUEUE, selfId);/*we associate a concession clerk's queue with this clerk*/
    setMv(&mtCb.queue[CC_QUEUE][selfId].state, NO_CUSTOMER);
    changeCcState(debugId, selfId, FREE_AND_SELLING_FOOD_BEING_FIRST);/*he is now ready to sell*/


    acquireCcLock(debugId, selfId);/*he'll acquire the lock for itself*/

    while(1)
    {
	
        currentState = getCcState(debugId, selfId);
        if( currentState == FREE_AND_SELLING_FOOD ||
                currentState == FREE_AND_SELLING_FOOD_BEING_FIRST )
        {
            if(getTotalCustCount(debugId, &mtCb.queue[CC_QUEUE][selfId]) == 0 )
            {/*there aint nobody in line*/
		    Print("ConcessionClerk %d has no one in line. I am available for a customer.\n", selfId, -1,-1 );
                changeCcState(debugId, selfId, FREE_AND_SELLING_FOOD_BEING_FIRST);/*ready for a new queue of customers*/
            }   
            else
            {   /*there are people in queue*/

                self->currentCust = queueRemoveCust(debugId, &mtCb.queue[CC_QUEUE][selfId]);/*we try and see if there's anyone in line*/
                if(self->currentCust != NULL)
                {/*we take out first customer from queue*/
                    while(getCustState(debugId, self->currentCust->selfId, self->currentCust->grpId) != WAIT)
			    YIELD();
		    Print("ConcessionClerk %d has a line length %d and is signalling a customer.\n", selfId, getTotalCustCount(debugId, &mtCb.queue[CC_QUEUE][selfId])+1,-1 );/*ticket clerk is calling up customer*/
                    changeCustState(debugId, self->currentCust->selfId, self->currentCust->grpId, SIGNAL);


                    waitOnCcLock(debugId, selfId);

		    popcorns =  getMv(&self->currentCust->msgBuffer);
                    setMv(&self->msgBuffer, (popcorns * PER_POPCORN_COST)); /*computing cost for popcorn*/
                              


                    signalToCcLock(debugId, selfId); /*waking up customer to get him to pay money*/
                    waitOnCcLock(debugId, selfId); /*wait for customer to pay*/


		    sodas =  getMv(&self->currentCust->msgBuffer);
                    setMv(&self->msgBuffer, (self->msgBuffer.value + (sodas * PER_SODA_COST)));  /*computing cost for soda*/
                    AcquireLock(printLock);/*PRINT*/
		    Print("ConcessionClerk %d has an order for %d popcorn and %d soda. ", selfId, popcorns, sodas);
		    Print("The cost is %d\n", self->msgBuffer.value,-1,-1);
		    ReleaseLock(printLock);
		    setMv(&self->money, self->money.value +  self->msgBuffer.value);

                    transferFoodFromTo(debugId, self, self->currentCust); /*giving soda and popcorn*/

                    signalToCcLock(debugId, selfId);

                    waitOnCcLock(debugId, selfId);
                    Print("ConcessionClerk %d has been paid for the order.\n", selfId, -1,-1);
                }
            }   
        }

        /*here finding out is there is any group at cc or not, if there is not any group left at cc, so cc should be returned.*/
	for (i= (cc_id * 2) ; i <= (cc_id * 2) +1 ; i++)
        {
		locCustGrp = getCustGrpLocation(debugId, i);
		if( locCustGrp != TICKET_CLERK && locCustGrp != START && locCustGrp != CONCESSION_CLERK );
		else  break;
        }
	if( i == (cc_id * 2) + 2 )
	{
		Print("ConcessionClerk %d is going on break.\n", selfId, -1, -1 );
		return;
	}
  

/*		locCustGrp = getCustGrpLocation(debugId, 0);
		if( locCustGrp != TICKET_CLERK && locCustGrp != START && locCustGrp != CONCESSION_CLERK )
		return;
  */ 

	YIELD();
    }   
}


void main()
{
	int debugId=1;

	initialize();
	initializeAllValues(debugId); /*intializing entities that shall be part of simulation*/
	initializeAllQueues(debugId);/*initializing all different queues that need to be formed*/
	initializeAllLocks(debugId);/*set up required locks*/
	ccMain(cc_number);/*start cc*/
	Exit(0);

}
