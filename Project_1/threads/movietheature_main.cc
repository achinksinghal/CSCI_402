#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <strings.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <stdlib.h>

#include "utility.h"
#include "system.h"
#include "movietheature_main.h"
#include "synch.h"

extern void ThreadTest_ForPart1();
extern void SelfTest1_ForPart1();

struct DebugControlBlock debugCb;

int gDebugFileNo=1;

/*
 * DEBUG LEVEL OPTION
 * change it to maximum 
 * 255 to get 
 * logs.
 * */
int gDebugLevel=0;

/* All The Locks and condition 
 * variables Which Are 
 * Required in the whole simulation
 * */
Lock *manLock;
Lock *ticketTaking;
Lock *tcLock[MAX_TC];
Lock *ttLock[MAX_TT];
Lock *ccLock[MAX_CC];
Lock *seats;
Lock *mtLock;
Lock *custLock[MAX_CUST_GRP][MAX_CUST];
Lock *queueLock[MAX_QUEUE_TYPE][MAX_NUM_QUEUE];
Lock *queueTcLock;
Lock *queueCcLock;
Lock *queueTtLock;

Condition *manCondVar;
Condition *mtCondVar;
Condition *tcCondVar[MAX_TC];
Condition *ttCondVar[MAX_TT];
Condition *ccCondVar[MAX_CC];
Condition *custCondVar[MAX_CUST_GRP][MAX_CUST];
Condition *queueCondVar[MAX_QUEUE_TYPE][MAX_NUM_QUEUE];

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
int mtTest2Called=0;
int mtTest6=0;
int mtTest6Called=0;

/* Movie Theature control 
 * block
 * */
MtCb mtCb;

/*
 * Function is to copy debug log file data into a 
 * new file with unique name which is given in 
 * accordance of time and date   
 * */
void copyFile(int debugId, struct tm *readableTime, struct timeval *currentTime)
{
	char truncatedDebugFileCompleteName[1024], command[2030];
	sprintf(truncatedDebugFileCompleteName,"%s_%.2dII%.2dII%.2d_%.2dII%.2dII%.2dII%.6d\0\0", debugCb.debugFileCompleteName[debugId], readableTime->tm_mon+1, readableTime->tm_mday, readableTime->tm_year, readableTime->tm_hour, readableTime->tm_min, readableTime->tm_sec, currentTime->tv_usec);
	sprintf(command, "scp %s %s\0", debugCb.debugFileCompleteName[debugId], truncatedDebugFileCompleteName );
	system(command);
}

/*
 * Function is to truncate the debug log file if its size get high.
 * */
void truncateDebugFile(int debugId, struct tm *readableTime)
{
	FILE *debugFilePtr;
	debugFilePtr = fopen(debugCb.debugFileCompleteName[debugId],"w");
	fclose(debugFilePtr);
	fseek(debugCb.currentDebugFilePtr[debugId], 0, SEEK_SET);

	fprintf(debugCb.currentDebugFilePtr[debugId],"*************************************************\nThis file is created at: \nMonth||Date||Year %.2d||%.2d||%d \nTime: Hour||Minutes||Seconds %.2d||%.2d||%.2d\n***************************************************\n\n", readableTime->tm_mon + 1, readableTime->tm_mday, readableTime->tm_year, readableTime->tm_hour, readableTime->tm_min, readableTime->tm_sec);
}

/*
 * Function is to get the file size 
 * using fstat system call.
 * */
int file_size(FILE *file)
{
	int fd = fileno(file);
	struct stat fileStat;
	fstat(fd, &fileStat);
	return fileStat.st_size;
}

/*
 * Function is to write the debug log prints
 * in a particular format including date, time, 
 * function name in which it is called, and 
 * also line number at which this is called;
 * in a file which is associated with the called thread.
 * */
void writeDebugPrints(int debugId, char *debugLevel, char *debugFileNo, const char *functionName, int lineNumber, char *format, ... )
{
	va_list debugRawData;
	struct timeval currentTime;
	struct tm readableTime;
	int bufferLen=0;
	char debugBufferData[1024];
	char *debugBufferData1;
	int debugFileSize=0;

	if(debugId==MAX_NUMBER_OF_DEBUG_THREADS && debugId == INVALID_DEBUG_ID)
	{
		return;
	}
	if(debugCb.inUse[debugId] == NOT_IN_USE)
	{
		return;
	}
	va_start(debugRawData, format);

	gettimeofday(&currentTime, 0);
	localtime_r(&currentTime.tv_sec, &readableTime);

	debugFileSize = file_size(debugCb.currentDebugFilePtr[debugId]);

	if(debugFileSize >= debugCb.recommendedFileSize[debugId])
	{
		copyFile(debugId, &readableTime, &currentTime);
		truncateDebugFile(debugId, &readableTime);
	}

	sprintf(debugBufferData, "%s||%s||%s||%d||%.2d-%.2d-%d||%.2d-%.2d-%.2d-%.6d||\0",  debugFileNo, debugLevel, functionName, lineNumber, readableTime.tm_mon+1, readableTime.tm_mday, readableTime.tm_year, readableTime.tm_hour, readableTime.tm_min, readableTime.tm_sec, currentTime.tv_usec);
	bufferLen=strlen(debugBufferData);
	debugBufferData1 = debugBufferData+bufferLen;	
	vsprintf(debugBufferData1, format, debugRawData);

	fprintf(debugCb.currentDebugFilePtr[debugId], debugBufferData);
	fflush(debugCb.currentDebugFilePtr[debugId]);
}

/*
 * Function is to initialize 
 * debug logging for a particular thread.
 * Input debugId should be a unique value.
 * */
int debug_init(const char *file_name, int *debugId)
{
	if(debugId == NULL)
	{ 
		return RETURN_NOK;
	}

	if((*debugId > MAX_NUMBER_OF_DEBUG_THREADS)||(*debugId < MIN_NUMBER_OF_DEBUG_THREADS))
        {
		return RETURN_NOK;
        }

	debugCb.inUse[*debugId] = IN_USE;
	sprintf(debugCb.debugFileCompleteName[*debugId], "%s\0", file_name);
	debugCb.currentDebugFilePtr[*debugId]=fopen(debugCb.debugFileCompleteName[*debugId], "w+");
	debugCb.recommendedFileSize[*debugId] = MAX_FILE_SIZE;

	return RETURN_OK;

}

/*
 * Function is to close the 
 * debug logging for a thread.
 * */
void debug_close(int debugId)
{
	debugCb.inUse[debugId] = NOT_IN_USE;
	strcpy(debugCb.debugFileCompleteName[debugId], "\0\0");
	debugCb.recommendedFileSize[debugId] = MAX_FILE_SIZE;
	fclose(debugCb.currentDebugFilePtr[debugId]);
	debugCb.currentDebugFilePtr[debugId]=NULL;
}

/*
 * Function is to obtain current state of movie technician
 * */
State getMtState(int debugId)
{
	State state;
	mtLock->Acquire();   //get lock before we get it
	state = mtCb.mt->state;
	MAIN_PRINT_DEBUG_WARN("MT state is %s\n", strState[state]);
	mtLock->Release(); //release the lock 
	return state;
}

/*
 * Function is to change state of movie technician
 * */
void changeMtState(int debugId, State state)
{
	mtLock->Acquire(); //acquire a lock before we do it
	MAIN_PRINT_DEBUG_WARN("MT state is changed from %s to %s\n", strState[mtCb.mt->state], strState[state]);
	mtCb.mt->state = state;
	
	if (state == STARTED_BY_MAN) //Mtechnician has been asked to start working 
	{
		MAIN_PRINT_DEBUG_WARN("Manager is now starting Movie Technician\n");
	}
	else if (state == MOVIE_IS_NOT_PLAYING) //Mtechnician is currently working
	{
		MAIN_PRINT_DEBUG_WARN("MT is going on WAIT..\n");
		mtCb.mt->msgToMan = MOVIE_OVER;
		mtCondVar->Wait(mtLock);
		MAIN_PRINT_DEBUG_WARN("after MT waken up from WAIT..\n");
	}
	else if ( state == MOVIE_IS_PLAYING || state == NO_MOVIE_REQD )
	{
		MAIN_PRINT_DEBUG_WARN("Sending MT a SIGNAL..\n");		
		mtCondVar->Signal(mtLock);		
		mtCb.mt->msgToMan = STARTING_MOVIE;
	}
	
	mtLock->Release(); //release the lock 
}

/*
 * Function is to obtain current concession clerk state
 * */
State getCcState(int debugId, int ccId)
{
	State state;
	state = mtCb.cc[ccId]->state;
	MAIN_PRINT_DEBUG_WARN("cc=%d state is %s\n", ccId, strState[state]);

	return state;
}

/*
 * Macro is broadcasting a CV for a concession clerk on lock
 * */
#define broadcastAllCcLock( debugId, ccId1)\
{\
	MAIN_PRINT_DEBUG_WARN("Going to broadcast to cc=%d lock\n", ccId1);\
	ccCondVar[ccId1]->Broadcast(ccLock[ccId1]); \
}\

/*
 * Macro is waiting on CV for a concession clerk on lock
 * */
#define waitOnCcLock( debugId, ccId1)\
{\
	MAIN_PRINT_DEBUG_WARN("Going to wait to cc=%d lock\n", ccId1);\
	ccCondVar[ccId1]->Wait(ccLock[ccId1]); \
	MAIN_PRINT_DEBUG_WARN("After a wait at cc=%d lock\n", ccId1);\
}\

/*
 * Macro is signalling concession clerk on CV with lock
 * */
#define signalToCcLock(debugId, ccId1)\
{\
	MAIN_PRINT_DEBUG_WARN("Going to signal to cc=%d lock\n", ccId1);\
	ccCondVar[ccId1]->Signal(ccLock[ccId1]); \
}\

/*
 * Macro is acquiring concession clerk lock
 * */
#define acquireCcLock(debugId, ccId1)\
{\
	ccLock[ccId1]->Acquire(); \
	MAIN_PRINT_DEBUG_WARN("Acquiring cc=%d lock\n", ccId1);\
}\

/*
 * Macro is releasing concession clerk lock  
 * */
#define releaseCcLock(debugId, ccId1)\
{\
	ccLock[ccId1]->Release(); \
	MAIN_PRINT_DEBUG_WARN("Releasing cc=%d lock\n", ccId1); \
}\

/*
 * Macro is to change state of concession clerk
 * */
#define changeCcState(debugId, ccId1, state1)\
{\
	MAIN_PRINT_DEBUG_WARN("CC=%d state is changed from %s to %s\n", ccId1, strState[mtCb.cc[ccId1]->state], strState[state1]);\
	mtCb.cc[ccId1]->state = state1;\
}


/*
 * Function is to get ticket clerk state.
 * */
State getTcState(int debugId, int tcId)
{
	State state;
	state = mtCb.tc[tcId]->state;
	MAIN_PRINT_DEBUG_WARN("tc=%d state is %s\n", tcId, strState[state]);

	return state;
}

/*
 * broadcasting a CV for a ticket clerk on lock 
 * */
#define broadcastAllTcLock( debugId, tcId1)\
{\
	MAIN_PRINT_DEBUG_WARN("Going to broadcast to tc=%d lock\n", tcId1);\
	tcCondVar[tcId1]->Broadcast(tcLock[tcId1]); \
}\

/*
 * this is enclosed Wait function on CV for a ticket clerk on lock
 * */
#define waitOnTcLock( debugId, tcId1)\
{\
	MAIN_PRINT_DEBUG_WARN("Going to wait to tc=%d lock\n", tcId1);\
	tcCondVar[tcId1]->Wait(tcLock[tcId1]); \
	MAIN_PRINT_DEBUG_WARN("After a wait at tc=%d lock\n", tcId1);\
}\

/*
 * enclosing call to Signal function of CV of ticket clerk on lock
 * */
#define signalToTcLock(debugId, tcId1)\
{\
	MAIN_PRINT_DEBUG_WARN("Going to signal to tc=%d lock\n", tcId1);\
	tcCondVar[tcId1]->Signal(tcLock[tcId1]); \
}\

/*
 * enclosed call to acquire ticket clerks's lock
 * */
#define acquireTcLock(debugId, tcId1)\
{\
	tcLock[tcId1]->Acquire(); \
	MAIN_PRINT_DEBUG_WARN("Acquiring tc=%d lock\n", tcId1);\
}\

/*
 * enclosing a call to release lock of ticket clerk 
 * */
#define releaseTcLock(debugId, tcId1)\
{\
	tcLock[tcId1]->Release(); \
	MAIN_PRINT_DEBUG_WARN("Releasing tc=%d lock\n", tcId1); \
}\

/*
 * enclosing a call to wait on CV of Ticket taker on lock
 * */
void waitOnTtLock(int debugId, int ttId)
{
	MAIN_PRINT_DEBUG_WARN("Going to wait to tt=%d lock\n", ttId);
	ttCondVar[ttId]->Wait(ttLock[ttId]); 
	MAIN_PRINT_DEBUG_WARN("After a wait at tt=%d lock\n", ttId);
	return ;
}

/*
 * enclosing a call to broadcast on CV on lock
 * */
#define broadcastAllTtLock( debugId, ttId1)\
{\
	MAIN_PRINT_DEBUG_WARN("Going to broadcast to tt=%d lock\n", ttId1);\
	ttCondVar[ttId1]->Broadcast(ttLock[ttId1]); \
}\

/*
 * function enclosing a call to signal CV of Ticket taker number
 * */
void signalToTtLock(int debugId, int ttId)
{
	MAIN_PRINT_DEBUG_WARN("Going to signal to tt=%d lock\n", ttId);
	ttCondVar[ttId]->Signal(ttLock[ttId]); 
	return ;
}

/*
 * function enclosing a call to acquire lock of Ticket taker number
 * */
void acquireTtLock(int debugId, int ttId)
{
	ttLock[ttId]->Acquire(); 
	MAIN_PRINT_DEBUG_WARN("Acquiring tt=%d lock\n", ttId);
	return ;
}

/*
 * function enclosing a call to release lock of Ticket taker number
 * */
void releaseTtLock(int debugId, int ttId)
{
	ttLock[ttId]->Release(); 
	MAIN_PRINT_DEBUG_WARN("Releasing tt=%d lock\n", ttId);
	return ;
}

/*
 * obtain ticket clerk current state
 * */
State getTtState(int debugId, int ttId)
{
	State state;
	state = mtCb.tt[ttId]->state;
	MAIN_PRINT_DEBUG_WARN("tt=%d state is %s\n", ttId, strState[state]);
	return state;
}

/*
 * setting new value for state of ticket taker  
 * */
void changeTtState(int debugId, int ttId, State state)
{
	MAIN_PRINT_DEBUG_WARN("TT=%d state is changed from %s to %s\n", ttId, strState[mtCb.tt[ttId]->state], strState[state]);
	mtCb.tt[ttId]->state = state;
}

/*
 * setting new state for ticket clerk
 * */
#define changeTcState(debugId, tcId1, state1)\
{\
	MAIN_PRINT_DEBUG_WARN("TC=%d state is changed from %s to %s\n", tcId1, strState[mtCb.tc[tcId1]->state], strState[state1]);\
	mtCb.tc[tcId1]->state = state1;\
}

/*
 * obtain current state of customer - 
 * We acquire a lock before seeing it 
 * and release lock once we are done
 * */
State getCustState(int debugId, int custId, int grpId)
{
	State state;
	custLock[grpId][custId]->Acquire(); 
	state = mtCb.custGrp[grpId]->cust[custId]->state;
	MAIN_PRINT_DEBUG_WARN("cust=%d grp=%d state is %s\n\0\0", custId, grpId, strState[state]);
	custLock[grpId][custId]->Release(); 
	return state;
}

/*
 * we set customer states here 
 * and call the associated wait 
 * and signal on CV as required
 * */
void changeCustState(int debugId, int custId, int grpId, State state)
{
	custLock[grpId][custId]->Acquire();
	State currentState = mtCb.custGrp[grpId]->cust[custId]->state;
	mtCb.custGrp[grpId]->cust[custId]->state = state;

	MAIN_PRINT_DEBUG_WARN("cust=%d grp=%d state is changed from %s to %s\n",custId, grpId, strState[currentState], strState[state]);

       //reactions to different states
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
			state == SEAT_TAKEN 
	  )
	{
		MAIN_PRINT_DEBUG_WARN("Sending cust=%d grp=%d on WAIT\n", custId, grpId);
		custCondVar[grpId][custId]->Wait(custLock[grpId][custId]);//if the state is set to be some sort of WAIT we call wait on CV
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
		MAIN_PRINT_DEBUG_WARN("Sending cust=%d grp=%d on SIGNAL\n", custId, grpId);
		custCondVar[grpId][custId]->Signal(custLock[grpId][custId]);//we stop waiting and signal customer to wake up
	}
	custLock[grpId][custId]->Release(); 

}

/*
 * obtaining state of entire custmer group
 * */
State getCustGrpState(int debugId, int grpId)
{
	State state;
	state = mtCb.custGrp[grpId]->state;
	MAIN_PRINT_DEBUG_WARN("grp=%d state is %s\n", grpId, strState[state]);
	return state;
}

/*
 * setting location of particular customer
 * */
void changeCustLocation(int debugId, int custId, int grpId, Location location)
{
	MAIN_PRINT_DEBUG_WARN("cust=%d grp=%d location is changed from %d to %d\n",custId, grpId, mtCb.custGrp[grpId]->cust[custId]->location, location);
	mtCb.custGrp[grpId]->cust[custId]->location = location;
}

/*
 * setting a new location for the customer group
 * */
void changeCustGrpLocation(int debugId, int grpId, Location location)
{
	int i=0;
	MAIN_PRINT_DEBUG_WARN("grp=%d location is changed from %d to %d\n",grpId, mtCb.custGrp[grpId]->location, location);
	mtCb.custGrp[grpId]->location = location;
	for(i=0; i < mtCb.custGrp[grpId]->numCust; i++)
	{
		if(location == LOBBY)
		{
			printf("Customer %d in Group %d is in the lobby\n", i, grpId);
		}
		else if(location == MOVIEROOM)
		{
			printf("Customer %d in Group %d is leaving the lobby\n", i, grpId);
		}
		changeCustLocation(debugId, i, grpId, location);//we set location of all individual customers as that of the group
	}
}

/*
 * finding value of location for a customer group
 * */
Location getCustGrpLocation(int debugId, int grpId)
{
	MAIN_PRINT_DEBUG_WARN("grp=%d location is %d\n",grpId, mtCb.custGrp[grpId]->location);
	return mtCb.custGrp[grpId]->location;
}

/*
 * finding vaalue of location for a particular customer
 * */
Location getCustLocation(int debugId, int custId, int grpId)
{
	MAIN_PRINT_DEBUG_WARN("cust=%d grp=%d location is %d\n",custId, grpId, mtCb.custGrp[grpId]->cust[custId]->location);
	return mtCb.custGrp[grpId]->cust[custId]->location;
}

/*
 * we set the state of whole group together here
 * */
void changeCustGrpState(int debugId, int grpId, State state)
{
	int i=0;
	MAIN_PRINT_DEBUG_WARN("grp=%d state is changed from %s to %s\n",grpId, strState[mtCb.custGrp[grpId]->state], strState[state]);
	mtCb.custGrp[grpId]->state = state;
	if(state == WAIT)
	{
	}
	else if(state == SIGNAL)
	{
	}
	else if( state == GOT_TICKET || state == GOT_FOOD || state == TICKET_TAKEN || SEATS_READY )
	{  //once group head gets tickets or food or has ticket taken by ticket taker or find seats, he lets other know - using a signal
		for(i=0; i < mtCb.custGrp[grpId]->numCust; i++)
		{
			if(!mtCb.custGrp[grpId]->cust[i]->IAMTicketBuyer)
			{
				changeCustState(debugId, i, grpId, state);
				custLock[grpId][i]->Acquire();
				MAIN_PRINT_DEBUG_WARN("cust=%d grp=%d state on SIGNAL\n",i, grpId);
				custCondVar[grpId][i]->Signal(custLock[grpId][i]);
				custLock[grpId][i]->Release();
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

	for(i=0; i<mtCb.custGrp[toCust->grpId]->numCust; i++)
	{
		MAIN_PRINT_DEBUG_WARN("TICKET_%d is getting transferred to grp=%d\n", i, toCust->grpId);
		mtCb.custGrp[toCust->grpId]->ticket[i] = (Ticket *)malloc(sizeof(Ticket)); 
		mtCb.custGrp[toCust->grpId]->ticket[i]->roomNum = 1; 
	}
}

/*
 * transferring food to customer and add details to customer
 * */
void transferFoodFromTo(int debugId, CC *fromCc, Cust *toCust)
{

	MAIN_PRINT_DEBUG_WARN("FOOD is getting transferred to grp=%d\n", toCust->grpId);
	mtCb.custGrp[toCust->grpId]->food = (Food *)malloc(sizeof(Food)); 
}

/*
 * creates new List to use as queue 
 * */
void initializeQueue(int debugId, mtQueue *queue)
{
	MAIN_PRINT_DEBUG_VERB("Initializing queue\n");
	queue->queue = new List;
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
	MAIN_PRINT_DEBUG_VERB("queueType=%d queue=%d \n", queue->queueType, queue->queueId);
	queueLock[queue->queueType][queue->queueId]->Acquire();
	if(queue->queueAddress == NULL)
	{
		queueLock[queue->queueType][queue->queueId]->Release();
		return NO_ADDRESS;
	}
	else if( queue->queueType == TC_QUEUE && getTcState(debugId, queue->queueId) == ON_BREAK )
	{
		printf("Customer %d in Group %d sees TicketClerk %d is on break", cust->selfId, cust->grpId, queue->queueId );
		queueLock[queue->queueType][queue->queueId]->Release();
		return NO_ADDRESS;
	}
	else if( queue->queueType == CC_QUEUE && getCcState(debugId, queue->queueId) == ON_BREAK )
	{
		printf("Customer %d in Group %d sees ConcessionClerk %d is on break", cust->selfId, cust->grpId, queue->queueId );
		queueLock[queue->queueType][queue->queueId]->Release();
		return NO_ADDRESS;
	}
	else if( queue->queueType == TT_QUEUE && getTtState(debugId, queue->queueId) == ON_BREAK )
	{
		printf("Customer %d in Group %d sees TicketTaker %d is on break", cust->selfId, cust->grpId, queue->queueId );
		queueLock[queue->queueType][queue->queueId]->Release();
		return NO_ADDRESS;
	}
	MAIN_PRINT_DEBUG_INFO("Adding cust=%d custGrp=%d with queue=%d \n", cust->selfId, cust->grpId, queue->queueId);
	queue->queue->Append((void *)cust);
	queue->numCust++;
	MAIN_PRINT_DEBUG_INFO("cust=%d custGrp=%d added to queue=%d numCust=%d\n", cust->selfId, cust->grpId, queue->queueId,queue->numCust );
	queueLock[queue->queueType][queue->queueId]->Release();
	return ROK;
}

/*
 * removing a customer from a queue
 * */
Cust* queueRemoveCust(int debugId, mtQueue *queue)
{
	//We acquire a lock on the queue and then modify it. A customer is removed and is ready to be served.
	queueLock[queue->queueType][queue->queueId]->Acquire();
	MAIN_PRINT_DEBUG_VERB("Going to remove a cust from the queue\n");
	MAIN_PRINT_DEBUG_INFO("before removing from queue=%d numCust=%d\n", queue->queueId,queue->numCust );
	Cust *cust =(Cust *)queue->queue->Remove();
	if(cust!=NULL)
	{//if there were customers in line, we take them out and reduce count of number of customers in queue. After that, we release lock and return pointer to customer
		queue->numCust--;
		MAIN_PRINT_DEBUG_INFO("after removing from queue=%d numCust=%d\n", queue->queueId,queue->numCust );
		queueLock[queue->queueType][queue->queueId]->Release();
		return cust;
	}
	else 
	{//if there were no customers in queue we simply release lock and return a Null signifying no customers

		queueLock[queue->queueType][queue->queueId]->Release();
		return NULL;
	}
}

/*
 * this function is for linking the workers - tc,cc, and tt- to their queues 
 * */
void addAddressToQueue(int debugId, int id, QueueType queueType, int queueId)
{
	queueLock[queueType][queueId]->Acquire(); 
	MAIN_PRINT_DEBUG_INFO("Adding address to queue=%d\n", queueId);
	if(queueType == TC_QUEUE)  
	{ //associating ticket clerk with queue

		MAIN_PRINT_DEBUG_VERB("Adding address to TC queue\n");
		mtCb.tc[id]->queue = mtCb.queue[queueType][queueId];
		mtCb.tc[id]->queue->queueType = queueType;
		mtCb.tc[id]->queue->queueAddress = (Employee)mtCb.tc[queueId];
	}
	else if(queueType == CC_QUEUE)  
	{ //associating concession clerk with queue

		MAIN_PRINT_DEBUG_VERB("Adding address to CC queue\n");
		mtCb.cc[id]->queue = mtCb.queue[queueType][queueId];
		mtCb.cc[id]->queue->queueType = queueType;
		mtCb.cc[id]->queue->queueAddress = (Employee)mtCb.cc[queueId];
	} 
	else if(queueType == TT_QUEUE)  
	{ //associating ticket taker with queue
		MAIN_PRINT_DEBUG_VERB("Adding address to TT queue\n");
		mtCb.tt[id]->queue = mtCb.queue[queueType][queueId];
		mtCb.tt[id]->queue->queueType = queueType;
		mtCb.tt[id]->queue->queueAddress = (Employee)mtCb.tt[queueId];
	} 
	queueLock[queueType][queueId]->Release(); 
}

/*
 * Opposite to upper one, it is to 
 * de-associating queues from workers
 * */
void removeAddressToQueue(int debugId, int id, QueueType queueType, int queueId)
{
	queueLock[queueType][queueId]->Acquire(); 
	MAIN_PRINT_DEBUG_INFO("Removing address to queue=%d\n", queueId);
	if(queueType == TC_QUEUE)  
	{
		MAIN_PRINT_DEBUG_VERB("Removing address to TC queue\n");
		mtCb.tc[id]->queue->queueType = INVALID_QUEUE;
		mtCb.tc[id]->queue->queueAddress = NULL;
		mtCb.tc[id]->queue = NULL;
	}
	else if(queueType == CC_QUEUE)  
	{ 
		MAIN_PRINT_DEBUG_VERB("Removing address to CC queue\n");
		mtCb.cc[id]->queue->queueType = INVALID_QUEUE;
		mtCb.cc[id]->queue->queueAddress = NULL;
		mtCb.cc[id]->queue = NULL;
	} 
	queueLock[queueType][queueId]->Release(); 
}

/*
 * finding total number of customer in particular queue 
 * */
int getTotalCustCount(int debugId, mtQueue *queue)
{
	queueLock[queue->queueType][queue->queueId]->Acquire();
	MAIN_PRINT_DEBUG_INFO("Returning numCust=%d of queue=%d\n", queue->numCust, queue->queueId);
	int count=queue->numCust;
	queueLock[queue->queueType][queue->queueId]->Release();
	return count ;
}

/*
 * We'll setup all the queues that
 * need to be part of simulation
 * */
void initializeAllQueues(int debugId )
{

	int i=0;
	char *queueName;
	MAIN_PRINT_DEBUG_VERB("Initializing all queues\n");
	for(i=0; i<mtCb.numTC; i++) //setting up requisite number of ticket clerk queues
	{
		mtCb.queue[TC_QUEUE][i] = (mtQueue *)malloc(sizeof(mtQueue));
		initializeQueue(debugId, mtCb.queue[TC_QUEUE][i]);
		mtCb.queue[TC_QUEUE][i]->queueId = i;
		mtCb.queue[TC_QUEUE][i]->queueType = TC_QUEUE;
		queueName = (char *)malloc(sizeof(100));
		sprintf(queueName, "QUEUE_%d_%d\0",TC_QUEUE,i);
		MAIN_PRINT_DEBUG_INFO("Initializing queue %s\n", queueName);
		queueLock[TC_QUEUE][i] = new Lock(queueName);
		queueCondVar[TC_QUEUE][i] = new Condition(queueName);
	}

	for(i=0; i<mtCb.numCC; i++) //setting up requisite number of concession clerk queues
	{
		mtCb.queue[CC_QUEUE][i] = (mtQueue *)malloc(sizeof(mtQueue));
		initializeQueue(debugId, mtCb.queue[CC_QUEUE][i]);
		mtCb.queue[CC_QUEUE][i]->queueId = i;
		mtCb.queue[CC_QUEUE][i]->queueType = CC_QUEUE;
		queueName = (char *)malloc(sizeof(100));
		sprintf(queueName, "QUEUE_%d_%d\0",CC_QUEUE,i);
		MAIN_PRINT_DEBUG_INFO("Initializing queue %s\n", queueName);
		queueLock[CC_QUEUE][i] = new Lock(queueName);
		queueCondVar[CC_QUEUE][i] = new Condition(queueName);
	}

	for(i=0; i<mtCb.numTT; i++) //setting up requisite number of ticket taker queues
	{
		mtCb.queue[TT_QUEUE][i] = (mtQueue *)malloc(sizeof(mtQueue));
		initializeQueue(debugId, mtCb.queue[TT_QUEUE][i]);
		mtCb.queue[TT_QUEUE][i]->queueId = i;
		mtCb.queue[TT_QUEUE][i]->queueType = TT_QUEUE;
		queueName = (char *)malloc(sizeof(100));
		sprintf(queueName, "QUEUE_%d_%d\0",TT_QUEUE,i);
		MAIN_PRINT_DEBUG_INFO("Initializing queue %s\n", queueName);
		queueLock[TT_QUEUE][i] = new Lock(queueName);
		queueCondVar[TT_QUEUE][i] = new Condition(queueName);
	}

}

/*
 * this will initialize the locks and condition variables that we created for each queues, manager, ticker clerks, concession clerks, tikcet takers, customers
 * */
void initializeAllLocks(int debugId)
{
	int i=0, j=0; char *lockName;
	MAIN_PRINT_DEBUG_VERB("Initializing all locks\n");

	for(i=0; i<mtCb.numCustGrp; i++)
	{ //we start with the customers and create locks and CVs for them
		for(j=0; j<mtCb.custGrp[i]->numCust; j++)
		{
			lockName = (char *)malloc(sizeof(100));
			sprintf(lockName, "CUST_%d_%d\0",i,j);
			MAIN_PRINT_DEBUG_INFO("Initializing lock %s\n", lockName);
			custLock[i][j]=new Lock(lockName);
			custCondVar[i][j]=new Condition(lockName);
		}
	}

	for(i=0; i<mtCb.numTC; i++)
	{ // locks and CVs for ticket clerks
		lockName = (char *)malloc(sizeof(100));
		sprintf(lockName, "TC_%d\0",i);
		MAIN_PRINT_DEBUG_INFO("Initializing lock %s\n", lockName);
		tcLock[i]=new Lock(lockName);
		tcCondVar[i]=new Condition(lockName);
	}
	for(i=0; i<mtCb.numCC; i++)
	{// locks and CVs for concession clerks 

		lockName = (char *)malloc(sizeof(100));
		sprintf(lockName, "CC_%d\0",i);
		MAIN_PRINT_DEBUG_INFO("Initializing lock %s\n", lockName);
		ccLock[i]=new Lock(lockName);
		ccCondVar[i]=new Condition(lockName);
	}
	for(i=0; i<mtCb.numTT; i++)
	{	// locks and CVs for ticket takers

		lockName = (char *)malloc(sizeof(100));
		sprintf(lockName, "TT_%d\0",i);
		MAIN_PRINT_DEBUG_INFO("Initializing lock %s\n", lockName);
		ttLock[i]=new Lock(lockName);
		ttCondVar[i]=new Condition(lockName);
	}

//Now initialising individual locks and CVs for:
	//---- manager
	lockName = (char *)malloc(sizeof(100));
	sprintf(lockName, "MANAGER\0");
	MAIN_PRINT_DEBUG_INFO("Initializing lock %s\n", lockName);
	manLock=new Lock(lockName);
	manCondVar=new Condition(lockName);

	//--- seats
	lockName = (char *)malloc(sizeof(100));
	sprintf(lockName, "TTaking\0");
	MAIN_PRINT_DEBUG_INFO("Initializing lock %s\n", lockName);
	ticketTaking=new Lock(lockName);
	//---ticket takers
	lockName = (char *)malloc(sizeof(100));
	sprintf(lockName, "SEATS\0");
	MAIN_PRINT_DEBUG_INFO("Initializing lock %s\n", lockName);
	seats=new Lock(lockName);
	//---movie technician
	lockName = (char *)malloc(sizeof(100));
	sprintf(lockName, "MT\0");
	MAIN_PRINT_DEBUG_INFO("Initializing lock %s\n", lockName);
	mtLock=new Lock(lockName);
	mtCondVar=new Condition(lockName);
}

/*
 * reset the number of tickets received by all ticket takers
 * */
void resetNumOfTicketTaken(int debugId)
{
	ticketTaking->Acquire();
	mtCb.numOfTicketsTaken  = 0;
	MAIN_PRINT_DEBUG_INFO("resetted num of ticket received by tt are %d\n", mtCb.numOfTicketsTaken);
	ticketTaking->Release();
}

/*
 * tickets received by tt in current cycle
 * */
int getNumOfTicketTaken(int debugId)
{
	ticketTaking->Acquire();
	MAIN_PRINT_DEBUG_INFO("num of ticket received by tt are %d\n", mtCb.numOfTicketsTaken);
	int value = mtCb.numOfTicketsTaken;
	ticketTaking->Release();
	return value;
}

/*
 * ticket taker has collected this many tickets
 * */
int increaseNumOfTicketTaken(int debugId, int value)
{
	ticketTaking->Acquire();
	MAIN_PRINT_DEBUG_INFO("current num of tickets received by tt are %d\n", mtCb.numOfTicketsTaken);

	if((mtCb.numOfTicketsTaken + value) <= MAX_SEATS)
	{
		mtCb.numOfTicketsTaken = mtCb.numOfTicketsTaken + value;
		MAIN_PRINT_DEBUG_INFO("after adding, num of tickets received by tt are %d\n", mtCb.numOfTicketsTaken);
		ticketTaking->Release();
		return ROK;
	}
	else
	{
		MAIN_PRINT_DEBUG_INFO("Not much seats are left in movieroom \n");
		ticketTaking->Release();
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
		MAIN_PRINT_DEBUG_INFO("cust=%d custGrp=%d is ticketBuyer\n", custId, grpId);
		return 1;
	}
	else
	{
		MAIN_PRINT_DEBUG_INFO("cust=%d custGrp=%d is NOT ticketBuyer\n", custId, grpId);
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

	printf("Write Number Of Ticket Clerks\n");
	scanf("%d", &mtCb.numTC);
	MAIN_PRINT_DEBUG_INFO("Number of TC=%d\n", mtCb.numTC);
	for(i=0; i<mtCb.numTC; i++)
	{
		mtCb.tc[i] = (TC *)malloc(sizeof(TC));
	}

	printf("Write Number Of Concession Clerks\n");
	scanf("%d", &mtCb.numCC);
	MAIN_PRINT_DEBUG_INFO("Number of CC=%d\n", mtCb.numCC);
	for(i=0; i<mtCb.numCC; i++)
	{
		mtCb.cc[i] = (CC *)malloc(sizeof(CC));
	}

	printf("Write Number Of Ticket Takers\n");
	scanf("%d", &mtCb.numTT);
	MAIN_PRINT_DEBUG_INFO("Number of TT=%d\n", mtCb.numTT);
	for(i=0; i<mtCb.numTT; i++)
	{
		mtCb.tt[i] = (TT *)malloc(sizeof(TT));
	}

	mtCb.mvRoom = (MvRoom *)malloc(sizeof(MvRoom));
	mtCb.mt = (MT *)malloc(sizeof(MT));
	
	printf("Write Total Number Of Customers\n");
	scanf("%d", &mtCb.numCust);

	mtCb.numCustGrp=0;	
	i=mtCb.numCust;
	while(i > 0)
	{
		/*
		Each group can only have upto 5 members.
		 So we generate a random number and 
		allocate that number to first group. 
		Next we reduce the total number of
		customers left to be allocated groups and 
		go back to start of loop to put
		more ppl in other groups.
		*/

		int randomNum = random(); 
		randomNum = (randomNum % 5) + 1;
		if(i <= 5)
		{
			randomNum = i;
		}
		i = i- randomNum;
				
		mtCb.custGrp[mtCb.numCustGrp] = (CustGrp *)malloc(sizeof(CustGrp));
		mtCb.custGrp[mtCb.numCustGrp]->grpId = mtCb.numCustGrp;
		mtCb.custGrp[mtCb.numCustGrp]->location = START;
		mtCb.custGrp[mtCb.numCustGrp]->numCust = randomNum; 
		MAIN_PRINT_DEBUG_INFO("grp=%d number of cust=%d\n", mtCb.numCustGrp, mtCb.custGrp[mtCb.numCustGrp]->numCust);
		for(j=0; j<mtCb.custGrp[mtCb.numCustGrp]->numCust; j++)
		{
			mtCb.custGrp[mtCb.numCustGrp]->cust[j] = (Cust *)malloc(sizeof(Cust));
			mtCb.custGrp[mtCb.numCustGrp]->cust[j]->selfId = j;
			mtCb.custGrp[mtCb.numCustGrp]->cust[j]->grpId = mtCb.numCustGrp;
			mtCb.custGrp[mtCb.numCustGrp]->cust[j]->money = CUST_MONEY;
			mtCb.custGrp[mtCb.numCustGrp]->cust[j]->IAMTicketBuyer = isCustTicketBuyer(debugId, mtCb.numCustGrp,j);
		}
		mtCb.numCustGrp++;

	}

	MAIN_PRINT_DEBUG_INFO("Customer Group decided\n");

//Entities given are being checked here.
	if( 
			mtCb.numTC > 5 ||  mtCb.numTC <1 ||
			mtCb.numCC > 5 ||  mtCb.numCC <1 ||
			mtCb.numTT > 3 ||  mtCb.numTT <1 
	  )
	{
		printf("Number of employees, you entered are higher than the allowable limit. So, the program will now exit.\n", mtCb.numCust);
		exit(0);
	}
	if( 
			mtCb.numCust > 40
	  )
	{
		printf("WARNING: Number of customers are greater than 50. \n\n", mtCb.numCust);
	}


//Printing entities
	printf("Number Of Customers = %d \n", mtCb.numCust);
	printf("Number Of Groups = %d \n", mtCb.numCustGrp);
	printf("Number Of TicketClerks = %d \n", mtCb.numTC);
	printf("Number Of ConcessionClerks = %d \n", mtCb.numCC);
	printf("Number Of TicketTakers = %d \n", mtCb.numTT);
}

/*
 * we use this whenever we need to handle 
 * probablility - basically we generate a 
 * random number if that number is less than 
 * equal to desired percent reqd 
 * we say yes - done
 * */
int checkChances(int debugId, int percentChances)
{
	int randomNum = random(); 
	randomNum = randomNum % 100;
	MAIN_PRINT_DEBUG_INFO("randomNum=%d for chances\n", randomNum);
	if( randomNum <= percentChances )
	{
		MAIN_PRINT_DEBUG_VERB("Chance is there\n");
		return 1;
	}
	else if( randomNum > percentChances )
	{
		MAIN_PRINT_DEBUG_VERB("NO chance\n");
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
	mtCb.man->money = 0;
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
		currentMoney =  mtCb.tc[i]->money;
		
		money[0][i] = currentMoney - money[0][i];
		mtCb.man->money = money[0][i] + mtCb.man->money;
		printf("Manager collected %d from TicketClerk %d\n", money[0][i], i);
		money[0][i] = currentMoney;
	}
	MAIN_PRINT_DEBUG_INFO("manager=$%d from all TCs\n",  mtCb.man->money);
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
		currentMoney =  mtCb.cc[i]->money;
		
		money[1][i] = currentMoney - money[1][i];
		mtCb.man->money = money[1][i] + mtCb.man->money;
		printf("Manager collected %d from ConcessionClerk %d\n", money[1][i], i);
		money[1][i] = currentMoney;
	}
	MAIN_PRINT_DEBUG_INFO("manager=$%d from all CCs\n",  mtCb.man->money);
}

/*
 * function is for management of the TT queue by the Manager
 * */
void queueTtManage(int debugId)
{      
    int i=0, j=0, k=0;
    for( i=0; i<mtCb.numTT; i++)
    {
        int currentCustCount = getTotalCustCount(debugId, mtCb.queue[TT_QUEUE][i]);
        if( currentCustCount < 3 && currentCustCount !=0 && getTtState(debugId, i) != ON_BREAK )
        { //we have fewer than 3 people in line and ticket taker is not on break
            k=0;
            for( j=0; j<mtCb.numTT; j++)
            {
                if(j != i && ( getTtState(debugId, j) == ON_BREAK || mtCb.tt[j]->msgByMan==GO_ON_BREAK))
                    k++;//number of other TTs on break or who've have been asked by manager to go on break
            }
            if(k != (mtCb.numTT - 1) && checkChances(debugId, 20))
            {//if not everyone else is on break, we check chances if this taker can go on break - and, if he can then we...
                MAIN_PRINT_DEBUG_ERR("sending message TT=%d on BREAK as cust count is %d\n",  i, currentCustCount);
                mtCb.tt[i]->msgByMan=GO_ON_BREAK;
                printf("Manager has told TicketTaker %d to go on break.\n", i);
            }
        }
        if( currentCustCount > 5 )
        {//if this employee has more than 5 customers to handlle, we need to get someone off from break
            k=0;
            for( j=0; j<mtCb.numTT; j++)
            {//we are looking for other TTs
                if(j != i && getTtState(debugId, j) == ON_BREAK)
                {//we found one on a break - so we get him off break and wake him up
                    MAIN_PRINT_DEBUG_ERR("sending TT=%d to off its BREAK as cust count at TT=%d is %d\n",  j, i, currentCustCount);
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
		int currentCustCount = getTotalCustCount(debugId, mtCb.queue[TC_QUEUE][i]);
		if( currentCustCount < 3 && currentCustCount !=0 && getTcState(debugId, i) != ON_BREAK )
		{ //we have fewer than 3 people in line and ticket clerk is not on break
			k=0;
			for( j=0; j<mtCb.numTC; j++)
			{
				if(j != i && ( getTcState(debugId, j) == ON_BREAK || mtCb.tc[j]->msgByMan==GO_ON_BREAK))
					k++; //number of other ticket clerks on break or who've have been asked by manager to go on break

			}
			if(k != (mtCb.numTC - 1) && checkChances(debugId, 20))
			{//if not everyone else is on break, we check chances if this clerk can go on break - and, if he can then we...
				MAIN_PRINT_DEBUG_ERR("sending message TC=%d on BREAK as cust count is %d\n",  i, currentCustCount);
				mtCb.tc[i]->msgByMan=GO_ON_BREAK;
				printf("Manager has told TicketClerk %d to go on break.\n", i);
			}
		}
		if( currentCustCount > 5 )
		{//if this employee has more than 5 customers to handlle, we need to get someone off from break
			k=0;
			for( j=0; j<mtCb.numTC; j++)
			{//we are looking for other TCs
				if(j != i && getTcState(debugId, j) == ON_BREAK)
				{ //we found one on a break - so we get him off break and wake him up
					MAIN_PRINT_DEBUG_ERR("sending TC=%d to off its BREAK as cust count at TC=%d is %d\n",  j, i, currentCustCount);
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
        int currentCustCount = getTotalCustCount(debugId, mtCb.queue[CC_QUEUE][i]);
        if( currentCustCount < 3 && currentCustCount !=0 && getCcState(debugId, i) != ON_BREAK )
        { //we have fewer than 3 people in line and concession clerk is not on break
            k=0;
            for( j=0; j<mtCb.numCC; j++)
            {
                if(j != i && ( getCcState(debugId, j) == ON_BREAK || mtCb.cc[j]->msgByMan==GO_ON_BREAK))
                    k++;//number of other TCs on break or who've have been asked by manager to go on break
            }
            if(k != (mtCb.numCC - 1) && checkChances(debugId, 20))
            {//if not everyone else is on break, we check chances if this conc clerk can go on break - and, if he can then we...
                MAIN_PRINT_DEBUG_ERR("sending message CC=%d on BREAK as cust count is %d\n",  i, currentCustCount);
                mtCb.cc[i]->msgByMan=GO_ON_BREAK;
		printf("Manager has told ConcessionClerk %d to go on break.\n", i);
            }
        }
        if( currentCustCount > 5 )
        {//if this employee has more than 5 customers to handlle, we need to get someone off from break
            k=0;
            for( j=0; j<mtCb.numCC; j++)
            {//we are looking for other TCs

                if(j != i && getCcState(debugId, j) == ON_BREAK)
                {
                    MAIN_PRINT_DEBUG_ERR("sending CC=%d to off its BREAK as cust count at CC=%d is %d\n",  j, i, currentCustCount);
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
	MAIN_PRINT_DEBUG_WARN("Transferring Money=%d from cust=%d grp=%d to CC=%d\n",  mtCb.cc[toCcId]->msgBuffer, fromCust->selfId, fromCust->grpId, toCcId);
	
	MAIN_PRINT_DEBUG_INFO("before cust=$%d to CC=$%d\n",  fromCust->money, mtCb.cc[toCcId]->money);
	fromCust->money = fromCust->money - mtCb.cc[toCcId]->msgBuffer;
	mtCb.cc[toCcId]->money = mtCb.cc[toCcId]->money +  mtCb.cc[toCcId]->msgBuffer;
	MAIN_PRINT_DEBUG_INFO("after cust=$%d to CC=$%d\n",  fromCust->money, mtCb.cc[toCcId]->money);
}

/*
 * the concession clerk's main program
 * */
void ccMain(int selfId )
{
    int debugId = selfId+300;
    char debugName[20];
    sprintf(debugName, "CC_%d.log\0", selfId);
    int ret_val=0;
    DEBUG_INIT(((char const *)debugName), &debugId);



    int i=0;
    CC *self = mtCb.cc[selfId];//set up a pointer to itself
    State currentState;
    currentState = getCcState(debugId, selfId);//get the current state
    if( currentState == STARTED_BY_MAN)//clerk has just been started 
    {
        addAddressToQueue(debugId, selfId, CC_QUEUE, selfId);//we associate a concession clerk's queue with this clerk
        changeCcState(debugId, selfId, FREE_AND_SELLING_FOOD_BEING_FIRST);//he is now ready to sell
    }


    acquireCcLock(debugId, selfId);//he'll acquire the lock for itself
    MAIN_PRINT_DEBUG_VERB("cc=%d is going to start its activities\n",  selfId);


    while(1)
    {

        currentState = getCcState(debugId, selfId);
        MAIN_PRINT_DEBUG_VERB("cc=%d at start of its while\n",  selfId);
        if( self->msgByMan == GO_ON_BREAK)
        {//we check if manager asked him to go on break

            changeCcState(debugId, selfId, ON_BREAK);//change state to go on break
	    printf("ConcessionClerk %d is going on break.\n", selfId );

            while(!mtCb.queue[CC_QUEUE][selfId]->queue->IsEmpty())
            {//there are customers in the clerk's queue
                self->currentCust = queueRemoveCust(debugId, mtCb.queue[CC_QUEUE][selfId]);//remove a customer from queue
                if(self->currentCust != NULL)
                {
                     while(getCustState(debugId, self->currentCust->selfId, self->currentCust->grpId) != WAIT)//if customer is not waiting for some CV (and thus, asleep), then we yield thread to take it to back of ready queue
                        self->selfThread->Yield();

                    changeCustState(debugId, self->currentCust->selfId, self->currentCust->grpId, SIGNAL); //if the customer was on wait, he is ready to move on
		    while(getCustState(debugId, self->currentCust->selfId, self->currentCust->grpId) != LOOKING_FOR_CC_QUEUE)
			    self->selfThread->Yield();
                }
                self->currentCust = NULL;//we clear data before beginning new iteration
            }
            broadcastAllCcLock(debugId, selfId);//we'll wake up everyone waiting on that tc's lock
            self->msgByMan = INVALID_MSG; //this is to ensure that the msg is dealt with only once
            waitOnCcLock(debugId, selfId);//we now go to wait for lock
	    printf("ConcessionClerk %d is coming off break.\n", selfId);
            changeCcState(debugId, selfId, FREE_AND_SELLING_FOOD); //back from break and ready to sell again  

  
        }
        if( self->msgByMan == SELL_FOOD )
        {//manager has asked to sell food
            changeCcState(debugId, selfId, FREE_AND_SELLING_FOOD);//set state to selling food
            self->msgByMan = INVALID_MSG;//message taken care of, so removing from memory
        }
        else if( currentState == FREE_AND_SELLING_FOOD ||
                currentState == FREE_AND_SELLING_FOOD_BEING_FIRST )
        {
            if(getTotalCustCount(debugId, mtCb.queue[CC_QUEUE][selfId]) == 0 )
            {//there aint nobody in line

		    printf("ConcessionClerk %d has no one in line. I am available for a customer.\n", selfId );
                changeCcState(debugId, selfId, FREE_AND_SELLING_FOOD_BEING_FIRST);//ready for a new queue of customers
            }   
            else
            {   //there are people in queue

                self->currentCust = queueRemoveCust(debugId, mtCb.queue[CC_QUEUE][selfId]);//we try and see if there's anyone in line
                if(self->currentCust != NULL)
                {//we take out first customer from queue
                    while(getCustState(debugId, self->currentCust->selfId, self->currentCust->grpId) != WAIT)
                        self->selfThread->Yield();

		    printf("ConcessionClerk %d has a line length %d and is signalling a customer.\n", selfId, getTotalCustCount(debugId, mtCb.queue[CC_QUEUE][selfId])+1 );//ticket clerk is calling up customer
                    changeCustState(debugId, self->currentCust->selfId, self->currentCust->grpId, SIGNAL);


                    waitOnCcLock(debugId, selfId);

                    MAIN_PRINT_DEBUG_ERR("num=%d of POPCORN from cust=%d grp=%d.\n", self->currentCust->msgBuffer, self->currentCust->selfId, self->currentCust->grpId);
		    int popcorns =  self->currentCust->msgBuffer;
                    self->msgBuffer = self->currentCust->msgBuffer * PER_POPCORN_COST; //computing cost for popcorn
                              

                    MAIN_PRINT_DEBUG_ERR("popcorn cost=%d cust=%d grp=%d.\n", self->msgBuffer, self->currentCust->selfId, self->currentCust->grpId);
                    MAIN_PRINT_DEBUG_ERR("SIGNALING cust to order SODA cust=%d grp=%d.\n", self->currentCust->selfId, self->currentCust->grpId);

                    signalToCcLock(debugId, selfId); //waking up customer to get him to pay money
                    waitOnCcLock(debugId, selfId); //wait for customer to pay


                    MAIN_PRINT_DEBUG_ERR("num=%d of SODA from cust=%d grp=%d.\n", self->currentCust->msgBuffer, self->currentCust->selfId, self->currentCust->grpId);
		    int sodas =  self->currentCust->msgBuffer;
                    self->msgBuffer = self->msgBuffer + self->currentCust->msgBuffer * PER_SODA_COST;  //computing cost for soda
                    MAIN_PRINT_DEBUG_ERR("total cost=%d placed for cust=%d grp=%d to pay.\n", self->msgBuffer, self->currentCust->selfId, self->currentCust->grpId);
                    printf("ConcessionClerk %d has an order for %d popcorn and %d soda. The cost is %d\n", selfId, popcorns, sodas, self->msgBuffer);
                    MAIN_PRINT_DEBUG_ERR("Going to TRANSFERRING FOOD to cust.\n");

                    transferFoodFromTo(debugId, self, self->currentCust); //giving soda and popcorn

                    MAIN_PRINT_DEBUG_ERR("TRANSFERRED FOOD to cust.\n");
                    signalToCcLock(debugId, selfId);

                    MAIN_PRINT_DEBUG_ERR("Waiting till cust transfers money.\n");
                    waitOnCcLock(debugId, selfId);
                    printf("ConcessionClerk %d has been paid for the order.\n", selfId);
                   


                    MAIN_PRINT_DEBUG_ERR("current customer is free for OTHER TASKS, CC queue is made to BUILD.\n");
					{// Test 2 specific
						if(mtTest2 == 1)
						mtTest2Called++;
					}




                }
            }   
        }

        //here finding out is there is any group at cc or not, if there is not any group left at cc, so cc should be returned.
        for (i=0; i<mtCb.numCustGrp; i++)
        {
            if( getCustGrpLocation(debugId, i) != TICKET_CLERK && getCustGrpLocation(debugId, i) != START && getCustGrpLocation(debugId, i) != CONCESSION_CLERK );
            else  break;
        }
        if( i == mtCb.numCustGrp)
        {
            MAIN_PRINT_DEBUG_ERR("My job is over so I am leaving, bbye...:)\n");
            return;
        }
   
        self->selfThread->Yield();
    }   
}

/*
 * the ticket clerk's main program
 * */
void tcMain(int selfId )
{
	int debugId = selfId+200;
	char debugName[20];
	sprintf(debugName, "TC_%d.log\0", selfId);
	int ret_val=0;
	DEBUG_INIT(((char const *)debugName), &debugId);





	int i=0;
	TC *self = mtCb.tc[selfId]; //set up a pointer to itself

	State currentState;
	currentState = getTcState(debugId, selfId); //get the current state

	if(mtTest4 == 0)
	{
		if( currentState == STARTED_BY_MAN)//clerk has just been started
		{
			addAddressToQueue(debugId, selfId, TC_QUEUE, selfId);//we associate a ticket clerk's queue with this ticket clerk
			changeTcState(debugId, selfId, FREE_AND_SELLING_TICKET_BEING_FIRST);//he is now ready to sell

		}
	}
	acquireTcLock(debugId, selfId);//he'll acquire the lock for itsel
	MAIN_PRINT_DEBUG_VERB("tc=%d is going to start its activities\n",  selfId);

	while(1)
	{

		currentState = getTcState(debugId, selfId);
		MAIN_PRINT_DEBUG_VERB("tc=%d at start of its while\n",  selfId);
		if( self->msgByMan == GO_ON_BREAK)
		{//manager asks the clerk to go on break

			changeTcState(debugId, selfId, ON_BREAK);//change state to go on break
			printf("TicketClerk %d is going on break.\n", selfId );
			if(mtTest4 == 1)
			{ //specific to test case 4
				mtCb.tc[1]->queue = mtCb.queue[TC_QUEUE][1];
				mtCb.tc[1]->queue->queueType = TC_QUEUE;
				mtCb.tc[1]->queue->queueAddress = (Employee)mtCb.tc[1];

				waitOnTcLock(debugId, selfId);
				printf("TicketClerk %d is coming off break.\n", selfId);
				exit(0);

			}
			else
			{
				while(!mtCb.queue[TC_QUEUE][selfId]->queue->IsEmpty())
				{//there are customers in the clerk's queue
					self->currentCust = queueRemoveCust(debugId, mtCb.queue[TC_QUEUE][selfId]); //remove a customer from queue

					if(self->currentCust != NULL)
					{
						while(getCustState(debugId, self->currentCust->selfId, self->currentCust->grpId) != WAIT) //if customer is not waiting for some CV (and thus, asleep), then we yield thread to take it to back of ready queue
							self->selfThread->Yield();
						changeCustState(debugId, self->currentCust->selfId, self->currentCust->grpId, SIGNAL); //if the customer was on wait, he is ready to move on
						while(getCustState(debugId, self->currentCust->selfId, self->currentCust->grpId) != LOOKING_FOR_TC_QUEUE)
							self->selfThread->Yield();
					}
					self->currentCust = NULL;//we clear data before beginning new iteration
				}
				broadcastAllTcLock(debugId, selfId);//we'll wake up everyone waiting on that tc's lock
				self->msgByMan = INVALID_MSG;//this is to ensure that the msg is dealt with only once
				waitOnTcLock(debugId, selfId);//we now go to wait for lock
				printf("TicketClerk %d is coming off break.\n", selfId);
				changeTcState(debugId, selfId, FREE_AND_SELLING_TICKET);    //back from break and ready to sell again
			}
		}
		if( self->msgByMan == SELL_TICKETS )
		{//manager has asked to sell tickets
			changeTcState(debugId, selfId, FREE_AND_SELLING_TICKET);//set state to selling tickets
			self->msgByMan = INVALID_MSG;
		}
		else if( currentState == FREE_AND_SELLING_TICKET ||
				currentState == FREE_AND_SELLING_TICKET_BEING_FIRST )
		{//if tickets are currently being sold
			if(getTotalCustCount(debugId, mtCb.queue[TC_QUEUE][selfId]) == 0 )
			{//there aint nobody in line
				printf("TicketClerk %d has no one in line. I am available for a customer.\n", selfId );
				changeTcState(debugId, selfId, FREE_AND_SELLING_TICKET_BEING_FIRST);//ready for a new queue of customers
			}	
			else
			{	
				self->currentCust = queueRemoveCust(debugId, mtCb.queue[TC_QUEUE][selfId]);//we try and see if there's anyone in line
				if(self->currentCust != NULL)
				{//we take out first customer from queue
					while(getCustState(debugId, self->currentCust->selfId, self->currentCust->grpId) != WAIT)
					self->selfThread->Yield();
//until customer and clerk are ready to interact we go on wait

					printf("TicketClerk %d has a line length %d and is signalling a customer.\n", selfId, getTotalCustCount(debugId, mtCb.queue[TC_QUEUE][selfId])+1 );//ticket clerk is calling up customer


					changeCustState(debugId, self->currentCust->selfId, self->currentCust->grpId, SIGNAL); //customer is now awoken from wait
					printf("Customer %d in Group %d is walking up to TicketClerk %d to buy %d tickets\n", self->currentCust->selfId, self->currentCust->grpId, selfId, mtCb.custGrp[self->currentCust->grpId]->numCust );



					MAIN_PRINT_DEBUG_ERR("num=%d of tickets from cust=%d grp=%d.\n",mtCb.custGrp[self->currentCust->grpId]->numCust, self->currentCust->selfId, self->currentCust->grpId);
					self->msgBuffer = mtCb.custGrp[self->currentCust->grpId]->numCust * PER_TICKET_COST;//computing cost

					printf("TicketClerk %d has an order for %d and the cost is %d\n", selfId, mtCb.custGrp[self->currentCust->grpId]->numCust, self->msgBuffer );


					MAIN_PRINT_DEBUG_ERR("SIGNAL cust to transfer money=%d cust=%d grp=%d.\n", self->msgBuffer, self->currentCust->selfId, self->currentCust->grpId);
					signalToTcLock(debugId, selfId);//waking up customer to get him to pay money



					MAIN_PRINT_DEBUG_ERR("On WAIT till cust transfer money.\n");
					waitOnTcLock(debugId, selfId);//wait for customer to pay



					MAIN_PRINT_DEBUG_ERR("Now, going to TRANSFERING ticket to customer.\n");
					transferTicketFromTo(debugId, self, self->currentCust);//giving ticket


					MAIN_PRINT_DEBUG_ERR("current customer is free for OTHER TASKS, queue is made to BUILD.\n");
					signalToTcLock(debugId, selfId);
					waitOnTcLock(debugId, selfId);


					{// Test 3 specific
						if(mtTest3)
						mtTest3Called++;
					}



				}
			}	
		}

		//here finding out is there is any group at tc or not, if there is not any group left at tc, so tc should be returned.
		for (i=0; i<mtCb.numCustGrp; i++)
		{
			if( getCustGrpLocation(debugId, i) != TICKET_CLERK && getCustGrpLocation(debugId, i) != START );
			else  break;
		}
		if( i == mtCb.numCustGrp)
		{
			MAIN_PRINT_DEBUG_ERR("My job is over so I am leaving, bbye...:)\n");
			return;
		}
	
		self->selfThread->Yield();

	}	
}

/*
 * The Ticket Taker Main Program
 * */
void ttMain(int selfId )
{
	int debugId = selfId+400;
	char debugName[20];
	sprintf(debugName, "TT_%d.log\0", selfId);
	int ret_val=0;
	DEBUG_INIT(((char const *)debugName), &debugId);





	int i=0;
	TT *self = mtCb.tt[selfId]; //set up a pointer to itself
	State currentState;


	currentState = getTtState(debugId, selfId); //get the current state
	if( currentState == STARTED_BY_MAN)//ticket taker has just been started by manage
	{
		addAddressToQueue(debugId, selfId, TT_QUEUE, selfId);;//we associate a ticket taker's queue with this ticket taker
		changeTtState(debugId, selfId, FREE_AND_TAKING_TICKET_BEING_FIRST);
	}


	acquireTtLock(debugId, selfId);//get lock before modifying
	MAIN_PRINT_DEBUG_VERB("tt=%d is going to start its activities\n",  selfId);
	while(1)
	{
		currentState = getTtState(debugId, selfId);
		MAIN_PRINT_DEBUG_VERB("tt=%d at start of its while\n",  selfId);
		if( currentState == MAN_STARTING_MOVIE )
		{ //manager asked for movie to start
			changeTtState(debugId, selfId, ON_BREAK);
			printf("TicketTaker %d is going on break.\n", selfId);

			while(!mtCb.queue[TT_QUEUE][selfId]->queue->IsEmpty())
			{//if there is somebody in queue
				self->currentCust = queueRemoveCust(debugId, mtCb.queue[TT_QUEUE][selfId]);//take someone from queue
				if(self->currentCust != NULL)
				{
					while(getCustState(debugId, self->currentCust->selfId, self->currentCust->grpId) != WAIT)
						self->selfThread->Yield();
					changeCustState(debugId, self->currentCust->selfId, self->currentCust->grpId, SIGNAL); //if the customer was  on wait, he is ready to move on
				}
				self->currentCust = NULL;//we clear data before beginning new iteration
			}
			broadcastAllTtLock(debugId, selfId); //we'll wake up everyone waiting on that tt's lock
			self->msgByMan = INVALID_MSG;//this is to ensure that the msg is dealt with only once
			waitOnTtLock(debugId, selfId);//we now go to wait for lock
			printf("TicketTaker %d is coming off break.\n", selfId);
			changeTtState(debugId, selfId, FREE_AND_TAKING_TICKET);  //back from break to take tickets again 
		}
		else if( self->msgByMan == GO_ON_BREAK)
		{//has been asked to go on break
			self->msgToMan = INVALID_MSG;
			changeTtState(debugId, selfId, ON_BREAK);//change state to being on break
			self->msgByMan = INVALID_MSG;
			printf("TicketTaker %d is going on break.\n", selfId);
			while(!mtCb.queue[TT_QUEUE][selfId]->queue->IsEmpty())
			{//if queue wasnt empty when he was asked to go on a break
				self->currentCust = queueRemoveCust(debugId, mtCb.queue[TT_QUEUE][selfId]);
				if(self->currentCust != NULL)
				{// if there was somebody in queue
					while(getCustState(debugId, self->currentCust->selfId, self->currentCust->grpId) != WAIT)
						self->selfThread->Yield();
					changeCustState(debugId, self->currentCust->selfId, self->currentCust->grpId, SIGNAL); //wake up customer if he was sleeping
				}
				self->currentCust = NULL;
			}
			broadcastAllTtLock(debugId, selfId);//wake up all those who were waiting for the ticket taker's lock
			self->msgByMan = INVALID_MSG;
			waitOnTtLock(debugId, selfId);;//wait for its own lock
			printf("TicketTaker %d is coming off break.\n", selfId);
			changeTtState(debugId, selfId, FREE_AND_TAKING_TICKET);
		}
		else if( self->msgByMan == FILL_CUST )
		{//manager has asked taker to start taking tickets

			changeTtState(debugId, selfId, FREE_AND_TAKING_TICKET);//setting state of ticket taker as ready to take the first ticket
			self->msgByMan = INVALID_MSG;;//message was received - now we dissolve it to ensure it is not taken care of again
		}
		if( currentState == FREE_AND_TAKING_TICKET ||
				currentState == FREE_AND_TAKING_TICKET_BEING_FIRST )
		{//if the taker is currently taking tickets
			if(getTotalCustCount(debugId, mtCb.queue[TT_QUEUE][selfId]) == 0 )
			{//if no one is in queue
				printf("TicketTaker %d has no one in line. I am available for a customer.\n", selfId);
				changeTtState(debugId, selfId, FREE_AND_TAKING_TICKET_BEING_FIRST);// we set state as ready to take first ticket again
			}
			else
			{//there is some one in queue - we take them out
				self->currentCust = queueRemoveCust(debugId, mtCb.queue[TT_QUEUE][selfId]);   
				if(self->currentCust != NULL)
				{
					while(getCustState(debugId, self->currentCust->selfId, self->currentCust->grpId) != WAIT)
						self->selfThread->Yield();

//wait for customers to be in WAIT state before wqking them up to interact with them 
					printf("TicketTaker %d has a line length %d and is signalling a customer.\n", selfId, getTotalCustCount(debugId, mtCb.queue[TT_QUEUE][selfId])+1);


					if( mtCb.custGrp[self->currentCust->grpId]->numCust <= (MAX_SEATS - getNumOfTicketTaken(debugId)) )
					{//movie room has sufficient seats for this group
						self->msgToCust = YES;
					}
					else
					{	//movie room diesnt have sufficient seats for this group
						printf("TicketTaker %d is not allowing the group into the theater. The number of taken tickets is %d and the group size is %d.\n", selfId,  getNumOfTicketTaken(debugId), mtCb.custGrp[self->currentCust->grpId]->numCust);
						self->msgToCust = NO;
					}
					changeCustState(debugId, self->currentCust->selfId, self->currentCust->grpId, SIGNAL);//now we awake the customer who was waiting for tt to check
					signalToTtLock(debugId, selfId);//tt now let others get access to its CV
					waitOnTtLock(debugId, selfId);// it now goes on wait   
				}
			}  
		}
		int groupInLobby=0;
		groupInLobby=0;

		//here finding out if there is any group in the lobby. If there is any then if its size < than the number of seats left in the movieroom, then it shuold be first accomodated. Else movie should be started.
		for (i=0; i<mtCb.numCustGrp; i++)
		{
			if(
					mtCb.custGrp[i]->location == START ||
					mtCb.custGrp[i]->location == TICKET_CLERK ||
					mtCb.custGrp[i]->location == CONCESSION_CLERK ||
					mtCb.custGrp[i]->location == LOBBY
			  )
			{
				groupInLobby=1;//there is someone in lobby
				if( ( mtCb.custGrp[i]->numCust <= (MAX_SEATS - getNumOfTicketTaken(debugId)) ))
				{
					changeTtState(debugId, selfId, FREE_AND_TAKING_TICKET);
					break;
				}
			}
		}
		if(i == mtCb.numCustGrp && groupInLobby==1)
		{//we have checked all groups in lobby and accomodated them if possible
			self->msgToMan = START_MOVIE;
			printf("TicketTaker %d has stopped taking tickets\n", selfId);
			changeTtState(debugId, selfId, MAN_STARTING_MOVIE);

		}

		//here finding out is there is any group in lobby or movieroom, if there is not any group left in lobby and movieroom then tt should exit the theater - his job is over as everyone of customers who bought tickets have shown him tickets.
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
		{//all tickets taken by ticket clerk - everyone's been to movie room
			printf("TicketTaker %d has stopped taking tickets\n", selfId);
			MAIN_PRINT_DEBUG_ERR("My job is over so I am leaving, bbye...:)\n");
			return;
		}

		//here finding out, is there is any group in lobby or not, if there is not any group left in lobby then movie should be started.
		for (i=0; i<mtCb.numCustGrp; i++)
		{
			Location grpLoc = getCustGrpLocation(debugId, i);
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
			printf("TicketTaker %d has stopped taking tickets\n", selfId);
			changeTtState(debugId, selfId, MAN_STARTING_MOVIE);
		}
		self->selfThread->Yield();
	}   
}

/*
 * Movie Technician Main's Program
 * */
void mtMain(int selfId)
{
	int debugId = selfId+500;
	char debugName[20];
	sprintf(debugName, "MT.log\0");
	int ret_val=0;
	DEBUG_INIT(((char const *)debugName), &debugId);

	MT *self = mtCb.mt;//creating pointer to self
	Cust *curentCust;
	State currentState;

	while(1)
	{
		MAIN_PRINT_DEBUG_VERB("MT at start of its while..\n");
		currentState = getMtState(debugId); //get state intially
		if( getMtState(debugId) == STARTED_BY_MAN)
		{//if state is started by manager
			self->selfThread->Yield();
		}
		if(getMtState(debugId) == NO_MOVIE_REQD )
		{ // state is that everyone has seen movie
			MAIN_PRINT_DEBUG_ERR("My job is over so I am leaving, bbye...:)\n");
			return;
		}
		if((getMtState(debugId) == MOVIE_IS_PLAYING) && (mtCb.man->msgToAll == MOVIE_RUNNING))
		{//if a movie is now to be played
			int i=0, j=0, num=0;

			Thread *cust;
			char *threadName;
			State custState;

				//The customers have now been seated
				//Movie is now being in played
				//Play movie by calling Thread->Yield for some random time between 200 and 300 

			int movieDuration = random();
			movieDuration %= 100;
			movieDuration += 200;

			for(i=0; i<MAX_ROWS; i++)//Making sure everyone is seated
			{
				for(j=0; j<MAX_COLS; j++) //These customers have seen the movie they now ought to leave
				{
					Cust *currentCust = (Cust *)mtCb.mvRoom->seat[i][j].cust;
					if( currentCust!=NULL )
					while(getCustState(debugId, currentCust->selfId, currentCust->grpId) != SEAT_TAKEN)
					{
						self->selfThread->Yield();
					}
				}
			}
			printf("The MovieTechnician has started the movie\n");
	
			MAIN_PRINT_DEBUG_VERB("mt movie duration = %d\n", movieDuration);
			for(i=0;i<movieDuration;i++)
			{//the movie is beingplayed by calling thread->yields				
				MAIN_PRINT_DEBUG_ERR("mt time=%d movie duration = %d\n", i, movieDuration);
				currentThread->Yield();
			}	
			printf("The MovieTechnician has ended the movie\n");
			MAIN_PRINT_DEBUG_VERB("movie over\n");
			//The movie is now over.. Have to tell manager
			//The movie is now over so changing back status of ticket owners to READY_TO_LEAVE_MOVIE_ROOM
			for(i=0; i<MAX_ROWS; i++)
			{
				for(j=0; j<MAX_COLS; j++) //These customers have seen the movie they now ought to leave
				{
					Cust *currentCust = (Cust *)mtCb.mvRoom->seat[i][j].cust;
					if( currentCust!=NULL && currentCust->seatTaken == 1 && getCustState(debugId, currentCust->selfId, currentCust->grpId) == SEAT_TAKEN)
					{
						changeCustState(debugId, currentCust->selfId, currentCust->grpId, READY_TO_LEAVE_MOVIE_ROOM);    //customers shall be awoken here - SIGNAL them
					}
				}
			}
			printf("The MovieTechnician has told customers to leave the theater room\n");
			changeMtState(debugId, MOVIE_IS_NOT_PLAYING);

			self->selfThread->Yield();
		}
		self->selfThread->Yield();
	}	
}

/*
 * this is to setup queues for each of ticket taker
 * ,ticket clerk and concession clerk
 * */
int selectAndAddInQueue(int debugId, Cust *self, QueueType queueType)
{
    int queueId=0, numCust=10000;
    int ret=0,num_ele=0;
    int reqdQueueId=-1;
    State tcState, ccState, ttState, state; //we acquire respective locks depending on the queue type we are trying to manage
    if(queueType == TC_QUEUE)
    {
	if(queueTcLock!=NULL) queueTcLock->Acquire();
        state = getTcState(debugId, queueId);
        num_ele=mtCb.numTC;
    }
    else if(queueType == CC_QUEUE)
    {
	if(queueCcLock!=NULL) queueCcLock->Acquire();
        num_ele=mtCb.numCC;
    }
    else if(queueType == TT_QUEUE)
    {
	if(queueTtLock!=NULL) queueTtLock->Acquire();
        num_ele=mtCb.numTT;
    }
    for(queueId=0; queueId < num_ele; queueId++)
    {//we get state of associated clerk whose queue was passed in as parameter to this function
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
        MAIN_PRINT_DEBUG_VERB("checking queueType=%d queue=%d\n", queueType, queueId);
        if(getTotalCustCount(debugId, mtCb.queue[queueType][queueId]) < numCust)
        {
            if(mtCb.queue[queueType][queueId]->queueAddress != NULL)
            {
                MAIN_PRINT_DEBUG_VERB("Address is not null\n");
                if(state != ON_BREAK)
                {//this is to manage if a queue has too many ppl - we get someone off from break
                    MAIN_PRINT_DEBUG_VERB("Not on break\n");
                    numCust = getTotalCustCount(debugId, mtCb.queue[queueType][queueId]);
                    reqdQueueId = queueId;
                }
            }
        }
    }
    MAIN_PRINT_DEBUG_VERB("queueType=%d reqdQueue=%d \n", queueType, reqdQueueId);
    if(reqdQueueId == -1)
    {
	    MAIN_PRINT_DEBUG_VERB("1.releasing queue lock queueType=%d reqdQueue=%d \n", queueType, reqdQueueId);
	    if(queueType == TC_QUEUE)
	    {
		    if(queueTcLock!=NULL) queueTcLock->Release();
	    }
	    else if(queueType == CC_QUEUE)
	    {
		    if(queueCcLock!=NULL) queueCcLock->Release();
	    }
	    else if(queueType == TT_QUEUE)
	    {
		    if(queueTtLock!=NULL) queueTtLock->Release();
	    }
	    return NOK;
    }
    if(queueType == TC_QUEUE)
    {
        tcState = getTcState(debugId, reqdQueueId);
        if(tcState != ON_BREAK)
        {
            ret=queueAddCust(debugId, mtCb.queue[queueType][reqdQueueId], self);//attempting to add customer to queue of queueType
            if(ret != NO_ADDRESS )
            {//there was a line ot get into
                printf("Customer %d in Group %d is getting in TicketClerk line %d\n", self->selfId, self->grpId, reqdQueueId );

                self->queue = mtCb.queue[queueType][reqdQueueId];
		if(queueTcLock != NULL) queueTcLock->Release();
                changeCustState(debugId, self->selfId, self->grpId, WAIT);
                if(getTcState(debugId, reqdQueueId ) == ON_BREAK)
                {//the clerk was on break - so we look for one that wasnt on break
                    self->queue = NULL;
                    changeCustState(debugId, self->selfId, self->grpId, LOOKING_FOR_TC_QUEUE);
                    return NOK;
                }
                acquireTcLock(debugId, reqdQueueId);//we get a lock on that ticket clerk
            }
            else
            {
		    if(queueTcLock != NULL) queueTcLock->Release();
		    return NOK;
            }
        }
        else if(tcState == ON_BREAK)
        {// we see clerk on lock
	    if(queueTcLock != NULL) queueTcLock->Release();// we let go of lock for queue
            printf("Customer %d in Group %d sees TicketClerk %d is on break\n", self->selfId, self->grpId, reqdQueueId );
        }           
    }
    else if(queueType == CC_QUEUE)
    {//now managing concession clerk queue
        ccState = getCcState(debugId, reqdQueueId);
        if(ccState != ON_BREAK)
        {
            ret=queueAddCust(debugId, mtCb.queue[queueType][reqdQueueId], self);
            if(ret != NO_ADDRESS )
            {//there was space in queue
                printf("Customer %d in Group %d is getting in ConcessionClerk line %d\n", self->selfId, self->grpId, reqdQueueId );
                self->queue = mtCb.queue[queueType][reqdQueueId];
		if(queueCcLock != NULL) queueCcLock->Release();
                changeCustState(debugId, self->selfId, self->grpId, WAIT);
                if(getCcState(debugId, reqdQueueId ) == ON_BREAK)
                {	//cc was on break so we look for another
                    self->queue = NULL;
                    changeCustState(debugId, self->selfId, self->grpId, LOOKING_FOR_CC_QUEUE);
                    return NOK;
                }
                acquireCcLock(debugId, reqdQueueId);//we get lock for that cc before we interact with him
            }
            else
            {//nobody there - we let of of our lock
		    if(queueCcLock != NULL) queueCcLock->Release();
		    return NOK;
            }
        }
        else if (ccState == ON_BREAK)
        {
		if(queueCcLock != NULL) queueCcLock->Release();
            printf("Customer %d in Group %d sees ConcessionClerk %d is on break\n", self->selfId, self->grpId, reqdQueueId );
        }
    }
    else if(queueType == TT_QUEUE)
    {//now working with ticker taker's queue
        ttState = getTtState(debugId, reqdQueueId);
        if(ttState != ON_BREAK)
        {
            ret=queueAddCust(debugId, mtCb.queue[queueType][reqdQueueId], self);
            if(ret != NO_ADDRESS )
            {//found a place in ticket taker's line
                printf("Customer %d in Group %d is getting in TicketTaker line %d\n", self->selfId, self->grpId, reqdQueueId );

                self->queue = mtCb.queue[queueType][reqdQueueId];
		if(queueTtLock != NULL) queueTtLock->Release();
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
		    if(queueTtLock != NULL) queueTtLock->Release();
                return NOK;
            }
        }
        else if(ttState == ON_BREAK)
        {
	    if(queueTtLock != NULL) queueTtLock->Release();
            printf("Customer %d in Group %d sees TicketTaker %d is on break\n", self->selfId, self->grpId, reqdQueueId );
        }           
    }
    return ROK;
}


void transferMoneyFromCustToTc(int debugId, Cust *fromCust, int toTcId)
{ //showing that customer is pay for his buys - to ticket clerk
	MAIN_PRINT_DEBUG_WARN("Transferring Money=%d from cust=%d grp=%d to TC=%d\n",  mtCb.tc[toTcId]->msgBuffer, fromCust->selfId, fromCust->grpId, toTcId);
	MAIN_PRINT_DEBUG_INFO("before cust=$%d to TC=$%d\n",  fromCust->money, mtCb.tc[toTcId]->money);
	fromCust->money = fromCust->money - mtCb.tc[toTcId]->msgBuffer;
	mtCb.tc[toTcId]->money = mtCb.tc[toTcId]->money +  mtCb.tc[toTcId]->msgBuffer;
	MAIN_PRINT_DEBUG_INFO("after cust=$%d to TC=$%d\n",  fromCust->money, mtCb.tc[toTcId]->money);
}


int askPopcorn(int debugId, Cust *self)
{//the group head is asking others if they need popcorn
	int i=0;
	int numPopcorn=0;
	for(i=0; i < mtCb.custGrp[self->grpId]->numCust; i++)
	{//there is a 75% probability that popcorn will be needed - this function checkChances will give 1 if that is the case
		mtCb.custGrp[self->grpId]->cust[i]->takePopcorn = checkChances(debugId, 75);
		if(mtCb.custGrp[self->grpId]->cust[i]->takePopcorn)
		{
			numPopcorn++;//total number of popcorn for that group enhanced by 1
		}
	}
	MAIN_PRINT_DEBUG_WARN("cust=%d grp=%d requires popcorn=%d\n",  self->selfId, self->grpId, numPopcorn);
	return numPopcorn;
}

int askSoda(int debugId, Cust *self)
{//the group head is asking others if they need soda
	int i=0;
	int numSoda=0;
	for(i=0; i < mtCb.custGrp[self->grpId]->numCust; i++)
	{
		mtCb.custGrp[self->grpId]->cust[i]->takeSoda = checkChances(debugId, 75);//there is a 75% probability that soda will be needed - this function checkChances will give 1 if that is the case
		if(mtCb.custGrp[self->grpId]->cust[i]->takeSoda)
		{
			numSoda++;//total number of soda for that group enhanced by 1
		}
	}
	MAIN_PRINT_DEBUG_WARN("cust=%d grp=%d requires soda=%d\n",  self->selfId, self->grpId, numSoda);
	return numSoda;
}

/*
 * Customer Main Program
 * */
void custMain(int selfId )
{
	int debugId = selfId+1000;
	char debugName[20];
	sprintf(debugName, "CUST_%d.log\0", debugId);
	int ret_val=0;
	DEBUG_INIT(((char const *)debugName), &debugId);


//initializing some values before beginning interactions
	Cust *self;
	CustGrp *selfGrp;
	int i=selfId / 10;
	int j=selfId % 10;
	int k=0;
	int seatsTaken = 0;
	int seatTook = 0;
	selfId=j;
	self = mtCb.custGrp[i]->cust[j];
	selfGrp = mtCb.custGrp[i];
	
	MAIN_PRINT_DEBUG_ERR("My grpId=%d selfId=%d.\n", self->grpId, selfId);

	while(1)
	{
		if(getCustState(debugId, selfId, self->grpId) == STARTED_BY_MAIN)//customers have been just created
		{
			if(self->IAMTicketBuyer)//if this is the group head
			{	
				printf("Customer %d in Group %d has entered the movie theater.\n", selfId, self->grpId);
				for(i=1; i< mtCb.custGrp[self->grpId]->numCust; i++)
				{   //these are other customers in the group - they all wait for him to get back from ticket clerk
					while(getCustState(debugId, i, self->grpId) != WAIT_FOR_TICKET_BUYER_FROM_TC)
					{ self->selfThread->Yield(); }
				}
				changeCustState(debugId, self->selfId, self->grpId, LOOKING_FOR_TC_QUEUE);//it will now look to enter queue
				changeCustGrpLocation(debugId, self->grpId, TICKET_CLERK);//the group is now at ticket clerk
			}
			else
			{	//this aint group head

				printf("Customer %d of Group %d is waiting for HeadCustomer\n", self->selfId, self->grpId );
				changeCustState(debugId, self->selfId, self->grpId, WAIT_FOR_TICKET_BUYER_FROM_TC);//non group heading customers are waiting for him
			}
		}
		if(getCustState(debugId, self->selfId, self->grpId) == LOOKING_FOR_TC_QUEUE)
		{ //if this thread is waiting for a place in a tc queue (will only happen if this grp head)
			if(mtTest6==1) //specific to test case 6
			{
				printf("Customer %d in Group %d is getting in TicketClerk line 0\n", self->selfId, self->grpId );
				printf("Customer %d in Group %d is leaving TicketClerk 0\n", self->selfId, self->grpId);
				changeCustState(debugId, self->selfId, self->grpId, GOT_TICKET);
				printf("HeadCustomer %d of Group %d has told the group to proceed\n", self->selfId, self->grpId );
				changeCustGrpState(debugId, self->grpId, GOT_TICKET);	
			}
			else
			{
				if(selectAndAddInQueue(debugId, self, TC_QUEUE)==ROK)
				{//found a queue
					if(getCustState(debugId, self->selfId, self->grpId) == SIGNAL)
					{//the clerk signalled this customer

						printf("Customer %d in Group %d in TicketClerk line %d is paying %d for tickets\n", self->selfId, self->grpId, self->queue->queueId, mtCb.tc[self->queue->queueId]->msgBuffer );
						MAIN_PRINT_DEBUG_ERR("going to TRANSFER money to tc=%d.\n", self->queue->queueId);
						transferMoneyFromCustToTc(debugId, self, self->queue->queueId);	
//giving money and taking tickets

						MAIN_PRINT_DEBUG_ERR("SIGNALLING tc=%d to transfer TICKETS as money is transferred.\n", self->queue->queueId);
						signalToTcLock(debugId,  self->queue->queueId);
//get tc off lock on CV by giving him money

						MAIN_PRINT_DEBUG_ERR("WAITING for tc to TRANSFER tickets.\n");
						waitOnTcLock(debugId,  self->queue->queueId);//wait for him to give me tickets
						printf("Customer %d in Group %d is leaving TicketClerk %d\n", self->selfId, self->grpId, self->queue->queueId );
						releaseTcLock(debugId,  self->queue->queueId);//done with ticket clerk so dont need to him lock him up anymore - other customers can use him now
						signalToTcLock(debugId,  self->queue->queueId);



						MAIN_PRINT_DEBUG_ERR("cust GOT TICKETS.\n");
						changeCustState(debugId, self->selfId, self->grpId, GOT_TICKET);//have tickets - let's go

						printf("HeadCustomer %d of Group %d has told the group to proceed\n", self->selfId, self->grpId );
						changeCustGrpState(debugId, self->grpId, GOT_TICKET);	
					}
				}
			}
		}














		//now onto concession clerk
		if(getCustState(debugId, self->selfId, self->grpId) == GOT_TICKET)
		{
			{// Test 3 specific
				if(mtTest3 == 1)
				return;
			}

			if(self->IAMTicketBuyer)
			{   //if this is the group head
				for(i=1; i< mtCb.custGrp[self->grpId]->numCust; i++)
				{//these are other customers in the group - they all wait for him to get back from concession clerk
					while(getCustState(debugId, i, self->grpId) != WAIT_FOR_TICKET_BUYER_FROM_CC)
					{ self->selfThread->Yield(); }
				}
				changeCustState(debugId, self->selfId, self->grpId, LOOKING_FOR_CC_QUEUE);//it will now look to enter queue
				changeCustGrpLocation(debugId, self->grpId, CONCESSION_CLERK);//the group is now at concession clerk
			}
			else
			{//this aint group head
				printf("Customer %d in Group %d has entered the movie theater.\n", selfId, self->grpId);
				printf("Customer %d of Group %d is waiting for HeadCustomer\n", self->selfId, self->grpId );//non group heading customers are waiting for him
				changeCustState(debugId, self->selfId, self->grpId, WAIT_FOR_TICKET_BUYER_FROM_CC);
			}
		}

		if(getCustState(debugId, self->selfId, self->grpId) == LOOKING_FOR_CC_QUEUE)
		{//if this thread is waiting for a place in a cc queue (will only happen if this grp head)
			if(mtTest6==1) //Test 6 specific
			{
				printf("Customer %d in Group %d is getting in ConcessionClerk line 0\n", self->selfId, self->grpId );
				printf("Customer %d in Group %d is leaving ConcessionClerk 0\n", self->selfId, self->grpId );
				changeCustState(debugId, self->selfId, self->grpId, GOT_FOOD);
				printf("HeadCustomer %d of Group %d has told the group to proceed\n", self->selfId, self->grpId );
				changeCustGrpState(debugId, self->grpId, GOT_FOOD);	
			}
			else
			{
				int popcorns = askPopcorn(debugId, self);//asking itself and other grp members how many popcorns the group needs
				int sodas = askSoda(debugId, self);//find gorup's soda requirement
				for(i=0; i< mtCb.custGrp[self->grpId]->numCust; i++)
				{
					if(mtCb.custGrp[self->grpId]->cust[i]->takePopcorn || mtCb.custGrp[self->grpId]->cust[i]->takeSoda)
						printf("Customer %d in Group %d has %d popcorn and %d soda request from a group member\n", self->selfId, self->grpId, mtCb.custGrp[self->grpId]->cust[i]->takePopcorn, mtCb.custGrp[self->grpId]->cust[i]->takeSoda );
				}


				if(selectAndAddInQueue(debugId, self, CC_QUEUE)==ROK)
				{//found a line

					if(getCustState(debugId, self->selfId, self->grpId) == SIGNAL)
					{//the clerk signalled this customer
						self->msgBuffer = popcorns;//noting the number needed
						MAIN_PRINT_DEBUG_ERR("POPCORN order has been placed by cust.\n");

						MAIN_PRINT_DEBUG_ERR("SIGNALLING cc=%d to get POPCORN value saved at cc.\n", self->queue->queueId);
						signalToCcLock(debugId,  self->queue->queueId);

						MAIN_PRINT_DEBUG_ERR("WAITING for cc.\n");
						waitOnCcLock(debugId,  self->queue->queueId);//waiting for opportunity to reach cc






						self->msgBuffer = sodas;//noting requirement
						MAIN_PRINT_DEBUG_ERR("SODA order has been placed by cust.\n");
//will now go and get it form concession clerk
						printf("Customer %d in Group %d is walking up to ConcessionClerk %d to buy %d popcorn and %d soda\n", self->selfId, self->grpId, self->queue->queueId, popcorns, sodas );
						MAIN_PRINT_DEBUG_ERR("SIGNALLING cc=%d to get food.\n", self->queue->queueId);
						signalToCcLock(debugId,  self->queue->queueId);//wake up cc to get him to give food to me

						MAIN_PRINT_DEBUG_ERR("WAITING for cc to ask for money after cc transfers food.\n");
						waitOnCcLock(debugId,  self->queue->queueId);//wait for him to get back to me with food and then give him money





						MAIN_PRINT_DEBUG_ERR("going to TRANSFER money to cc=%d.\n", self->queue->queueId);
						transferMoneyFromCustToCc(debugId, self, self->queue->queueId);   //giving money
						printf("Customer %d in Group %d in ConcessionClerk line %d is paying %d for food\n", self->selfId, self->grpId, self->queue->queueId, mtCb.cc[self->queue->queueId]->msgBuffer );
						printf("Customer %d in Group %d is leaving ConcessionClerk %d\n", self->selfId, self->grpId, self->queue->queueId);

//everything for food is done - let's go

						releaseCcLock(debugId,  self->queue->queueId);
						signalToCcLock(debugId,  self->queue->queueId);



						MAIN_PRINT_DEBUG_ERR("cust GOT FOOD.\n");
						changeCustState(debugId, self->selfId, self->grpId, GOT_FOOD);
						printf("HeadCustomer %d of Group %d has told the group to proceed\n", self->selfId, self->grpId ); //head customer asks others to get  move on

						changeCustGrpState(debugId, self->grpId, GOT_FOOD);   
					}
				}
			}
		}



		if(mtTest2 ==1) //test2 specific
		{
			return;
		}









		if(getCustState(debugId,  self->selfId, self->grpId) == GOT_FOOD)
		{
//food done - now lets try to get into movie room
			if(self->IAMTicketBuyer)
			{//this is group head

				for(i=1; i< mtCb.custGrp[self->grpId]->numCust; i++)
				{//these are other customers in the group - they all wait for him to get back from ticket taker
					while(getCustState(debugId, i, self->grpId) != WAIT_FOR_TICKET_BUYER_FROM_TT)
					{ self->selfThread->Yield(); }
				}
				changeCustGrpLocation(debugId, self->grpId, LOBBY);//evryone is in lobby
				changeCustState(debugId, self->selfId, self->grpId, LOOKING_FOR_TT);//the group head is looking for ticket taker
			}
			else
			{//others are waiting for grp head to get back ticket taker
				printf("Customer %d of Group %d is waiting for HeadCustomer\n", self->selfId, self->grpId );
				changeCustState(debugId, self->selfId, self->grpId, WAIT_FOR_TICKET_BUYER_FROM_TT);
			}
		}

		if(getCustState(debugId,  self->selfId, self->grpId) == LOOKING_FOR_TT)
		{//if this thread is waiting for a place in a tt queue (will only happen if this grp head)
			if(mtTest6==1) //specific to test 6
			{
				printf("Customer %d in Group %d is getting in TicketTaker line 0\n", self->selfId, self->grpId );
				printf("Customer %d in Group %d is leaving TicketTaker 0\n", self->selfId, self->grpId );
				printf("HeadCustomer %d of Group %d has told the group to proceed\n", self->selfId, self->grpId );
				changeCustState(debugId, self->selfId, self->grpId, SEATS_READY);
				changeCustGrpState(debugId, self->grpId, SEATS_READY);
			}
			else
			{
				if(selectAndAddInQueue(debugId, self, TT_QUEUE)==ROK)
				{//found a line
					if(getCustState(debugId, self->selfId, self->grpId) == SIGNAL)
					{    //the ticket taker signalled this customer      

						changeTtState(debugId, self->queue->queueId, BUSY_WITH_CUST); //the tt is now busy with this customer
						mtCb.tt[self->queue->queueId]->currentCust = NULL;
						if(mtCb.tt[self->queue->queueId]->msgToCust == YES)
						{//ticket taker says there is place inside for this group
							if(increaseNumOfTicketTaken(debugId, mtCb.custGrp[self->grpId]->numCust ) == ROK)
							{//add tickets in this group to total tickets with customer
								printf("Customer %d in Group %d is walking upto TicketTaker %d to give %d tickets.\n", self->selfId, self->grpId, self->queue->queueId, mtCb.custGrp[self->grpId]->numCust);
								printf("TicketTaker %d is allowing the group into the theater. The number of tickets taken is %d. \n", self->queue->queueId, mtCb.custGrp[self->grpId]->numCust);
								changeCustGrpLocation(debugId, self->grpId, MOVIEROOM);//the grp will now move to movie room
								signalToTtLock(debugId, self->queue->queueId);
								printf("Customer %d in Group %d is leaving TicketTaker %d\n", self->selfId, self->grpId, self->queue->queueId);// they have now gone past ticket taker
								releaseTtLock(debugId, self->queue->queueId);//release the ticket taker's lock
							}
							else
							{//the taker is not taking tickets - so the grp head releases its locks and moves back to lobby to his other group mates
								signalToTtLock(debugId, self->queue->queueId);
								releaseTtLock(debugId, self->queue->queueId);
								printf("Customer %d in Group %d sees TicketTaker %d is no longer taking tickets. Going to the lobby\n", self->selfId, self->grpId, self->queue->queueId);
								changeCustState(debugId, self->selfId, self->grpId, WAIT_IN_LOBBY_AS_TICKET_NOT_TAKEN_BY_TT);
								changeCustState(debugId, self->selfId, self->grpId, LOOKING_FOR_TT);//he must now look for a working ticket taker again
							}
						}   
						else
						{//ticket taker says sorry no space inside
							signalToTtLock(debugId, self->queue->queueId);
							releaseTtLock(debugId, self->queue->queueId);
							printf("Customer %d in Group %d sees TicketTaker %d is no longer taking tickets. Going to the lobby\n", self->selfId, self->grpId, self->queue->queueId);
							changeCustState(debugId, self->selfId, self->grpId, WAIT_IN_LOBBY_AS_TICKET_NOT_TAKEN_BY_TT);
							changeCustState(debugId, self->selfId, self->grpId, LOOKING_FOR_TT);//he must now look for a working ticket taker again
						}

					}

				}

				//once we make it to movie room

				if(getCustGrpLocation(debugId, self->grpId) == MOVIEROOM)
				{
					changeCustState(debugId, self->selfId, self->grpId, TICKET_TAKEN);//We have tickets!!
					printf("HeadCustomer %d of Group %d has told the group to proceed\n", self->selfId, self->grpId );
					changeCustGrpState(debugId, self->grpId, TICKET_TAKEN); 
				}
				self->selfThread->Yield();//now we wait for others for do their work
				if(mtCb.man->msgToAll == MOVIE_RUNNING && getCustGrpLocation(debugId, self->grpId) == LOBBY)
				{//manager has said that movie is running - ie ticket taker is no longer taking tickets.. but grp is in lobby - so it waits till next movie
					MAIN_PRINT_DEBUG_ERR("As MOVIE is started. so, going to WAIT till manager signal.\n");
					changeCustState(debugId, self->selfId, self->grpId, WAIT_IN_LOBBY_AS_TICKET_NOT_TAKEN_BY_TT);
					changeCustState(debugId, self->selfId, self->grpId, LOOKING_FOR_TT);

				}
				self->selfThread->Yield();
				if(getCustGrpLocation(debugId, self->grpId) == LOBBY && mtCb.custGrp[self->grpId]->numCust > (MAX_SEATS - getNumOfTicketTaken(debugId)))
				{//ticket taker is taking tickets but your grp is too big t fit so you need to wait
					MAIN_PRINT_DEBUG_ERR("As cannot accomodate in room. so, going to WAIT till manager signal.\n");
					changeCustState(debugId, self->selfId, self->grpId, WAIT_IN_LOBBY_AS_TICKET_NOT_TAKEN_BY_TT);
					changeCustState(debugId, self->selfId, self->grpId, LOOKING_FOR_TT);
				}
			}
		}






















		//now lets get seated in movie room
		if(getCustState(debugId,  self->selfId, self->grpId) == TICKET_TAKEN)
		{//we have tickets
			if(self->IAMTicketBuyer)
			{//this is group head
				for(i=1; i< mtCb.custGrp[self->grpId]->numCust; i++)
				{//these are other customers in the group - they all wait for him to tell them everyone's all seated and set
					while(getCustState(debugId, i, self->grpId) != WAIT_FOR_TICKET_BUYER_TILL_SEATS_TAKEN)
					{ self->selfThread->Yield(); }
				}
				changeCustState(debugId, self->selfId, self->grpId, LOOKING_FOR_SEATS);//i am trying to seat my group
			}
			else
			{// i am not grp head - waiting for him to tell us that seats taken
				changeCustState(debugId, self->selfId, self->grpId, WAIT_FOR_TICKET_BUYER_TILL_SEATS_TAKEN);
			}
		}

		if(getCustState(debugId,  self->selfId, self->grpId) == LOOKING_FOR_SEATS)
		{
			seats->Acquire();//get the lock to look for seats
			while(!seatsTaken)//we arent sat yet
			{	//looking for seats in same row
				for(i=0; i<MAX_ROWS; i++)
				{
					if(!seatsTaken)
					for(j=0; j<MAX_COLS; j++)
					{
						if(mtCb.mvRoom->seat[i][j].cust == NULL )
						{//thers's nobody here on this seat yet
							if((j+mtCb.custGrp[self->grpId]->numCust ) > MAX_COLS)
							break;

							for(k=j+1; k<(j+mtCb.custGrp[self->grpId]->numCust); k++)
							{
								if(mtCb.mvRoom->seat[i][k].cust != NULL )
								break;
								
							}
							if(k==(j+mtCb.custGrp[self->grpId]->numCust))
							{//allocating seats
								mtCb.mvRoom->seat[i][j].cust = self; 
								self->seat =  &mtCb.mvRoom->seat[i][j];
								for(k=j+1; k<(j+mtCb.custGrp[self->grpId]->numCust); k++)
								{
									mtCb.mvRoom->seat[i][k].cust = (Cust *)mtCb.custGrp[self->grpId]->cust[k-j]; 
									mtCb.custGrp[self->grpId]->cust[k-j]->seat =  &mtCb.mvRoom->seat[i][k];
									printf("Customer %d in Group %d has found the following seat: row %d and seat %d\n", (k-j), self->grpId, i, k);
								}
								seatsTaken = 1;
							}
							break;
						}
						
					}
				}
				//looking for seats in consecutive rows
				k=0;
				if(!seatsTaken)
				{
					while(k!=mtCb.custGrp[self->grpId]->numCust)
					{
						seatTook =0;
						for(i=0; i<MAX_ROWS; i++)
						{
							for(j=0; j<MAX_COLS; j++)
							{
								if(mtCb.mvRoom->seat[i][j].cust == NULL )
								{ //nobody is sat here - we take the first seat we see empty
									mtCb.mvRoom->seat[i][j].cust = (Cust *)mtCb.custGrp[self->grpId]->cust[k];
									mtCb.custGrp[self->grpId]->cust[k]->seat =  &mtCb.mvRoom->seat[i][j];
									printf("Customer %d in Group %d has found the following seat: row %d and seat %d\n", k, self->grpId, i, j);
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
			seats->Release();
		}
		if(seatsTaken)
		{//people have selected seats

			changeCustState(debugId, self->selfId, self->grpId, SEATS_READY);
			changeCustGrpState(debugId, self->grpId, SEATS_READY);
		}
		if(getCustState(debugId,  self->selfId, self->grpId) == SEATS_READY)
		{
			if(mtTest6==1)// specific to test 6
			{
				if(self->IAMTicketBuyer)
				{
					for(i=1; i< mtCb.custGrp[self->grpId]->numCust; i++)
					{
						while( getCustState(debugId, i, self->grpId) != READY_TO_LEAVE_MOVIE_ROOM )
						{ self->selfThread->Yield(); }
					}
				}
				printf("Customer %d in Group %d is sitting in a theater room seat\n", self->selfId, self->grpId);
				changeCustState(debugId, self->selfId, self->grpId, READY_TO_LEAVE_MOVIE_ROOM);
			}
			else
			{

				self->seat->isTaken = 1;
				self->seatTaken = 1;
				if(self->IAMTicketBuyer)
				{
					for(i=1; i< mtCb.custGrp[self->grpId]->numCust; i++)
					{//wait for evryone in group to sit down
						while( getCustState(debugId, i, self->grpId) != SEAT_TAKEN )
						{ self->selfThread->Yield(); }
					}
				}
				printf("Customer %d in Group %d is sitting in a theater room seat\n", self->selfId, self->grpId);
				changeCustState(debugId, self->selfId, self->grpId, SEAT_TAKEN);
			}
		}





		//time to go after movie
		if((getCustState(debugId,  self->selfId, self->grpId) == READY_TO_LEAVE_MOVIE_ROOM) )
		{
			if(mtTest6==0) //test 6 specific
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
					{ self->selfThread->Yield(); }
				}
			}

			if(self->IAMTicketBuyer)// allowing the group to proceed
			{printf("HeadCustomer %d of Group %d has told the group to proceed\n", self->selfId, self->grpId );}

			printf("Customer %d in Group %d is getting out of a theater room seat\n", self->selfId, self->grpId);
			changeCustState(debugId, self->selfId, self->grpId, LEFT_MOVIE_ROOM_AFTER_MOVIE);
		}

		if(getCustState(debugId,  self->selfId, self->grpId) == LEFT_MOVIE_ROOM_AFTER_MOVIE )
		{
			if(self->IAMTicketBuyer)
			{   //I am grp head - getting evryone out of movie room
				changeCustGrpLocation(debugId, self->grpId, OUT_OF_MOVIEROOM);
				for(i=1; i< mtCb.custGrp[self->grpId]->numCust; i++)
				{  //checking for others
					while(
						getCustState(debugId, i, self->grpId) != HEADING_FOR_BATHROOM &&
						getCustState(debugId, i, self->grpId) != USING_BATHROOM &&
						getCustState(debugId, i, self->grpId) != READY_TO_GO_OUT_OF_MT
					     )
					{ self->selfThread->Yield(); }//while everyone is ready to leave theater, we wait
				}
			}
			changeCustState(debugId, self->selfId, self->grpId, HEADING_FOR_BATHROOM); //movie over, heading to bathroom

		}
		if(getCustState(debugId,  self->selfId, self->grpId) == HEADING_FOR_BATHROOM)
		{//we'll see here if I do get to go
			if(checkChances(debugId, 25))
			{//25% probability that customer will go

				printf("Customer %d in Group %d is going to the bathroom.\n", self->selfId, self->grpId);
				changeCustState(debugId, self->selfId, self->grpId, USING_BATHROOM);

				int useDuration = random();
				useDuration %= 10;
				useDuration += 5;
				//we assume once a peson goes to bathroom, he returns after doing his business in some random time between 5 and 15 thread yields
				MAIN_PRINT_DEBUG_VERB("cust=%d custGrp=%d bathroom use duration = %d\n", self->selfId, self->grpId, useDuration);
				for(i=0; i<useDuration; i++)
				{				
					MAIN_PRINT_DEBUG_ERR("using bathroom time=%d duration=%d \n", i, useDuration);
					currentThread->Yield();
				}	
				printf("Customer %d in Group %d is leaving the bathroom.\n", self->selfId, self->grpId);
			}
			changeCustState(debugId, self->selfId, self->grpId, READY_TO_GO_OUT_OF_MT);
		}
		if(getCustState(debugId,  self->selfId, self->grpId) == READY_TO_GO_OUT_OF_MT)
		{ //ready to go out of theater
			printf("Customer %d in Group %d is in the lobby\n", self->selfId, self->grpId);
			for(i=0; i< mtCb.custGrp[self->grpId]->numCust; i++)
			{  //checking if people in my group needed tto use bathroom or not
				while(
						getCustState(debugId, i, self->grpId) != CUSTOMER_SIMULATION_COMPLETED &&
						getCustState(debugId, i, self->grpId) != READY_TO_GO_OUT_OF_MT
				     )
				{ self->selfThread->Yield(); }//we wait for everyone to leave theater and simulation completes
			}
			printf("Customer %d in Group %d is leaving the lobby\n", self->selfId, self->grpId);

			MAIN_PRINT_DEBUG_ERR("As there is no one in the custGrp=%d using bathroom so, I am going ou of movieroom \n", self->grpId);
			printf("Customer %d in Group %d has left the movie theater.\n", selfId, self->grpId);
			changeCustState(debugId, self->selfId, self->grpId, CUSTOMER_SIMULATION_COMPLETED);//once everyone has left theater - the simulation is ove
			if(self->IAMTicketBuyer)
			{ //the grp head waits for everyone to leave theater and simulation ending
				for(i=1; i< mtCb.custGrp[self->grpId]->numCust; i++)
				{
					while(
							getCustState(debugId, i, self->grpId) != CUSTOMER_SIMULATION_COMPLETED
					     )
					{ self->selfThread->Yield(); }
				}
				changeCustGrpLocation(debugId, self->grpId, OUT_OF_MOVIETHEATURE);
			}
			mtTest6Called++;
			return;
		}	

		self->selfThread->Yield();
	}
}

/*
 * Starting Customer Threads
 * */
void startCustThreads(int debugId )
{
	int i=0, j=0, num=0;
	Thread *cust;
	char *threadName;

	for(i=0; i<mtCb.numCustGrp; i++)
	{  //we have i groups and j people in i group
		for(j=0; j<mtCb.custGrp[i]->numCust; j++)
		{
			threadName = (char *)malloc(sizeof(100));
			sprintf(threadName, "CUST_%d_%d\0",i,j);
			mtCb.custGrp[i]->cust[j]->selfThread = new Thread(threadName);
			changeCustState(debugId, j, i, STARTED_BY_MAIN);
			MAIN_PRINT_DEBUG_VERB("cust=%d grpId=%d is to be started\n", j,i);
			num = (10*i)+j;
			mtCb.custGrp[i]->cust[j]->selfThread->Fork((VoidFunctionPtr)custMain, num);
		}
	}
}

int SeatsTaken(int debugId)//This function will be used to know how many seats are available at a time
{
	int i=0,seatsCount=0;
	int j=0;
	for (i = 0; i < MAX_ROWS; i++)//rows in movie room
	{	
		for (j=0; j<MAX_COLS ; j++)//cols in movie room

		{
			if(mtCb.mvRoom->seat[i][j].cust != NULL && mtCb.mvRoom->seat[i][j].isTaken == 1) 
			{
				Cust *cust = (Cust *) mtCb.mvRoom->seat[i][j].cust;
				MAIN_PRINT_DEBUG_VERB("cust=%d custGrp=%d has occupied seat[%d][%d]\n",cust->selfId, cust->grpId, i, j);
				seatsCount++; 
			}

		}
	}
	MAIN_PRINT_DEBUG_INFO("num of seats taken were %d\n", seatsCount);
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
		if( mtCb.tt[i]->msgToMan == START_MOVIE )
		{//once movie is begun, manager asks ticket takers to go on break
			mtCb.tt[i]->msgByMan = GO_ON_BREAK;
			printf("Manager has told TicketTaker %d to go on break.\n", i);
			mtCb.man->msgToAll = MOVIE_RUNNING;
			mtCb.tt[i]->msgToMan = INVALID_MSG;
			MAIN_PRINT_DEBUG_ERR("As movieroom is filled. so broadcasting MOVIE_RUNNING msg.\n");
		}
	}

	if (mtCb.man->msgToAll == MOVIE_RUNNING)
	{//manager tells everyone that movie is running
		for(i=0; i<mtCb.numTT; i++)
		{
			while(getTtState(debugId, i) != ON_BREAK)
			{ //the ticket taker is working but there is no customer at him
				if( ( getTtState(debugId, i) == FREE_AND_TAKING_TICKET ||
							getTtState(debugId, i) == FREE_AND_TAKING_TICKET_BEING_FIRST) && mtCb.tt[i]->currentCust == NULL)
				{
					acquireTtLock(debugId, i);//aquire its loc
					signalToTtLock(debugId, i);//signal it to wake up
					releaseTtLock(debugId, i);//let go of lock
				}
				mtCb.man->selfThread->Yield();//manager yields to let others run
			}
		}

		if (getMtState(debugId) == MOVIE_IS_NOT_PLAYING )//We havent checked here if all 25 have to be in
		{	
			while( !IsSeatsOccupied && SeatsTaken(debugId) != 0) //seats taken by custs should be equal to zero b4 next movie starts
			{ 
				mtCb.man->selfThread->Yield(); //manager waits for seats to be taken
			}
			for (i=0; i<mtCb.numCustGrp; i++)
			{//checking if everyone has seen movie
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
				MAIN_PRINT_DEBUG_ERR("Changing MT state to NO_MOVIE_REQD, as all custs have seen movie.\n");
				changeMtState(debugId, NO_MOVIE_REQD);
				IsSeatsOccupied = 1;
				return;
			}

			if(!IsSeatsOccupied)
			{//seats are taken so ticket takers list of tickets taken is to be completed
				resetNumOfTicketTaken(debugId);
				mtCb.man->msgToAll = MOVIE_NOT_RUNNING;
			//sending msg to the Ticket Taker to take tickets and fill seats	
				for(i=0; i<mtCb.numTT; i++)
				{//we set ticket taker to initial values
					acquireTtLock(debugId, i);
					mtCb.tt[i]->msgByMan = FILL_CUST;
					mtCb.tt[i]->msgToMan = INVALID_MSG;
					mtCb.tt[i]->currentCust = NULL;
					signalToTtLock(debugId, i);

					MAIN_PRINT_DEBUG_ERR("As movie is completed, so refilling custs.\n");
					releaseTtLock(debugId, i);
				}
			}
			IsSeatsOccupied = 1;
			if(mtCb.man->msgToAll == MOVIE_NOT_RUNNING)
			{  //if movie is not running, tickets can be given to ticket taker
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
	//We havent checked here if all 25 have to be in
	if (mtCb.man->msgToAll == MOVIE_RUNNING)
	{
		int sTaken = SeatsTaken(debugId);
		int numTcktTaken = getNumOfTicketTaken(debugId);
		if(sTaken == numTcktTaken && sTaken != 0 && (getMtState(debugId) == MOVIE_IS_NOT_PLAYING || getMtState(debugId) == STARTED_BY_MAN))
		{ //we see if seats have been taken
			MAIN_PRINT_DEBUG_ERR("movie now can be started.\n");
			//Ask the Technician to start movie;
			mtCb.mt->msgByMan = START_MOVIE;
			printf("Manager is telling the MovieTechnician to start the movie\n");
			changeMtState(debugId, MOVIE_IS_PLAYING);	
			IsSeatsOccupied = 0;
		}

	}
}

/*
 * Manager Main's Program
 * */
void manMain( int a )
{
	int debugId = 2;
	int ret_val=0;
	DEBUG_INIT("manager.log", &debugId);
	IsSeatsOccupied = 0;

	int i=0, j=0;
	Manager *self = mtCb.man;
	Thread *tcThread, *ccThread, *ttThread, *mtThread;
	char *threadName;

	MAIN_PRINT_DEBUG_VERB("num tc=%d and num cc=%d\n", mtCb.numTC, mtCb.numCC);
	mtCb.man->msgToAll = MOVIE_NOT_RUNNING; //we intialize states of all workers to be MOVIE_NOT_RUNNING

	for(i=0; i<mtCb.numTC; i++)
	{//the manager will start threads of the ticket clerks
		MAIN_PRINT_DEBUG_VERB("going to open tc=%d\n", i);
		threadName = (char *)malloc(sizeof(100));
		sprintf(threadName, "TC_%d\0",i);
		tcThread = new Thread(threadName);//intialising the values for the ticket clerks
		mtCb.tc[i]->selfThread = tcThread;
		mtCb.tc[i]->queueType = TC_QUEUE; 
		mtCb.tc[i]->queue = mtCb.queue[TC_QUEUE][i];
		mtCb.tc[i]->location = TICKET_CLERK;
		if(mtTest2 == 0)
		{
			changeTcState(debugId, i, STARTED_BY_MAN);
		}
		mtCb.tc[i]->selfThread->Fork(tcMain, i); //the ticket clerk is now beginning to work
		if(mtTest4 == 1) break; //test 4 specific
	}
	MAIN_PRINT_DEBUG_VERB("open for TC\n");
	if(mtTest4 == 0)  //if test 4  not running as these are not required in test 4.
	{
		for(i=0; i<mtCb.numCC; i++)
		{	//the manager will now start concession clerks
			MAIN_PRINT_DEBUG_VERB("going to open cc=%d\n", i);
			threadName = (char *)malloc(sizeof(100));
			sprintf(threadName, "CC_%d\0",i);
			ccThread = new Thread(threadName);//intialising the values for the concession clerk
			mtCb.cc[i]->selfThread = ccThread;
			mtCb.cc[i]->queueType = CC_QUEUE; 
			mtCb.cc[i]->queue = mtCb.queue[CC_QUEUE][i];
			mtCb.cc[i]->location = CONCESSION_CLERK;
			if(mtTest2 == 0)
			{
				changeCcState(debugId, i, STARTED_BY_MAN);
			}
			mtCb.cc[i]->selfThread->Fork(ccMain, i); //the concession clerk is now beginning to work
		}
		MAIN_PRINT_DEBUG_VERB("open for CC\n");
		if(mtTest2 == 0) //if test 2  not running as these are not required in test 2.
		{
			for(i=0; i<mtCb.numTT; i++)
			{//the manager will now start ticket takers
				MAIN_PRINT_DEBUG_VERB("going to open tt=%d\n", i);
				threadName = (char *)malloc(sizeof(100));
				sprintf(threadName, "TT_%d\0",i);
				ttThread = new Thread(threadName);//intialising the values for the ticket taker
				mtCb.tt[i]->selfThread = ttThread;
				mtCb.tt[i]->location = MOVIEROOM;
				changeTtState(debugId, i, STARTED_BY_MAN);
				mtCb.tt[i]->selfThread->Fork(ttMain, i); //ticket taker will now fork.	
			}

			//setting up the movie technician
			MAIN_PRINT_DEBUG_VERB("Going to open mt\n");
			threadName = (char *)malloc(sizeof(100));
			sprintf(threadName, "MT\0");
			mtThread = new Thread(threadName);
			mtCb.mt->selfThread = mtThread;
			mtCb.mt->location = MOVIEROOM;
			changeMtState(debugId, STARTED_BY_MAN);
			mtCb.mt->selfThread->Fork(mtMain, 0);	//the movie technician has now been forked 


		}
	}
	while(1)
	{
		if(mtTest2 == 0)  //if test 2 not running these are not required in test 2.
		{
			queueTcManage(debugId);
			if(mtTest4 == 0)  //if test 4 not running these are not required in test 4.
			{
				queueCcManage(debugId);
				queueTtManage(debugId);
			}
		}
		if(mtTest4 == 0)  //if test 4 not running these are not required in test 4.
		{
			moneyTcManage(debugId);
			moneyCcManage(debugId);
			printf("Total money made by office = %d\n", mtCb.man->money);
			if(mtTest2 == 0)  ///if test 2 not running these are not required in test 2.

			{
				movieManage(debugId);	

				for (i=0; i<mtCb.numCustGrp; i++)
				{
					if(getCustGrpLocation(debugId, i) == OUT_OF_MOVIETHEATURE);
					else break;
				}
				if( i == mtCb.numCustGrp )
				{
					exit(0);
				}
			}

			if(mtTest2 == 1)  ///if test 2 not running these are not required in test 2.
			{
				if(mtTest2Called >= 5)
				{ 
					exit(0);
				}
			}
		}
		self->selfThread->Yield();
		self->selfThread->Yield();
		self->selfThread->Yield();
	}	
}

/*
 * Manager Thread is started in it.
 * */
void startManThread(int debugId )
{
	char *threadName;	
	threadName = (char *)malloc(sizeof(100));
	sprintf(threadName, "MANAGER\0");
	mtCb.man = (Manager *)malloc(sizeof(Manager));
	mtCb.man->selfThread = new Thread(threadName);
	MAIN_PRINT_DEBUG_VERB("Manager thread is be started\n");
	mtCb.man->selfThread->Fork(manMain, 0); //his thread has been forked, calling manMain function 
}

/*
 * This is the first function that is called 
 * when the customised simulation is run.
 * */
void movieTheatureMain()
{
	//Setting up debugging information
	char dbg[10], logFileName[100];
	int i=0,ret_val=0;

	int debugId=1;
	DEBUG_INIT("main.log", &debugId);

	initializeAllValues(debugId); //intializing entities that shall be part of simulation
	initializeAllQueues(debugId);//initializing all different queues that need to be formed
	initializeAllLocks(debugId);//set up required locks
	startManThread(debugId);//start the manager
	startCustThreads(debugId);//bring in the customers

	DEBUG_CLOSE(debugId);
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
	MAIN_PRINT_DEBUG_INFO("Number of TC=%d\n", mtCb.numTC);
	for(i=0; i<mtCb.numTC; i++)
	{
		mtCb.tc[i] = (TC *)malloc(sizeof(TC));
	}

	mtCb.numCC = 5;
	MAIN_PRINT_DEBUG_INFO("Number of CC=%d\n", mtCb.numCC);
	for(i=0; i<mtCb.numCC; i++)
	{
		mtCb.cc[i] = (CC *)malloc(sizeof(CC));
	}

	mtCb.numTT = 3;
	MAIN_PRINT_DEBUG_INFO("Number of TT=%d\n", mtCb.numTT);
	for(i=0; i<mtCb.numTT; i++)
	{
		mtCb.tt[i] = (TT *)malloc(sizeof(TT));
	}

	mtCb.mvRoom = (MvRoom *)malloc(sizeof(MvRoom));
	mtCb.mt = (MT *)malloc(sizeof(MT));
	
	mtCb.numCust = 40;

	mtCb.numCustGrp=0;	
	i=mtCb.numCust;
	while(i > 0)
	{
		int randomNum = random(); 
		randomNum = (randomNum % 5) + 1;
		if(i <= 5)
		{
			randomNum = i;
		}
		i = i- randomNum;
				
		mtCb.custGrp[mtCb.numCustGrp] = (CustGrp *)malloc(sizeof(CustGrp));
		mtCb.custGrp[mtCb.numCustGrp]->grpId = mtCb.numCustGrp;
		mtCb.custGrp[mtCb.numCustGrp]->location = START;
		mtCb.custGrp[mtCb.numCustGrp]->numCust = randomNum; 
		MAIN_PRINT_DEBUG_INFO("grp=%d number of cust=%d\n", mtCb.numCustGrp, mtCb.custGrp[mtCb.numCustGrp]->numCust);
		for(j=0; j<mtCb.custGrp[mtCb.numCustGrp]->numCust; j++)
		{
			mtCb.custGrp[mtCb.numCustGrp]->cust[j] = (Cust *)malloc(sizeof(Cust));
			mtCb.custGrp[mtCb.numCustGrp]->cust[j]->selfId = j;
			mtCb.custGrp[mtCb.numCustGrp]->cust[j]->grpId = mtCb.numCustGrp;
			mtCb.custGrp[mtCb.numCustGrp]->cust[j]->money = CUST_MONEY;
			mtCb.custGrp[mtCb.numCustGrp]->cust[j]->IAMTicketBuyer = isCustTicketBuyer(debugId, mtCb.numCustGrp,j);
		}
		mtCb.numCustGrp++;

	}

	MAIN_PRINT_DEBUG_INFO("Customer Group decided\n");

	printf("Number Of Customers = %d \n", mtCb.numCust);
	printf("Number Of Groups = %d \n", mtCb.numCustGrp);
	printf("Number Of TicketClerks = %d \n", mtCb.numTC);
	printf("Number Of ConcessionClerks = %d \n", mtCb.numCC);
	printf("Number Of TicketTakers = %d \n", mtCb.numTT);
}

/*
 * Here is the first function to be called 
 * for the complete simulation with 40 customers.
 * This option is given the test suite.
 * */
void movieTheatureMain_Test7()
{

	//char *dbg = NULL, logFileName[100];
	char dbg[10], logFileName[100];
	int i=0,ret_val=0;

	int debugId=1;
	DEBUG_INIT("main.log", &debugId);

	initializeAllValues_Test7(debugId);
	initializeAllQueues(debugId);
	initializeAllLocks(debugId);
	startManThread(debugId);
	startCustThreads(debugId);

	DEBUG_CLOSE(debugId);
}

/*
 * This is to initialize 
 * 6 customers simulation and 
 * one of each employee type.
 * */
void initializeAllValues_Test8(int debugId)
{
	int i=0, j=0;

	mtCb.numTC = 1;
	MAIN_PRINT_DEBUG_INFO("Number of TC=%d\n", mtCb.numTC);
	for(i=0; i<mtCb.numTC; i++)
	{
		mtCb.tc[i] = (TC *)malloc(sizeof(TC));
	}

	mtCb.numCC = 1;
	MAIN_PRINT_DEBUG_INFO("Number of CC=%d\n", mtCb.numCC);
	for(i=0; i<mtCb.numCC; i++)
	{
		mtCb.cc[i] = (CC *)malloc(sizeof(CC));
	}

	mtCb.numTT = 1;
	MAIN_PRINT_DEBUG_INFO("Number of TT=%d\n", mtCb.numTT);
	for(i=0; i<mtCb.numTT; i++)
	{
		mtCb.tt[i] = (TT *)malloc(sizeof(TT));
	}

	mtCb.mvRoom = (MvRoom *)malloc(sizeof(MvRoom));
	mtCb.mt = (MT *)malloc(sizeof(MT));
	
	mtCb.numCust = 6;

	mtCb.numCustGrp=0;	
	i=mtCb.numCust;
	while(i > 0)
	{
		int randomNum = random(); 
		randomNum = (randomNum % 5) + 1;
		if(i <= 5)
		{
			randomNum = i;
		}
		i = i- randomNum;
				
		mtCb.custGrp[mtCb.numCustGrp] = (CustGrp *)malloc(sizeof(CustGrp));
		mtCb.custGrp[mtCb.numCustGrp]->grpId = mtCb.numCustGrp;
		mtCb.custGrp[mtCb.numCustGrp]->location = START;
		mtCb.custGrp[mtCb.numCustGrp]->numCust = randomNum; 
		MAIN_PRINT_DEBUG_INFO("grp=%d number of cust=%d\n", mtCb.numCustGrp, mtCb.custGrp[mtCb.numCustGrp]->numCust);
		for(j=0; j<mtCb.custGrp[mtCb.numCustGrp]->numCust; j++)
		{
			mtCb.custGrp[mtCb.numCustGrp]->cust[j] = (Cust *)malloc(sizeof(Cust));
			mtCb.custGrp[mtCb.numCustGrp]->cust[j]->selfId = j;
			mtCb.custGrp[mtCb.numCustGrp]->cust[j]->grpId = mtCb.numCustGrp;
			mtCb.custGrp[mtCb.numCustGrp]->cust[j]->money = CUST_MONEY;
			mtCb.custGrp[mtCb.numCustGrp]->cust[j]->IAMTicketBuyer = isCustTicketBuyer(debugId, mtCb.numCustGrp,j);
		}
		mtCb.numCustGrp++;

	}

	MAIN_PRINT_DEBUG_INFO("Customer Group decided\n");

	printf("Number Of Customers = %d \n", mtCb.numCust);
	printf("Number Of Groups = %d \n", mtCb.numCustGrp);
	printf("Number Of TicketClerks = %d \n", mtCb.numTC);
	printf("Number Of ConcessionClerks = %d \n", mtCb.numCC);
	printf("Number Of TicketTakers = %d \n", mtCb.numTT);
}

/*
 * This is a simulation start-up
 * for 6 customers option. Option 
 * is given in our test suite
 * */
void movieTheatureMain_Test8()
{

	//char *dbg = NULL, logFileName[100];
	char dbg[10], logFileName[100];
	int i=0,ret_val=0;

	int debugId=1;
	DEBUG_INIT("main.log", &debugId);

	initializeAllValues_Test8(debugId);
	initializeAllQueues(debugId);
	initializeAllLocks(debugId);
	startManThread(debugId);
	startCustThreads(debugId);

	DEBUG_CLOSE(debugId);
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
	MAIN_PRINT_DEBUG_INFO("Number of TC=%d\n", mtCb.numTC);
	for(i=0; i<mtCb.numTC; i++)
	{
		mtCb.tc[i] = (TC *)malloc(sizeof(TC));
		mtCb.tc[i]->state = FREE_AND_SELLING_FOOD_BEING_FIRST;
	}

	mtCb.numCust = 10;

	mtCb.numCustGrp=0;	
	i=mtCb.numCust;
	while(i > 0)
	{
		i = i- 1;

		mtCb.custGrp[mtCb.numCustGrp] = (CustGrp *)malloc(sizeof(CustGrp));
		mtCb.custGrp[mtCb.numCustGrp]->grpId = mtCb.numCustGrp;
		mtCb.custGrp[mtCb.numCustGrp]->location = START;
		mtCb.custGrp[mtCb.numCustGrp]->numCust = 1; 
		MAIN_PRINT_DEBUG_INFO("grp=%d number of cust=%d\n", mtCb.numCustGrp, mtCb.custGrp[mtCb.numCustGrp]->numCust);
		mtCb.custGrp[mtCb.numCustGrp]->cust[0] = (Cust *)malloc(sizeof(Cust));
		mtCb.custGrp[mtCb.numCustGrp]->cust[0]->selfId = j;
		mtCb.custGrp[mtCb.numCustGrp]->cust[0]->grpId = mtCb.numCustGrp;
		mtCb.custGrp[mtCb.numCustGrp]->cust[0]->money = CUST_MONEY;
		mtCb.custGrp[mtCb.numCustGrp]->cust[0]->IAMTicketBuyer = isCustTicketBuyer(debugId, mtCb.numCustGrp,j);
		mtCb.numCustGrp++;

	}

	MAIN_PRINT_DEBUG_INFO("Customer Groups decided\n");

	printf("Number Of Customers = 10 \n");
	printf("Number Of Groups = 10 \n");
	printf("Number Of TicketClerks = 3 \n");
	printf("Number Of ConcessionClerks = 0 \n");
	printf("Number Of TicketTakers = 0 \n");
}



/*
 * This initializes queues 
 * pertaining to test case 1
 * */
void initializeAllQueues_Test1(int debugId )
{

	int i=0;
	char *queueName;
	MAIN_PRINT_DEBUG_VERB("Initializing all queues\n");
	for(i=0; i<mtCb.numTC; i++)
	{
		mtCb.queue[TC_QUEUE][i] = (mtQueue *)malloc(sizeof(mtQueue));
		initializeQueue(debugId, mtCb.queue[TC_QUEUE][i]);
		mtCb.queue[TC_QUEUE][i]->queueId = i;
		mtCb.queue[TC_QUEUE][i]->queueType = TC_QUEUE;
		queueName = (char *)malloc(sizeof(100));
		sprintf(queueName, "QUEUE_%d_%d\0",TC_QUEUE,i);
		MAIN_PRINT_DEBUG_INFO("Initializing queue %s\n", queueName);
		queueLock[TC_QUEUE][i] = new Lock(queueName);
		queueCondVar[TC_QUEUE][i] = new Condition(queueName);
	}
	queueName = (char *)malloc(sizeof(100));
	sprintf(queueName, "QUEUE_TC\0");
	queueTcLock = new Lock(queueName);
}

/*
 * This adds address to queue
 * pertaining to test case one
 * */
void addAddressToQueue_Test1(int debugId)
{
	int i=0;
	MAIN_PRINT_DEBUG_INFO("Adding address to queue=TC_QUEUE\n");
	for(i=0; i<mtCb.numTC; i++)
	{
		MAIN_PRINT_DEBUG_VERB("Adding address to TC queue\n");
		mtCb.tc[i]->queue = mtCb.queue[TC_QUEUE][i];
		mtCb.tc[i]->queue->queueType = TC_QUEUE;
		mtCb.tc[i]->queue->queueAddress = (Employee)mtCb.tc[i];
	}
}

/*
 * This adds address to queue
 * pertaining to test case 2
 * */
void addAddressToQueue_Test2(int debugId)
{
	int i=0;
	MAIN_PRINT_DEBUG_INFO("Adding address to queue=TC_QUEUE\n");
	for(i=0; i<mtCb.numTC; i++)
	{
		MAIN_PRINT_DEBUG_VERB("Adding address to TC queue\n");
		mtCb.tc[i]->queue = mtCb.queue[TC_QUEUE][i];
		mtCb.tc[i]->queue->queueType = TC_QUEUE;
		mtCb.tc[i]->queue->queueAddress = (Employee)mtCb.tc[i];
	}
	for(i=0; i<mtCb.numCC; i++)
	{
		MAIN_PRINT_DEBUG_VERB("Adding address to CC queue\n");
		mtCb.cc[i]->queue = mtCb.queue[CC_QUEUE][i];
		mtCb.cc[i]->queue->queueType = CC_QUEUE;
		mtCb.cc[i]->queue->queueAddress = (Employee)mtCb.cc[i];
	}

}

/*
 * This is to initialze locks
 * pertaining to test case 1.
 *
 * */
void initializeAllLocks_Test1(int debugId)
{
	int i=0, j=0; char *lockName;
	MAIN_PRINT_DEBUG_VERB("Initializing all locks\n");

	for(i=0; i<mtCb.numCustGrp; i++)
	{
		for(j=0; j<mtCb.custGrp[i]->numCust; j++)
		{
			lockName = (char *)malloc(sizeof(100));
			sprintf(lockName, "CUST_%d_%d\0",i,j);
			MAIN_PRINT_DEBUG_INFO("Initializing lock %s\n", lockName);
			custLock[i][j]=new Lock(lockName);
			custCondVar[i][j]=new Condition(lockName);
		}
	}

	for(i=0; i<mtCb.numTC; i++)
	{
		lockName = (char *)malloc(sizeof(100));
		sprintf(lockName, "TC_%d\0",i);
		MAIN_PRINT_DEBUG_INFO("Initializing lock %s\n", lockName);
		tcLock[i]=new Lock(lockName);
		tcCondVar[i]=new Condition(lockName);
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
	//char *dbg = NULL, logFileName[100];
	char dbg[10], logFileName[100];
	int i=0,ret_val=0;

	int debugId=1;
	DEBUG_INIT("main.log", &debugId);

	initializeAllValues_Test1(debugId);
	initializeAllQueues_Test1(debugId);
	initializeAllLocks_Test1(debugId);
	addAddressToQueue_Test1(debugId);
	startCustThreads(debugId);
	
	for(i=0; i< mtCb.numCustGrp; i++)
	{
		while(mtCb.custGrp[i]->cust[0]->state != WAIT)
		{ 
			currentThread->Yield(); 
		}
	}
	exit(0);

	DEBUG_CLOSE(debugId);
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
	MAIN_PRINT_DEBUG_INFO("Number of TC=%d\n", mtCb.numTC);
	for(i=0; i<mtCb.numTC; i++)
	{
		mtCb.tc[i] = (TC *)malloc(sizeof(TC));
		mtCb.tc[i]->state = FREE_AND_SELLING_TICKET_BEING_FIRST;
	}

	mtCb.numCust = 5;

	mtCb.numCustGrp=0;	
	i=mtCb.numCust;
	while(i > 0)
	{
		i = i- 1;

		mtCb.custGrp[mtCb.numCustGrp] = (CustGrp *)malloc(sizeof(CustGrp));
		mtCb.custGrp[mtCb.numCustGrp]->grpId = mtCb.numCustGrp;
		mtCb.custGrp[mtCb.numCustGrp]->location = START;
		mtCb.custGrp[mtCb.numCustGrp]->numCust = 1; 
		MAIN_PRINT_DEBUG_INFO("grp=%d number of cust=%d\n", mtCb.numCustGrp, mtCb.custGrp[mtCb.numCustGrp]->numCust);
		mtCb.custGrp[mtCb.numCustGrp]->cust[0] = (Cust *)malloc(sizeof(Cust));
		mtCb.custGrp[mtCb.numCustGrp]->cust[0]->selfId = j;
		mtCb.custGrp[mtCb.numCustGrp]->cust[0]->grpId = mtCb.numCustGrp;
		mtCb.custGrp[mtCb.numCustGrp]->cust[0]->money = CUST_MONEY;
		mtCb.custGrp[mtCb.numCustGrp]->cust[0]->IAMTicketBuyer = isCustTicketBuyer(debugId, mtCb.numCustGrp,j);
		mtCb.numCustGrp++;

	}

	MAIN_PRINT_DEBUG_INFO("Customer Groups decided\n");

	printf("Number Of Customers = 5 \n");
	printf("Number Of Groups = 5 \n");
	printf("Number Of TicketClerks = 1 \n");
	printf("Number Of ConcessionClerks = 0 \n");
	printf("Number Of TicketTakers = 0 \n");
}

/*
 * This is to initialize queues
 * pertaining to test case 3.
 *
 * */
void initializeAllQueues_Test3(int debugId )
{

	int i=0;
	char *queueName;
	MAIN_PRINT_DEBUG_VERB("Initializing all queues\n");
	for(i=0; i<mtCb.numTC; i++)
	{
		mtCb.queue[TC_QUEUE][i] = (mtQueue *)malloc(sizeof(mtQueue));
		initializeQueue(debugId, mtCb.queue[TC_QUEUE][i]);
		mtCb.queue[TC_QUEUE][i]->queueId = i;
		mtCb.queue[TC_QUEUE][i]->queueType = TC_QUEUE;
		queueName = (char *)malloc(sizeof(100));
		sprintf(queueName, "QUEUE_%d_%d\0",TC_QUEUE,i);
		MAIN_PRINT_DEBUG_INFO("Initializing queue %s\n", queueName);
		queueLock[TC_QUEUE][i] = new Lock(queueName);
		queueCondVar[TC_QUEUE][i] = new Condition(queueName);
	}
}

/*
 * This is to initialize all
 * locks pertaining to test case
 * 3.
 * */
void initializeAllLocks_Test3(int debugId)
{
	int i=0, j=0; char *lockName;
	MAIN_PRINT_DEBUG_VERB("Initializing all locks\n");

	for(i=0; i<mtCb.numCustGrp; i++)
	{
		for(j=0; j<mtCb.custGrp[i]->numCust; j++)
		{
			lockName = (char *)malloc(sizeof(100));
			sprintf(lockName, "CUST_%d_%d\0",i,j);
			MAIN_PRINT_DEBUG_INFO("Initializing lock %s\n", lockName);
			custLock[i][j]=new Lock(lockName);
			custCondVar[i][j]=new Condition(lockName);
		}
	}

	for(i=0; i<mtCb.numTC; i++)
	{
		lockName = (char *)malloc(sizeof(100));
		sprintf(lockName, "TC_%d\0",i);
		MAIN_PRINT_DEBUG_INFO("Initializing lock %s\n", lockName);
		tcLock[i]=new Lock(lockName);
		tcCondVar[i]=new Condition(lockName);
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
	int i=0,ret_val=0;

	int debugId=1;
	DEBUG_INIT("main.log", &debugId);

	char *threadName;
	initializeAllValues_Test3(debugId);
	initializeAllQueues_Test3(debugId);
	initializeAllLocks_Test3(debugId);
	mtTest3=1;
	startCustThreads(debugId);

	for(i=0; i<mtCb.numTC; i++)
	{
		MAIN_PRINT_DEBUG_VERB("going to open tc=%d\n", i);
		threadName = (char *)malloc(sizeof(100));
		sprintf(threadName, "TC_%d\0",i);
		mtCb.tc[i]->selfThread = new Thread(threadName);
		mtCb.tc[i]->queueType = TC_QUEUE; 
		mtCb.tc[i]->queue = mtCb.queue[TC_QUEUE][i];
		mtCb.tc[i]->location = TICKET_CLERK;
		changeTcState(debugId, i, STARTED_BY_MAN);
		mtCb.tc[i]->selfThread->Fork(tcMain, i); 
	}

	while(mtTest3Called != 5)
	{ 
		currentThread->Yield(); 
	}
	exit(0);

	DEBUG_CLOSE(debugId);
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
	MAIN_PRINT_DEBUG_INFO("Number of TC=%d\n", mtCb.numTC);
	for(i=0; i<mtCb.numTC; i++)
	{
		mtCb.tc[i] = (TC *)malloc(sizeof(TC));
		mtCb.tc[i]->state = FREE_AND_SELLING_TICKET_BEING_FIRST;
	}
	for(i=0; i<mtCb.numCC; i++)
	{
		mtCb.cc[i] = (CC *)malloc(sizeof(CC));
		mtCb.cc[i]->state = FREE_AND_SELLING_FOOD_BEING_FIRST;
	}


	mtCb.numCust = 5;

	mtCb.numCustGrp=0;	
	i=mtCb.numCust;
	while(i > 0)
	{
		i = i- 1;

		mtCb.custGrp[mtCb.numCustGrp] = (CustGrp *)malloc(sizeof(CustGrp));
		mtCb.custGrp[mtCb.numCustGrp]->grpId = mtCb.numCustGrp;
		mtCb.custGrp[mtCb.numCustGrp]->location = START;
		mtCb.custGrp[mtCb.numCustGrp]->numCust = 1; 
		MAIN_PRINT_DEBUG_INFO("grp=%d number of cust=%d\n", mtCb.numCustGrp, mtCb.custGrp[mtCb.numCustGrp]->numCust);
		mtCb.custGrp[mtCb.numCustGrp]->cust[0] = (Cust *)malloc(sizeof(Cust));
		mtCb.custGrp[mtCb.numCustGrp]->cust[0]->selfId = j;
		mtCb.custGrp[mtCb.numCustGrp]->cust[0]->grpId = mtCb.numCustGrp;
		mtCb.custGrp[mtCb.numCustGrp]->cust[0]->money = CUST_MONEY;
		mtCb.custGrp[mtCb.numCustGrp]->cust[0]->IAMTicketBuyer = isCustTicketBuyer(debugId, mtCb.numCustGrp,j);
		mtCb.numCustGrp++;

	}

	MAIN_PRINT_DEBUG_INFO("Customer Groups decided\n");

	printf("Number Of Customers = 5 \n");
	printf("Number Of Groups = 5 \n");
	printf("Number Of TicketClerks = 2 \n");
	printf("Number Of ConcessionClerks = 1 \n");
	printf("Number Of TicketTakers = 0 \n");
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
	char *queueName;
	MAIN_PRINT_DEBUG_VERB("Initializing all queues\n");
	for(i=0; i<mtCb.numTC; i++)
	{
		mtCb.queue[TC_QUEUE][i] = (mtQueue *)malloc(sizeof(mtQueue));
		initializeQueue(debugId, mtCb.queue[TC_QUEUE][i]);
		mtCb.queue[TC_QUEUE][i]->queueId = i;
		mtCb.queue[TC_QUEUE][i]->queueType = TC_QUEUE;
		queueName = (char *)malloc(sizeof(100));
		sprintf(queueName, "QUEUE_%d_%d\0",TC_QUEUE,i);
		MAIN_PRINT_DEBUG_INFO("Initializing queue %s\n", queueName);
		queueLock[TC_QUEUE][i] = new Lock(queueName);
		queueCondVar[TC_QUEUE][i] = new Condition(queueName);
	}
	queueName = (char *)malloc(sizeof(100));
	sprintf(queueName, "QUEUE_TC\0");
	queueTcLock = new Lock(queueName);
	for(i=0; i<mtCb.numCC; i++)
	{
		mtCb.queue[CC_QUEUE][i] = (mtQueue *)malloc(sizeof(mtQueue));
		initializeQueue(debugId, mtCb.queue[CC_QUEUE][i]);
		mtCb.queue[CC_QUEUE][i]->queueId = i;
		mtCb.queue[CC_QUEUE][i]->queueType = CC_QUEUE;
		queueName = (char *)malloc(sizeof(100));
		sprintf(queueName, "QUEUE_%d_%d\0",CC_QUEUE,i);
		MAIN_PRINT_DEBUG_INFO("Initializing queue %s\n", queueName);
		queueLock[CC_QUEUE][i] = new Lock(queueName);
		queueCondVar[CC_QUEUE][i] = new Condition(queueName);
	}
}

/*
 * This is to initialize 
 * all locks pertaining to test
 * case 2.
 * */
void initializeAllLocks_Test2(int debugId)
{
	int i=0, j=0; char *lockName;
	MAIN_PRINT_DEBUG_VERB("Initializing all locks\n");

	for(i=0; i<mtCb.numCustGrp; i++)
	{
		for(j=0; j<mtCb.custGrp[i]->numCust; j++)
		{
			lockName = (char *)malloc(sizeof(100));
			sprintf(lockName, "CUST_%d_%d\0",i,j);
			MAIN_PRINT_DEBUG_INFO("Initializing lock %s\n", lockName);
			custLock[i][j]=new Lock(lockName);
			custCondVar[i][j]=new Condition(lockName);
		}
	}

	for(i=0; i<mtCb.numTC; i++)
	{
		lockName = (char *)malloc(sizeof(100));
		sprintf(lockName, "TC_%d\0",i);
		MAIN_PRINT_DEBUG_INFO("Initializing lock %s\n", lockName);
		tcLock[i]=new Lock(lockName);
		tcCondVar[i]=new Condition(lockName);
	}
	for(i=0; i<mtCb.numCC; i++)
	{
		lockName = (char *)malloc(sizeof(100));
		sprintf(lockName, "CC_%d\0",i);
		MAIN_PRINT_DEBUG_INFO("Initializing lock %s\n", lockName);
		ccLock[i]=new Lock(lockName);
		ccCondVar[i]=new Condition(lockName);
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
	DEBUG_INIT("main.log", &debugId);

	char *threadName;
	initializeAllValues_Test2(debugId);
	initializeAllQueues_Test2(debugId);
	initializeAllLocks_Test2(debugId);
	addAddressToQueue_Test2(debugId);
	mtTest2=1;
	startManThread(debugId);
	startCustThreads(debugId);

	DEBUG_CLOSE(debugId);
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
	DEBUG_INIT("main.log", &debugId);

	char *threadName;
	initializeAllValues_Test2(debugId);
	initializeAllQueues_Test2(debugId);
	initializeAllLocks_Test2(debugId);
	addAddressToQueue_Test2(debugId);
	mtTest2=1;
	startManThread(debugId);
	startCustThreads(debugId);

	DEBUG_CLOSE(debugId);
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
	MAIN_PRINT_DEBUG_INFO("Number of TC=%d\n", mtCb.numTC);

	mtCb.numCust = 3;

	mtCb.numCustGrp=1;	
	for(i=0; i<mtCb.numCustGrp; i++)
	{
		mtCb.custGrp[i] = (CustGrp *)malloc(sizeof(CustGrp));
		mtCb.custGrp[i]->grpId = i;
		mtCb.custGrp[i]->location = START;
		mtCb.custGrp[i]->numCust = 3; 
		MAIN_PRINT_DEBUG_INFO("grp=%d number of cust=%d\n", i, mtCb.custGrp[i]->numCust);
		for(j=0; j<3; j++)
		{
			mtCb.custGrp[i]->cust[j] = (Cust *)malloc(sizeof(Cust));
			mtCb.custGrp[i]->cust[j]->selfId = j;
			mtCb.custGrp[i]->cust[j]->grpId = i;
			mtCb.custGrp[i]->cust[j]->money = CUST_MONEY;
			mtCb.custGrp[i]->cust[j]->IAMTicketBuyer = isCustTicketBuyer(debugId, i,j);
		}
	}

	MAIN_PRINT_DEBUG_INFO("Customer Groups decided\n");

	printf("Number Of Customers = 3 \n");
	printf("Number Of Groups = 1 \n");
	printf("Number Of TicketClerks = 0 \n");
	printf("Number Of ConcessionClerks = 0 \n");
	printf("Number Of TicketTakers = 0 \n");
}

/*
 * This initializes all
 * locks pertaining to test case
 * 6
 * */
void initializeAllLocks_Test6(int debugId)
{
	int i=0, j=0; char *lockName;
	MAIN_PRINT_DEBUG_VERB("Initializing all locks\n");

	for(i=0; i<mtCb.numCustGrp; i++)
	{
		for(j=0; j<mtCb.custGrp[i]->numCust; j++)
		{
			lockName = (char *)malloc(sizeof(100));
			sprintf(lockName, "CUST_%d_%d\0",i,j);
			MAIN_PRINT_DEBUG_INFO("Initializing lock %s\n", lockName);
			custLock[i][j]=new Lock(lockName);
			custCondVar[i][j]=new Condition(lockName);
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
	DEBUG_INIT("main.log", &debugId);

	char *threadName;
	initializeAllValues_Test6(debugId);
	initializeAllLocks_Test6(debugId);
	mtTest6=1;
	startCustThreads(debugId);
	while(mtTest6Called != 3)
	{ 
		currentThread->Yield(); 
	}
	exit(0);


	DEBUG_CLOSE(debugId);
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
	MAIN_PRINT_DEBUG_INFO("Number of TC=%d\n", mtCb.numTC);
	for(i=0; i<mtCb.numTC; i++)
	{
		mtCb.tc[i] = (TC *)malloc(sizeof(TC));
		mtCb.tc[i]->state = FREE_AND_SELLING_FOOD_BEING_FIRST;
	}
	mtCb.tc[0]->state = ON_BREAK;
	mtCb.tc[0]->msgByMan = GO_ON_BREAK;

	mtCb.numCust = 6;

	mtCb.numCustGrp=0;	
	i=mtCb.numCust;
	while(i > 0)
	{
		i = i- 1;

		mtCb.custGrp[mtCb.numCustGrp] = (CustGrp *)malloc(sizeof(CustGrp));
		mtCb.custGrp[mtCb.numCustGrp]->grpId = mtCb.numCustGrp;
		mtCb.custGrp[mtCb.numCustGrp]->location = START;
		mtCb.custGrp[mtCb.numCustGrp]->numCust = 1; 
		MAIN_PRINT_DEBUG_INFO("grp=%d number of cust=%d\n", mtCb.numCustGrp, mtCb.custGrp[mtCb.numCustGrp]->numCust);
		mtCb.custGrp[mtCb.numCustGrp]->cust[0] = (Cust *)malloc(sizeof(Cust));
		mtCb.custGrp[mtCb.numCustGrp]->cust[0]->selfId = j;
		mtCb.custGrp[mtCb.numCustGrp]->cust[0]->grpId = mtCb.numCustGrp;
		mtCb.custGrp[mtCb.numCustGrp]->cust[0]->money = CUST_MONEY;
		mtCb.custGrp[mtCb.numCustGrp]->cust[0]->IAMTicketBuyer = isCustTicketBuyer(debugId, mtCb.numCustGrp,j);
		mtCb.numCustGrp++;

	}

	MAIN_PRINT_DEBUG_INFO("Customer Groups decided\n");
	printf("Number Of Customers = 6 \n");
	printf("Number Of Groups = 6 \n");
	printf("Number Of TicketClerks = 2 \n");
	printf("Number Of ConcessionClerks = 0 \n");
	printf("Number Of TicketTakers = 0 \n");
}

/*
 * This is to initialze all
 * queues pertaining to test 
 * case 4.
 * */
void initializeAllQueues_Test4(int debugId )
{

	int i=0;
	char *queueName;
	MAIN_PRINT_DEBUG_VERB("Initializing all queues\n");
	for(i=0; i<mtCb.numTC; i++)
	{
		mtCb.queue[TC_QUEUE][i] = (mtQueue *)malloc(sizeof(mtQueue));
		initializeQueue(debugId, mtCb.queue[TC_QUEUE][i]);
		mtCb.queue[TC_QUEUE][i]->queueId = i;
		mtCb.queue[TC_QUEUE][i]->queueType = TC_QUEUE;
		queueName = (char *)malloc(sizeof(100));
		sprintf(queueName, "QUEUE_%d_%d\0",TC_QUEUE,i);
		MAIN_PRINT_DEBUG_INFO("Initializing queue %s\n", queueName);
		queueLock[TC_QUEUE][i] = new Lock(queueName);
		queueCondVar[TC_QUEUE][i] = new Condition(queueName);
	}

}

/*
 * This is to initialze all
 * locks pertaing to test case
 * 4
 * */
void initializeAllLocks_Test4(int debugId)
{
	int i=0, j=0; char *lockName;
	MAIN_PRINT_DEBUG_VERB("Initializing all locks\n");

	for(i=0; i<mtCb.numCustGrp; i++)
	{
		for(j=0; j<mtCb.custGrp[i]->numCust; j++)
		{
			lockName = (char *)malloc(sizeof(100));
			sprintf(lockName, "CUST_%d_%d\0",i,j);
			MAIN_PRINT_DEBUG_INFO("Initializing lock %s\n", lockName);
			custLock[i][j]=new Lock(lockName);
			custCondVar[i][j]=new Condition(lockName);
		}
	}


	for(i=0; i<mtCb.numTC; i++)
	{
		lockName = (char *)malloc(sizeof(100));
		sprintf(lockName, "TC_%d\0",i);
		MAIN_PRINT_DEBUG_INFO("Initializing lock %s\n", lockName);
		tcLock[i]=new Lock(lockName);
		tcCondVar[i]=new Condition(lockName);
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
	//char *dbg = NULL, logFileName[100];
	char dbg[10], logFileName[100];
	int i=0,ret_val=0;

	int debugId=1;
	DEBUG_INIT("main.log", &debugId);
	mtTest4=1;

	initializeAllValues_Test4(debugId);
	initializeAllQueues_Test4(debugId);
	initializeAllLocks_Test4(debugId);

	startCustThreads(debugId);
	startManThread(debugId);
	
	
	DEBUG_CLOSE(debugId);
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
	printf("\n\n\t\t-------- CSCI 402: Assignment 1 (Made By Group 3)--------\n\n\n");
	printf("Please enter the corresponding number from the options below to run desired simulation or testcases: \n\n\n");
	printf("\t1.  Run the given testsuite for Part 1\n\n");
	printf("\t2.  Run the test for Part 1 made by us \n\t\t: It shows that order of acquire and release is mantained across several cycles\n\n");
	printf("\t3.  Run the Test 1 for Part 2 \n\t\t: It shows that Customers always take the shortest line, but no 2 customers ever choose the same shortest line at the same time \n\n");
	printf("\t4.  Run the Test 2 for Part 2 \n\t\t: It shows that Managers only read one from one Clerk's total money received, at a time \n\n");
	printf("\t5.  Run the Test 3 for Part 2 \n\t\t: It shows that Customers do not leave a Clerk, or TicketTaker, until they are told to do so. Clerks and TicketTakers do not start with another Customer until they know the current Customer has left. customer until they know that the last Customer has left their area \n\n");
	printf("\t6.  Run the Test 4 for Part 2 \n\t\t: It shows that Managers get Clerks off their break when lines get too long \n\n");
	printf("\t7.  Run the Test 5 for Part 2 \n\t\t: It shows that Total sales never suffers from a race condition \n\n");
	printf("\t8.  Run the Test 6 for Part 2 \n\t\t: It shows that Customer groups always move together through the theater. This requires explicit synchronization that you implement. \n\n");
	printf("\t9. Run complete simulation with 6 customers and 1 employee of each type\n\n");
	printf("\t10. Run complete simulation with 40 customers and maximum allowable employees of each type\n\n");
	printf("\t11. Run complete customised simulation (with user inputs)\n\n");


	scanf("%d",&choice);

	switch (choice)
	{
		case 1 :
            printf("You selected %d. The given testsuite for part 1 will now run...\n\n", choice);
			ThreadTest_ForPart1();
            break;
	    case 2 :
            printf("You selected %d. Our self made test for Part 1 will now run...\n\n", choice);
			SelfTest1_ForPart1();
            break;
	    case 3 :
            printf("You selected %d. The Test 1 for Part 2 will now run...\n\n", choice);
			movieTheatureMain_Test1();
            break;
	    case 4 :
            printf("You selected %d. The Test 2 for Part 2 will now run...\n\n", choice);
			movieTheatureMain_Test2();
            break;
	    case 5 :
            printf("You selected %d. The Test 3 for Part 2 will now run...\n\n", choice);
			movieTheatureMain_Test3();
            break;
	    case 6 :
            printf("You selected %d. The Test 4 for Part 2 will now run...\n\n", choice);
			movieTheatureMain_Test4();
            break;
	    case 7 :
            printf("You selected %d. The Test 5 for Part 2 will now run...\n\n", choice);
			movieTheatureMain_Test5();
            break;
	    case 8 :
            printf("You selected %d. The Test 6 for Part 2 will now run...\n\n", choice);
			movieTheatureMain_Test6();
            break;
	    case 9 :
            printf("You selected %d. A complete simulation with 6 customers and 1 employee of each type will now run...\n\n", choice);
			movieTheatureMain_Test8();
            break;
	    case 10 :
            printf("You selected %d. A complete simulation, with 40 customers and maximum allowable employees of each type, will now run...\n\n", choice);
			movieTheatureMain_Test7();
            break;
	    case 11 :
            printf("You selected %d. A complete simulation with customised inputs will now run...\n\n", choice);
			movieTheatureMain();
            break;
       default:
            printf("You gave invalid input. The program will exit now...\n\n");
	}
	return;
}

