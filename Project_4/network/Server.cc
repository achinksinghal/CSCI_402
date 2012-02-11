#include "system.h"
#include "synch.h"
#include "network.h"
#include "post.h"
#include <sstream>
#include <string.h>
#include "bitmap.h"
#include "Request.h"
#include "timer.h"
#include <time.h>

Request *incomingRequest[1500];
BitMap ReqBitMap(1500);

Lock *reqLock = new Lock("RequestLock");
BitMap lockBitMap(550);
BitMap CVBitMap(550);
BitMap MVBitMap(550);
int netname;
#define UNUSED -100
extern int failsafe;

//int clientPingMailBoxId;
//Have locks for this - we are checking in this different threads : S-S, C-S and C-S2 (checking part)

typedef struct _RequestsTracker
{
	int createdLockID[LOCK_SIZE]; //list of lock id create by this server
	PacketHeader requestPacketHeader[LOCK_SIZE];//associated locks were created due to request by this client - mailbox id
	MailHeader requestMailHeader[LOCK_SIZE];//associated locks were created due to request by this client - network id
	int countLocksCreated;
	int forciblyReleasedLockID[LOCK_SIZE];
} RequestorsControlBlock;

RequestorsControlBlock reqCb;
//int forciblyReleasedLockID=-1;

Lock *reqCbLock = new Lock("reqCbLock");
Lock *forceReleaseLock = new Lock("forceRelLock");

void initializeReqCb()
{
	//printf("\ninside initializeReqCb\n");
	int idx = UNUSED;
	//printf("\ninside initializeReqCb - idx is set to %d\n", UNUSED);
	
	reqCb.countLocksCreated = -1;//the count for locks created starts from 0 - So actual locks created is countLocksCreated+1
	for (idx=0; idx<LOCK_SIZE; idx++)
	{
			reqCb.createdLockID[idx] = UNUSED;

			reqCb.requestPacketHeader[idx].from = UNUSED;
			reqCb.requestPacketHeader[idx].to = UNUSED;
			reqCb.requestPacketHeader[idx].length = 0;

			reqCb.requestMailHeader[idx].from = UNUSED;
			reqCb.requestMailHeader[idx].to = UNUSED;
			reqCb.requestMailHeader[idx].length = 0;
			
			reqCb.forciblyReleasedLockID[idx] = UNUSED; 
			//printf("\n JASPR  reqCb.createdLockID[%d] is %d",idx, reqCb.createdLockID[idx]);
	}
}
int FindLocks(char *lockName)
{
	for(int idx = 0; idx < LOCK_SIZE; idx++)
	{
		// check if all the created locks have been checked
		if(ServerLocks[idx] == NULL)
		{
			return -1;
		}
		else
		{// there are locks in the lock table check if the lock with a name sent has already been created
			if(!strcmp(ServerLocks[idx] -> getName(), lockName))
			{ 
				return idx;
			}
		}
	}
	
	// the entire lock table was scanned and the lock was not found
	return -1;
}

int FindCondition(char *cvName)
{
	for(int idx = 0; idx < CV_SIZE; idx++)
	{
		// check if all the created condition variables have been checked
		if(ServerConditions[idx] == NULL)
		{
			return -1;
		}
		else
		{
			// there are CVs in the CV table, check if the CV with the name sent has already been created
			if(!strcmp(ServerConditions[idx] -> getName(), cvName))
			{
				return idx;
			}
		}
	}
	
	// the entire table has been scanned 
	return -1;
}

int FindMV(char *mvName)
{
	for(int idx = 0; idx < MV_SIZE; idx++)
	{
		// check if all the created MVs have been checked
		if(MonitorVariables[idx] == NULL)
		{
			return -1;
		}
		else
		{// there are MVs in the MV table check if the MV with a name sent has already been created
			if(!strcmp(MonitorVariables[idx] -> getName(), mvName))
			{ 
				return idx;
			}
		}
	}
	
	// the entire MV table was scanned and the MV was not found
	return -1;
}

void CreateLock(char *name, PacketHeader pktHdr, MailHeader mailHdr, int msgID)
{
	//stringstream msgStream (stringstream :: in | stringstream :: out);
	
	char response[MaxMailSize];
	
	
	int lockID = lockBitMap.Find();
			
	//char *response = new char[MaxMailSize];
	
	//msgStream.clear();
//	msgStream.str("abc");
	//cout <<  msgStream.str();
			
	if(lockID != -1)
	{
		ServerLock *lock = new ServerLock(name);
		ServerLocks[lockID] = lock;

		//Project 4 - Extra Credit
		reqCbLock->Acquire();
		reqCb.countLocksCreated++;
		reqCb.createdLockID[reqCb.countLocksCreated] = lockID; //we store lockId created by this server
		reqCb.requestMailHeader[reqCb.countLocksCreated] = mailHdr;//we store mailboxHdr of requesting client
		//printf("\n\n-------------JASPR CreateLock recd from mailbox=%d, networkID=%d..________\n\n", mailHdr.to, pktHdr.to);

		reqCb.requestPacketHeader[reqCb.countLocksCreated] = pktHdr;
		reqCbLock->Release();

		//msgStream << "LOCK_CREATE 1 " << lockID;
		sprintf(response,"LOCK_CREATE 1 %d", lockID);
	}
	else
	{
		strcpy(response,"LOCK_CREATE 0 Exhausted");
	}
	
	//cout <<  msgStream.str();
	//response = const_cast<char *>(msgStream.str().c_str());
	
	mailHdr.length = strlen(response) + 1;
	mailHdr.from = 0;
	pktHdr.from = netname;
	postOffice -> Send(pktHdr, mailHdr, response);
	
	while((serverCount > 1) && (!incomingRequest[msgID] -> queuedList -> IsEmpty()))
	{
		Response *rsp = (Response *)incomingRequest[msgID] -> queuedList -> Remove();
				
		pktHdr.to = rsp -> networkID;
		mailHdr.to = rsp -> mailboxID;
		
		postOffice -> Send(pktHdr, mailHdr, response);
	}
	
	//delete[] response;
}

void CreateCondition(char *name, PacketHeader pktHdr, MailHeader mailHdr, int msgID)
{
	stringstream msgStream (stringstream :: in | stringstream :: out);
	
	int cvID = CVBitMap.Find();
			
	char response[MaxMailSize];
			
	if(cvID != -1)
	{
		ServerCondition *cv = new ServerCondition(name);
		ServerConditions[cvID] = cv;
				
		msgStream << "CV_CREATE 1 " << cvID;
	}
	else
	{
		msgStream << "CV_CREATE 0 Exhausted" << endl; 
	}
	
	strcpy(response,const_cast<char *>(msgStream.str().c_str()));
	mailHdr.length = strlen(response) + 1;
	postOffice -> Send(pktHdr, mailHdr, response);
	
	while((serverCount > 1) && (!incomingRequest[msgID] -> queuedList -> IsEmpty()))
	{
		Response *rsp = (Response *)incomingRequest[msgID] -> queuedList -> Remove();
				
		pktHdr.to = rsp -> networkID;
		mailHdr.to = rsp -> mailboxID;
		
		postOffice -> Send(pktHdr, mailHdr, response);
	}
}

void CreateMonitor(char *name, PacketHeader pktHdr, MailHeader mailHdr, int msgID)
{
	stringstream msgStream (stringstream :: in | stringstream :: out);
	
	int mvID = MVBitMap.Find();
			
	char response[MaxMailSize];
	
	if(mvID != -1)
	{
		MonitorVariable *mv = new MonitorVariable(name);
		MonitorVariables[mvID] = mv;
				
		msgStream << "MV_CREATE 1 " << mvID;
	}
	else
	{
		msgStream << "MV_CREATE 0 Exhausted";
	}
			
	strcpy(response,const_cast<char *>(msgStream.str().c_str()));
	mailHdr.length = strlen(response) + 1;
	postOffice -> Send(pktHdr, mailHdr, response);
	
	while((serverCount > 1) && (!incomingRequest[msgID] -> queuedList -> IsEmpty()))
	{
		Response *rsp = (Response *)incomingRequest[msgID] -> queuedList -> Remove();
				
		pktHdr.to = rsp -> networkID;
		mailHdr.to = rsp -> mailboxID;
		
		postOffice -> Send(pktHdr, mailHdr, response);
	}
}

void LockAcquire(char *lockName, char *actualMsg,int networkID, int mailboxID)
{
	PacketHeader outPktHdr;
	MailHeader outMailHdr;
	
	bool sendResponse = false;
	int lockID = FindLocks(lockName);
	
	stringstream msgStream (stringstream::in | stringstream::out);
	
	DEBUG('w',"@@@@@@@@@Inside Acquire and message is : %s :", actualMsg);
	
	if((lockID == -1) && (serverCount == 1))
	{
		msgStream << "LOCK_ACQ 0 BadLock";
		char response[MaxMailSize] ;
		strcpy(response,const_cast<char *>(msgStream.str().c_str()));
			
		outPktHdr.to = networkID;
		outMailHdr.to = mailboxID;
		outMailHdr.length = strlen(response) + 1;
			
		postOffice -> Send(outPktHdr, outMailHdr, response);
		
		return;
	}
	
	if(lockID != -1) // this server had created the lock
	{
		ServerLock *lock = ServerLocks[lockID];
		
		if(lock -> deleteCalled == false) // the lock was created and the delete was no called 
		{
			if(lock -> GetLockState() == FREE)
			{
				
				DEBUG('w',"The lock was free");
				sendResponse = true;
			}
					
			DEBUG('w',"<<<Inside Acquire , Acqquiring for for NID = %d and MID = %d>>>>>", networkID, mailboxID);
			lock -> Acquire(networkID, mailboxID);
		}
		else
		{
			msgStream << "LOCK_ACQ 0 BadLock";
			char response[MaxMailSize] ;
			strcpy(response,const_cast<char *>(msgStream.str().c_str()));
			
			outPktHdr.to = networkID;
			outMailHdr.to = mailboxID;
			outMailHdr.length = strlen(response) + 1;
			
			postOffice -> Send(outPktHdr, outMailHdr, response);
		}
		
		if(sendResponse)
		{
			char response[MaxMailSize];
			
			msgStream << "LOCK_ACQ 1 OK";
			
			strcpy(response,const_cast<char *>(msgStream.str().c_str()));
			
			outPktHdr.to = networkID;
			outPktHdr.from = netname;
			outMailHdr.to = mailboxID;
			outMailHdr.from = 0;
			outMailHdr.length = strlen(response) + 1;
			
			postOffice -> Send(outPktHdr, outMailHdr, response);
		}
		
	}
	else // lock belongs to some other server and we need to create a Request class object and push it in the incomingRequest queue
	{
		reqLock -> Acquire();
		int reqID = ReqBitMap.Find();
		
		Request *req = new Request(networkID, mailboxID);
		strcpy(req -> clientRequest, actualMsg);
		
		
		incomingRequest[reqID] = req;
		reqLock -> Release();
	}
	
}

void LockRelease(char *lockName, char *actualMsg,int networkID, int mailboxID, bool sendResponse)
{
	PacketHeader outPktHdr;
	MailHeader outMailHdr;
	
	int lockID = FindLocks(lockName);
	
	stringstream msgStream (stringstream::in | stringstream::out);
	
	if((lockID == -1) && (serverCount == 1))
	{
		msgStream << "LOCK_REL 0 BadLock";
		char response[MaxMailSize] ;
		strcpy(response,const_cast<char *>(msgStream.str().c_str()));
			
		outPktHdr.to = networkID;
		outMailHdr.to = mailboxID;
		outMailHdr.length = strlen(response) + 1;
			
		postOffice -> Send(outPktHdr, outMailHdr, response);
		
		return;
	}
	
	if(lockID != -1) // this server had created the lock
	{
		int forciblyReleased=0;
		reqCbLock->Acquire();
		/*
		* we execute this if we arent in failsafe code and 
		* if we are in failsafe code - we only do this is if 
		* this lock wasnt already forcibly released 
		*/
		if (failsafe) //we check here to find out if this lockID was forcible removed
		{
			int i;
			for (i=0; i<LOCK_SIZE ;i++)
			{
				if (reqCb.createdLockID[i] == UNUSED)
					break;
				else
					{
						if (reqCb.createdLockID[i] == lockID && reqCb.forciblyReleasedLockID[i] == 1)
							{
								//printf("\nlock_%d has been previously forcibly released..",lockID);
								forciblyReleased = 1;
								reqCb.forciblyReleasedLockID[i] = UNUSED;
								break;
							}
						
					}
			}
		}
				
		reqCbLock->Release();

		forceReleaseLock->Acquire();

		
			//Project 4 - Extra credit
			
	if (!failsafe || !forciblyReleased || (forciblyReleased && ServerLocks[lockID]->getOwnerNetworkID() != -1) ) 
	{
		ServerLock *lock = ServerLocks[lockID];
		
		Response *resp = lock -> Release(networkID, mailboxID);
		
		int RspCode;
		
		if(sendResponse)
		{
			RspCode = 1;
		}
		else
		{
			RspCode = 0;
		}
		
		msgStream << "LOCK_REL 1 OK " << RspCode;
		char response[MaxMailSize]; 
		strcpy(response,const_cast<char *>(msgStream.str().c_str()));
		
		outPktHdr.to = networkID;
		outMailHdr.to = mailboxID;
		outMailHdr.length = strlen(response) + 1;
		
		if(sendResponse)
		{
			postOffice -> Send(outPktHdr, outMailHdr, response);
		}	
		
		
		if((resp -> networkID != -1) && (resp -> mailboxID != -1) && !((resp -> networkID == networkID) && (resp -> mailboxID == mailboxID)))
		{
			printf("Releasing the response for blocked client with NID=%d and MID = %d", resp -> networkID, resp -> mailboxID);
			
			outPktHdr.to = resp -> networkID; // wake up a client blocked on the lock
			outMailHdr.to = resp -> mailboxID;
						
			int status = resp -> isError ? 0 : 1;
						
			msgStream.clear();
			msgStream.str("");
						
			msgStream << "LOCK_ACQ " << status << " " << resp -> response << endl;
						
						
			strcpy(response,const_cast<char *>(msgStream.str().c_str()));
			outMailHdr.length = strlen(response) + 1;
				
			// send the response to the client telling it to proceed
			postOffice -> Send(outPktHdr, outMailHdr, response);
						
		}
					
		delete resp;
					
		// perform cleanup
		// check if delete was called on the lock. If it was then delete the lock
		if((ServerLocks[lockID] -> deleteCalled) && (ServerLocks[lockID] -> refCount == 0))
		{
			delete ServerLocks[lockID];
			
			// clear the entry in the lock table
			lockBitMap.Clear(lockID);
			ServerLocks[lockID] = NULL;
		}
		
	}
	else if (failsafe)
	{
		forciblyReleased=0;
	}
		
		forceReleaseLock->Release();
				
	}
	else // lock belongs to some other server and we need to create a Request class object and push it in the incomingRequest queue
	{
		reqLock -> Acquire();
		int reqID = ReqBitMap.Find();
		
		Request *req = new Request(networkID, mailboxID);
		strcpy(req -> clientRequest, actualMsg);
		
		
		incomingRequest[reqID] = req;
		reqLock -> Release();
	}
}

void LockDestroy(char *lockName, char *actualMsg, int networkID, int mailboxID)
{
	PacketHeader outPktHdr;
	MailHeader outMailHdr;
	
	int lockID = FindLocks(lockName);
	
	stringstream msgStream (stringstream::in | stringstream::out);
	
	if((lockID == -1) && (serverCount == 1))
	{
		msgStream << "LOCK_DES 0 BadLock";
		char response[MaxMailSize] ;
		strcpy(response,const_cast<char *>(msgStream.str().c_str()));
			
		outPktHdr.to = networkID;
		outMailHdr.to = mailboxID;
		outMailHdr.length = strlen(response) + 1;
			
		postOffice -> Send(outPktHdr, outMailHdr, response);
		
		return;
	}
	
	if(lockID != -1)
	{
		if(ServerLocks[lockID] -> refCount == 0)
		{
			delete ServerLocks[lockID];
			// clear the entry in the lock table
			lockBitMap.Clear(lockID);
			ServerLocks[lockID] = NULL;
			
			//BroadcastToServers(actualMsg);
		}
		else
		{	
			ServerLocks[lockID] -> deleteCalled = true;
		}
	
		// send an ok status to the client
		msgStream << "LOCK_DES 1 OK"  << endl;
		char *response = const_cast<char *>(msgStream.str().c_str());
					
		outPktHdr.to = networkID;
		outMailHdr.to = mailboxID;
		outMailHdr.length = strlen(response) + 1;
					
		// send the response to client to unblock it
		postOffice -> Send(outPktHdr, outMailHdr, response);
	}
	else
	{
		reqLock -> Acquire();
		int reqID = ReqBitMap.Find();
		
		Request *req = new Request(networkID, mailboxID);
		strcpy(req -> clientRequest, actualMsg);
		
		
		incomingRequest[reqID] = req;
		reqLock -> Release();
	}
}

void ConditionWait(char *cvName, char *lName, char *actualMsg, int networkID, int mailboxID)
{
	PacketHeader outPktHdr;
	MailHeader outMailHdr;
	
	stringstream msgStream (stringstream::in | stringstream::out);
		
	msgStream.clear();
	msgStream.str("");
	
	char cv[20] = {'\0'};
	char lockName[20] = {'\0'};
	
	strcpy(cv,cvName);
	strcpy(lockName,lName);
	
	int cvID = FindCondition(cv);
	
	//cout << "The CV ID in wait is " << cvID << "and lockName passed is "<< lockName << endl;

	DEBUG('w',"The CVID in wait is %d and lockName passed is %s", cvID,lockName);
	
	if((cvID == -1) && (serverCount == 1))
	{
		msgStream << "CV_WT 0 BadCV";
		char response[MaxMailSize] ;
		strcpy(response,const_cast<char *>(msgStream.str().c_str()));
			
		outPktHdr.to = networkID;
		outMailHdr.to = mailboxID;
		outMailHdr.length = strlen(response) + 1;
			
		postOffice -> Send(outPktHdr, outMailHdr, response);
		
		return;
	}
	
	if(cvID != -1) // the cv belongs to the current server
	{
		//msgStream << "LOCK_REL " << lockName << " " << 0;
	
		char response[MaxMailSize];
		
		// release the associated lock
		
		if(ServerConditions[cvID] -> deleteCalled == true) 
		{
			//msgStream << "CV_WT 0 CVDeleted" << endl;
						
			//char *response = const_cast<char *>(msgStream.str().c_str());
				
			strcpy(response,"CV_WT 0 CVDeleted");
			//create an error response and send it to client
			outPktHdr.to = networkID;
			outMailHdr.to = mailboxID;
			outMailHdr.length = strlen(response) + 1;
			postOffice -> Send(outPktHdr, outMailHdr, response);
		}
		else
		{	// put the client on the condition wait queue
			sprintf(response,"LOCK_REL %s 0", lockName);
			ServerConditions[cvID] -> Wait(lockName, networkID, mailboxID);
			LockRelease(lockName, response, networkID, mailboxID, false);
		}			
	}
	else // the cv doesn't belong to current server
	{
		printf("Sending out a message CV_WT from wait to all the servers for CV %s\n", cvName);
		
		reqLock -> Acquire();
		int reqID = ReqBitMap.Find();
		
		Request *req = new Request(networkID, mailboxID);
		strcpy(req -> clientRequest, actualMsg);
		
		
		incomingRequest[reqID] = req;
		reqLock -> Release();
	}
}

void ConditionSignal(char *cvName, char *lockName, char *actualMsg, int networkID, int mailboxID)
{
	PacketHeader outPktHdr;
	MailHeader outMailHdr;
	
	int cvID = FindCondition(cvName);
	
	stringstream msgStream (stringstream::in | stringstream::out);
	
	if((cvID == -1) && (serverCount == 1))
	{
		msgStream << "CV_SIG 0 BadCV";
		char response[MaxMailSize] ;
		strcpy(response,const_cast<char *>(msgStream.str().c_str()));
			
		outPktHdr.to = networkID;
		outMailHdr.to = mailboxID;
		outMailHdr.length = strlen(response) + 1;
			
		postOffice -> Send(outPktHdr, outMailHdr, response);
		
		return;
	}
		
	if(cvID != -1) // the cv belongs to the current server
	{
		
		msgStream << "LOCK_ACQ " << lockName;
		// remove the client from condition variable wait queue
		Response *response = ServerConditions[cvID] -> Signal(lockName);
		
		
		// put it on the lock acquire queue
		
		if(response != NULL)
		{
			LockAcquire(lockName, const_cast<char *>(msgStream.str().c_str()), response -> networkID, response -> mailboxID);
			DEBUG('w',"Acquiring Lock in Signal,,for NID = %d  and MID = %d", response -> networkID, response -> mailboxID);
		}
		else
		{
			DEBUG('w',"Response is null");
		}
		
		msgStream.clear();
		msgStream.str("");
		
		if((ServerConditions[cvID] -> refCount == 0) && (ServerConditions[cvID] -> deleteCalled))
		{
			delete ServerConditions[cvID];
			CVBitMap.Clear(cvID);
			ServerConditions[cvID] = NULL;
		}
		
		msgStream << "CV_SIG 1 OK";
		char *resp = const_cast<char *>(msgStream.str().c_str());
		
		outPktHdr.to = networkID;
		outMailHdr.to = mailboxID;
		outMailHdr.length = strlen(resp) + 1;
		
		postOffice -> Send(outPktHdr, outMailHdr, resp);
	}
	else // the cv doesn't belong to current server
	{
		reqLock -> Acquire();
		int reqID = ReqBitMap.Find();
		
		Request *req = new Request(networkID, mailboxID);
		strcpy(req -> clientRequest, actualMsg);
		
		
		incomingRequest[reqID] = req;
		reqLock -> Release();
	}
}

void ConditionBroadcast(char *cvName, char *lockName, char *actualMsg, int networkID, int mailboxID)
{
	PacketHeader outPktHdr;
	MailHeader outMailHdr;
	
	int cvID = FindCondition(cvName);
	stringstream msgStream (stringstream::in | stringstream::out);
	
	if((cvID == -1) && (serverCount == 1))
	{
		msgStream << "CV_BCST 0 BadCV";
		char response[MaxMailSize] ;
		strcpy(response,const_cast<char *>(msgStream.str().c_str()));
			
		outPktHdr.to = networkID;
		outMailHdr.to = mailboxID;
		outMailHdr.length = strlen(response) + 1;
			
		postOffice -> Send(outPktHdr, outMailHdr, response);
		
		return;
	}
	
	if(cvID != -1) // the cv belongs to the current server
	{
		// remove the client from condition variable wait queue
		
		msgStream << "CV_SIG " << cvName << " " << lockName;
		
		while(!(ServerConditions[cvID] -> ConditionWaitQueue -> IsEmpty()))
		{
			ConditionSignal(cvName, lockName, const_cast<char *>(msgStream.str().c_str()), networkID, mailboxID);
		}
		
		if((ServerConditions[cvID] -> refCount == 0) && (ServerConditions[cvID] -> deleteCalled))
		{
			delete ServerConditions[cvID];
			CVBitMap.Clear(cvID);
			ServerConditions[cvID] = NULL;
		}			
	}
	else // the cv doesn't belong to current server
	{
		reqLock -> Acquire();
		int reqID = ReqBitMap.Find();
		
		Request *req = new Request(networkID, mailboxID);
		strcpy(req -> clientRequest, actualMsg);
		
		
		incomingRequest[reqID] = req;
		reqLock -> Release();
	}
}


void ConditionDestroy(char *cvName, char *actualMsg, int networkID, int mailboxID)
{
	PacketHeader outPktHdr;
	MailHeader outMailHdr;
	
	int cvID = FindCondition(cvName);
	
	stringstream msgStream (stringstream::in | stringstream::out);
	
	if((cvID == -1) && (serverCount == 1))
	{
		msgStream << "CV_DEL 0 BadCV";
		char response[MaxMailSize] ;
		strcpy(response,const_cast<char *>(msgStream.str().c_str()));
			
		outPktHdr.to = networkID;
		outMailHdr.to = mailboxID;
		outMailHdr.length = strlen(response) + 1;
			
		postOffice -> Send(outPktHdr, outMailHdr, response);
		
		return;
	}
	
	
	if(cvID != -1)
	{
		DEBUG('w',"The CV is %s with id %d and re count = %d", cvName, cvID, ServerConditions[cvID] -> refCount);
		
		if(ServerConditions[cvID] -> refCount == 0)
		{
			delete ServerConditions[cvID];
			// clear the entry in the condition table
			CVBitMap.Clear(cvID);
			ServerConditions[cvID] = NULL;
			
			//BroadcastToServers(actualMsg);
		}
		else
		{	
			ServerConditions[cvID] -> deleteCalled = true;
		}
	
		// send an ok status to the client
		msgStream << "CV_DEL 1 OK"  << endl;
		char *response = const_cast<char *>(msgStream.str().c_str());
					
		outPktHdr.to = networkID;
		outMailHdr.to = mailboxID;
		outMailHdr.length = strlen(response) + 1;
					
		// send the response to client to unblock it
		postOffice -> Send(outPktHdr, outMailHdr, response);
	}
	else
	{
		reqLock -> Acquire();
		int reqID = ReqBitMap.Find();
		
		Request *req = new Request(networkID, mailboxID);
		strcpy(req -> clientRequest, actualMsg);
		
		
		incomingRequest[reqID] = req;
		reqLock -> Release();
	}
}

void MonitorGet(char *mvName, char *actualMsg, int networkID, int mailboxID)
{

	PacketHeader outPktHdr;
	MailHeader outMailHdr;
	stringstream msgStream (stringstream::in | stringstream::out);
	
	int mvID = FindMV(mvName);
	
	if((mvID == -1) && (serverCount == 1))
	{
		msgStream << "MV_GET 0 BadMV";
		char response[MaxMailSize] ;
		strcpy(response,const_cast<char *>(msgStream.str().c_str()));
			
		outPktHdr.to = networkID;
		outMailHdr.to = mailboxID;
		outMailHdr.length = strlen(response) + 1;
			
		postOffice -> Send(outPktHdr, outMailHdr, response);
		
		return;
	}
	
	if(mvID != -1)
	{
	
		//msgStream << "MV_GET 1 " << MonitorVariables[mvID] -> getValue() << endl;
		char response[MaxMailSize];
		
		sprintf(response,"MV_GET 1 %d", MonitorVariables[mvID] -> getValue());
				
		//char *response = const_cast<char *>(msgStream.str().c_str());
		
		DEBUG('w',"MV_GET response is : %s :", response);
				
		// send te response to the client
		outPktHdr.to = networkID;
		outPktHdr.from = netname;
		outMailHdr.to = mailboxID;
		outMailHdr.length = strlen(response) + 1;
				
		postOffice -> Send(outPktHdr, outMailHdr, response);
	}
	else
	{
		reqLock -> Acquire();
		int reqID = ReqBitMap.Find();
		
		Request *req = new Request(networkID, mailboxID);
		strcpy(req -> clientRequest, actualMsg);
		
		
		incomingRequest[reqID] = req;
		incomingRequest[reqID] -> isPending = true;
		reqLock -> Release();
		
		DEBUG('w',"Created the request in MV_GET, %d", reqID);
	}
}

void MonitorSet(char *mvName, int val, char *actualMsg, int networkID, int mailboxID)
{
	PacketHeader outPktHdr;
	MailHeader outMailHdr;
	stringstream msgStream (stringstream::in | stringstream::out);
	
	DEBUG('w',"\nThe mmonitor name is %s\n", mvName);
	
	int mvID = FindMV(mvName);
	
	if((mvID == -1) && (serverCount == 1))
	{
		msgStream << "MV_SET 0 BadMV";
		char response[MaxMailSize] ;
		strcpy(response,const_cast<char *>(msgStream.str().c_str()));
			
		outPktHdr.to = networkID;
		outMailHdr.to = mailboxID;
		outMailHdr.length = strlen(response) + 1;
			
		postOffice -> Send(outPktHdr, outMailHdr, response);
		
		return;
	}
	
	if(mvID != -1)
	{
	
		DEBUG('w',"MV_SET value is %d", val);
		//msgStream << "MV_SET 1 " << MonitorVariables[mvID] -> setValue(val) << endl;
				
		//char *response = const_cast<char *>(msgStream.str().c_str());
		
		char response[MaxMailSize];
		
		sprintf(response,"MV_SET 1 %d", MonitorVariables[mvID] -> setValue(val));
		
		
		DEBUG('w',"MV_SET response is : %s :", response);
		// send te response to the client
		outPktHdr.to = networkID;
		outMailHdr.to = mailboxID;
		outMailHdr.length = strlen(response) + 1;
				
		postOffice -> Send(outPktHdr, outMailHdr, response);
	}
	else
	{
		reqLock -> Acquire();
		int reqID = ReqBitMap.Find();
		
		Request *req = new Request(networkID, mailboxID);
		strcpy(req -> clientRequest, actualMsg);
		
		
		incomingRequest[reqID] = req;
		reqLock -> Release();
	}
}

void MonitorDestroy(char *mvName, char *actualMsg, int networkID, int mailboxID)
{
	PacketHeader outPktHdr;
	MailHeader outMailHdr;

	stringstream msgStream (stringstream::in | stringstream::out);
	int mvID = FindMV(mvName);
	
	if((mvID == -1) && (serverCount == 1))
	{
		msgStream << "MV_DES 0 BadMV";
		char response[MaxMailSize] ;
		strcpy(response,const_cast<char *>(msgStream.str().c_str()));
			
		outPktHdr.to = networkID;
		outMailHdr.to = mailboxID;
		outMailHdr.length = strlen(response) + 1;
			
		postOffice -> Send(outPktHdr, outMailHdr, response);
		
		return;
	}
	if(mvID != -1)
	{
	
		delete MonitorVariables[mvID];
		MVBitMap.Clear(mvID);
		MonitorVariables[mvID] = NULL;
						
		msgStream << "MV_DES 1 OK";
		
		char *response = const_cast<char *>(msgStream.str().c_str());
				
		// send te response to the client
		outPktHdr.to = networkID;
		outMailHdr.to = mailboxID;
		outMailHdr.length = strlen(response) + 1;
				
		postOffice -> Send(outPktHdr, outMailHdr, response);
		
		//BroadcastToServers(actualMsg);
	}
	else
	{
		reqLock -> Acquire();
		int reqID = ReqBitMap.Find();
		
		DEBUG('w',"in MV_GET, the reqID is %d", reqID);
		
		Request *req = new Request(networkID, mailboxID);
		strcpy(req -> clientRequest, actualMsg);
		
		
		incomingRequest[reqID] = req;
		reqLock -> Release();
		
		DEBUG('w',"Created the request in MV_GET, %d", reqID);
	}
}

int FindPendingRequest()
{
	//reqLock -> Acquire();
	
	for(int idx = 0; idx < 1500; idx++)
	{
		
		
		if(incomingRequest[idx] == NULL)
		{
			//printf("\n incoming request for %d is null\n", idx);
			continue;
		}
		
		if(incomingRequest[idx] -> isPending)
		{
			DEBUG('w',"\npending true for id %d", idx);
			
			incomingRequest[idx] -> isPending = false;
			//reqLock -> Release();
			return idx;
		}
	}
	
	return -1;
}

void Dispatch(PacketHeader outPktHdr, MailHeader outMailHdr, char *msg)
{
	postOffice -> Send(outPktHdr, outMailHdr, msg);
}

void BroadcastToServers(char *msg)
{
	PacketHeader outPktHdr;
    MailHeader outMailHdr;
	
	outMailHdr.to = 2;
	outMailHdr.from = 2;
	outMailHdr.length = strlen(msg) + 1;
	char *buffer = new char[MaxMailSize];
	strcpy(buffer,msg);
	
	DEBUG('w',"Broadcasting to all the servers a message with length %d\n", outMailHdr.length);
	
	for(int servIdx = 0; servIdx < serverCount; servIdx++)
	{
		printf("trying to send to %d and netName is : %d\n", servIdx, netname);
		
		// send it to all the servers except yourself
		if(servIdx != netname)
		{
			printf("sending to server with machine id %d", servIdx);
			
			outPktHdr.to = servIdx; 
			outPktHdr.from = netname;
			
			//Dispatch(outPktHdr, outMailHdr, msg);
			
			postOffice -> Send(outPktHdr, outMailHdr, buffer);
		}
	}
}

void ServerSend(int i)
{
	PacketHeader outPktHdr;
    MailHeader outMailHdr;
	struct timeval timestamp;
    char buffer[MaxMailSize];
	char requestType[30];
	char msg[MaxMailSize];
	long mseconds;
	
	//stringstream msgStream (stringstream::in|stringstream::out);
	
	outMailHdr.to = 2; // The server to server communication is done on mailbox id 2
	outMailHdr.from = 2;
	
	printf("starting up the server send thread\n");
	
	while(true)
	{
		//read the request from the request queue and select the first pending request
		reqLock -> Acquire();
		
		int currentReq = FindPendingRequest();
		
		if(currentReq != -1)
		{
		
			//msgStream.clear();
			//msgStream.str("");
			printf("Here I am %d\n", currentReq);
			//reqLock -> Acquire();
			//msgStream << incomingRequest[currentReq] -> clientRequest;
			//msgStream >> requestType;
			
			sscanf(incomingRequest[currentReq] -> clientRequest, "%s", requestType);
		
			//reqLock -> Release();
		
			printf("Sending the request of type : %s\n", requestType);
		
			//find if the request is a create request
			if(!strcmp(requestType,"LOCK_CREATE") || !strcmp(requestType, "CV_CREATE") || !strcmp(requestType, "MV_CREATE"))
			{	
				// add timestamp to the message
				gettimeofday(&timestamp, NULL);
				mseconds = timestamp.tv_usec /  1000;
				printf("\n the timestamp is %ld\n", mseconds);
			//mseconds = mseconds / (3600 * 1000); // assuming the simulation will not run for more than an hour
			
			//maintain the timestamp record for the message
				incomingRequest[currentReq] -> timestamp = mseconds;
			
				// prepare the message to the servers
				//msgStream.clear();
				//msgStream.str("");
			
			// put the new message in the message stream
			
				printf("\nthe client request is : %s\n", incomingRequest[currentReq] -> clientRequest);
			
				//msgStream << incomingRequest[currentReq] -> clientRequest << " " << mseconds << " " << currentReq;
				sprintf(msg,"%s %d %d",incomingRequest[currentReq] -> clientRequest,(int)mseconds,currentReq);
				//msg = const_cast<char *>(msgStream.str().c_str()); // we have the desired message here
			
			// send to all other servers
				BroadcastToServers(msg);
			
			}
			else // these are operations on lock , cv and mv. We create a generic message like PARAMTYPE<space>PARAMNAME
			{	// which will mean that do you have a parameter with the paramname specified
			
				char name[20];
				//msgStream >> name;
				sscanf(incomingRequest[currentReq] -> clientRequest, "%s %s", requestType, name);
			
				//msgStream.clear();
				//msgStream.str("");
			
				if((!strcmp(requestType,"LOCK_ACQ")) || (!strcmp(requestType,"LOCK_REL")) || (!strcmp(requestType,"LOCK_DES")))
				{
					//msgStream << "LOCK " << name << " " << currentReq;
					sprintf(msg,"LOCK %s %d", name, currentReq);
				}
				else if((!strcmp(requestType,"CV_WT")) || (!strcmp(requestType,"CV_SIG")) || (!strcmp(requestType,"CV_BCST")) || (!strcmp(requestType,"CV_DEL")))
				{
					//msgStream << "CV " << name << " " << currentReq;
					sprintf(msg,"CV %s %d", name, currentReq);
				}
				else if((!strcmp(requestType,"MV_GET")) || (!strcmp(requestType, "MV_SET")) || (!strcmp(requestType, "MV_DES")))
				{
					//msgStream << "MV " << name << " " << currentReq;
					sprintf(msg, "MV %s %d", name, currentReq);
				}
			
				//msg = const_cast<char *>(msgStream.str().c_str());
			
				// send to all other servers
				BroadcastToServers(msg);
				
				//delete[] name;
			}
		}
		
		reqLock -> Release();
		//delete[] requestType;
		currentThread->Yield();
	}
}


void HandleCreateRequest(int status,int msgID)
{
	//stringstream msgStream (stringstream::in | stringstream :: out);
	MailHeader outMailHdr;
	PacketHeader outPktHdr;
	
	reqLock -> Acquire();
	
	if(status == 0)
	{
		incomingRequest[msgID] -> negResponse++;
	}
	
	incomingRequest[msgID] -> responseCount++;
	
	if(incomingRequest[msgID] -> responseCount == serverCount - 1)
	{
		
		if(incomingRequest[msgID] -> negResponse == serverCount - 1)
		{
			// read the message and create the param
			
			char requestType[20];
			char name[10];
			
			//msgStream << incomingRequest[msgID] -> clientRequest;
			sscanf(incomingRequest[msgID] -> clientRequest,"%s %s", requestType,name);
			//msgStream >> requestType;
				
			//msgStream >> name;
				
			outPktHdr.to = incomingRequest[msgID] -> response -> networkID;
			outPktHdr.from = netname;
			outMailHdr.to = incomingRequest[msgID] -> response -> mailboxID;
			outMailHdr.from = 0;
			
			if(!strcmp(requestType,"LOCK_CREATE"))
			{
				CreateLock(name, outPktHdr, outMailHdr, msgID);
			}
			else if(!strcmp(requestType, "CV_CREATE"))
			{
				CreateCondition(name, outPktHdr, outMailHdr, msgID);
			}
			else if(!strcmp(requestType, "MV_CREATE"))
			{
				CreateMonitor(name, outPktHdr, outMailHdr, msgID);
			}
			
			//delete[] name;
		//	delete[] requestType;
			
		}
		else
		{
			// return back to the client giving it an impression that the lock was created
			
			char response[MaxMailSize];
			
			strcpy(response,"LOCK_CREATE 1 OK");
			//response = const_cast<char *>(msgStream.str().c_str());
			outPktHdr.to = incomingRequest[msgID] -> response -> networkID;
			outMailHdr.to = incomingRequest[msgID] -> response -> mailboxID;
			outMailHdr.from = 0;
			outMailHdr.length = strlen(response) + 1;
			
			postOffice -> Send(outPktHdr, outMailHdr, response);
		}
		
		printf("Deleting the incoming request %d\n", msgID);
		
		incomingRequest[msgID] -> isPending = false;
		delete incomingRequest[msgID];
		incomingRequest[msgID] = NULL;

		ReqBitMap.Clear(msgID);
		
		reqLock -> Release();
	}
}

int MatchRequest(char *request)
{
	int reqID = -1;
	reqLock -> Acquire();
	
	for(int idx = 0; (incomingRequest[idx] != NULL) && (idx < 500); idx++)
	{
		if((incomingRequest[idx] -> isPending) && (!strcmp(incomingRequest[idx] -> clientRequest, request)))
		{
			reqID = idx;
			break;
		}
	}
	
	reqLock -> Release();
	return reqID;
}

long SearchRequestTable(char *msgType, char* paramName)
{
	stringstream msgStream (stringstream::in | stringstream::out);
	
	char *reqType = new char[20];
	char *name = new char[20];
	
	reqLock -> Acquire();
	
	// check if we have a non complete request with the same request type and same parameter name
	for(int idx = 0; (incomingRequest[idx] != NULL) && (idx < 1500) && (incomingRequest[idx] -> responseCount < serverCount - 1); idx++)
	{
		msgStream.clear();
		msgStream.str("");
		
		msgStream << incomingRequest[idx] -> clientRequest;
		msgStream >> reqType >> name;
		
		if((!strcmp(reqType, msgType)) && (!strcmp(name, paramName))) // we have found an exact request
		{
			// return the timestamp of the request
			reqLock -> Release();
			return incomingRequest[idx] -> timestamp;
		}
	}
	
	reqLock -> Release();
	
	delete[] reqType;
	delete[] name;
	// this server doesn't have any request matching the incoming request
	return -1;
}

void ServerReceive(int i)
{
	PacketHeader inPktHdr,outPktHdr;
    MailHeader inMailHdr, outMailHdr;
	
	char buffer[MaxMailSize];
	//stringstream msgStream (stringstream::in | stringstream::out);
	char msgType[15];
	
	while(true)
	{
		postOffice -> Receive(2, &inPktHdr, &inMailHdr, buffer);
		
		printf("In server receive\n");
		
		
		// figure out if it is a response to a request or a request itself
		// response will begin with key word RSP
		
		//msgStream.clear();
		//msgStream.str("");
		
		//msgStream << buffer;
		//msgStream >> msgType;
		
		sscanf(buffer,"%s", msgType);
		
		printf("Received message : %s : from NID = %d and MID = %d\n", buffer, inPktHdr.from, inMailHdr.from);
		
		if(strcmp(msgType,"RSP") && strcmp(msgType,"RSPC")) // its not a response, its a request
		{
			// check the type of request. Broadly categorized into create and other requests
			
			//printf("In server Receive request for creation of param : %s\n", msgType);
			// check if its a create request
			if(!strcmp(msgType,"LOCK_CREATE") || (!strcmp(msgType, "CV_CREATE")) || (!strcmp(msgType, "MV_CREATE")))
			{	
				long timestamp;
				int msgID;
				char paramName[20];
				//msgStream >> paramName;
				
				sscanf(buffer,"%s %s %ld %d", msgType,paramName, &timestamp, &msgID);
				printf("In server Receive, request for creation of param : %s with name %s\n", msgType, paramName);
				
				bool sendResponse = false;
				
				int pos = -1;
				
				if(!strcmp(msgType, "LOCK_CREATE"))
				{
					pos = FindLocks(paramName);
				}
				else if(!strcmp(msgType, "CV_CREATE"))
				{
					pos = FindCondition(paramName);
				}
				else if(!strcmp(msgType, "MV_CREATE"))
				{
					pos = FindMV(paramName);
				}
						
				//msgStream >> timestamp >> msgID;
					
				if(pos == -1) // we don't have the synch param created
				{
					// check if we have a request for the same synch param . If we have fetch the timestamp of the request
					
					long reqTimestamp = SearchRequestTable(msgType, paramName);
					
					char response[MaxMailSize] = {'\0'};
					
					if(reqTimestamp != -1) // this server has a similar non-completed request
					{
						//msgStream.clear();
						//msgStream.str("");
						
						if(timestamp < reqTimestamp) // the incoming request has a lower timestamp than the stored request
						{
							//msgStream << "RSPC 1 " << msgID << endl;
							//sendResponse = true;
							sprintf(response, "RSPC 0 %d ", msgID);
						}
						else if(timestamp > reqTimestamp)
						{
							//msgStream << "RSPC 0 " << msgID << endl; 
							sprintf(response, "RSPC 1 %d ", msgID);
							// we should queue the response here.
							// This is a hack but I couldn't figure out a better way to accomplish it
							
							//Response *queueResponse = new Response(inPktHdr.from, inMailHdr.from, false, "");
							//incomingRequest[msgID].rsp -> Append((void *) queueResponse);
						}
						else if(timestamp == reqTimestamp)
						{
							if(inPktHdr.from < netname) // the machine id of this server is greater than the requesting server
							{
								//msgStream << "RSPC 1 " << msgID << endl;
								sprintf(response,"RSPC 0 %d ", msgID);
								sendResponse = true;
							}							
							else // else this server has a priority over requesting server
							{
								//msgStream << "RSPC 0 " << msgID << endl;
								sprintf(response,"RSPC 1 %d", msgID);
								// queue the response here
								//Response *queueResponse = new Response(inPktHdr.from, inMailHdr.from, false, "");
								//incomingRequest[msgID].rsp -> Append((void *) queueResponse);
							}
						}
						
						
						outPktHdr.to = inPktHdr.from;
						outMailHdr.to = inMailHdr.from;
						outPktHdr.from = netname;
						outMailHdr.from = 2;
						outMailHdr.length = strlen(response);
						
						Dispatch(outPktHdr, outMailHdr, response);
						
					}
					else // we don't have any such create request
					{
						//msgStream.clear();
						//msgStream.str("");
						
						//msgStream << "RSPC 0 " << msgID << endl;
						//char response[MaxMailSize];
						//response = const_cast<char *>(msgStream.str().c_str());
						
						sprintf(response,"RSPC 0 %d ", msgID);
						outPktHdr.to = inPktHdr.from;
						outPktHdr.from = netname;
						outMailHdr.to = inMailHdr.from;
						outMailHdr.from = 2;
						outMailHdr.length = strlen(response);
						
						Dispatch(outPktHdr, outMailHdr, response);
					}
					
				}
				else // we have the synch param
				{
					char response[MaxMailSize];
					
					sprintf(response, "RSPC 1 %d ", msgID);
					
					outPktHdr.to = inPktHdr.from;
					outPktHdr.from = netname;
					outMailHdr.to = inMailHdr.from;
					outMailHdr.from = 2;
					outMailHdr.length = strlen(response);
						
					postOffice -> Send(outPktHdr, outMailHdr, response);
					
				}
				
				//delete[] paramName;
			}
			else
			{ // its a generic "do you have request"
			
				char name[15];
				char response[MaxMailSize];
					
				//msgStream >> name;
				int msgID;
				//msgStream >> msgID;

				//msgStream.clear();
				//msgStream.str("");
				
				sscanf(buffer, "%s %s %d", msgType,name, &msgID);
					
				if(!strcmp(msgType, "LOCK"))
				{
					if(FindLocks(name) == -1) // the lock is not available
					{
						//msgStream << "RSP 0 " << msgID << endl;
						sprintf(response,"RSP 0 %d ", msgID);
					}
					else
					{
						//msgStream << "RSP 1 " << msgID << endl;
						sprintf(response,"RSP 1 %d ", msgID);
					}
				}
				else if(!strcmp(msgType, "CV"))
				{
					if(FindCondition(name) == -1) // the CV is not available
					{
						sprintf(response,"RSP 0 %d ", msgID);
					}
					else
					{
						//msgStream << "RSP 1 " << msgID << endl;
						sprintf(response,"RSP 1 %d ", msgID);
					}
				}
				else if(!strcmp(msgType, "MV"))
				{
					if(FindMV(name) == -1) // the MV is not available
					{
						sprintf(response,"RSP 0 %d ", msgID);
					}
					else
					{
						//msgStream << "RSP 1 " << msgID << endl;
						sprintf(response,"RSP 1 %d ", msgID);
					}
				}
				
				//char *response;
				//response = const_cast<char *>(msgStream.str().c_str());
						
				outPktHdr.to = inPktHdr.from;
				outPktHdr.from = netname;
				outMailHdr.to = inMailHdr.from;
				outMailHdr.from = 2;
				outMailHdr.length = strlen(response);
						
				Dispatch(outPktHdr, outMailHdr, response);
			}
		}
		else if(!strcmp(msgType, "RSPC")) // its a response to a create message
		{ // we have responses from other clients. The responses can be general(acquire, release, wait, signal ) or for create message
		  // create requests will have to wait for positive responses from all servers to create
		  
		  int status, msgID;
		  
		  //msgStream >> status >> msgID;
		  sscanf(buffer,"%s %d %d", msgType, &status, &msgID);
		  
		  DEBUG('w',"####Received an RSPC message with status %d from NID %d and MID %d\n", status, inPktHdr.from, inMailHdr.from);
		  
		  HandleCreateRequest(status, msgID);
		}
		else if(!strcmp(msgType,"RSP"))
		{
			int status, msgID;
			
			//msgStream >> status >> msgID;
			sscanf(buffer,"%s %d %d", msgType, &status, &msgID);
			
			if(status == 1)
			{
				incomingRequest[msgID] -> owner = new Response(inPktHdr.from, inMailHdr.from, false, "");
			}
			else
			{
				incomingRequest[msgID] -> negResponse++;
			}
			
			incomingRequest[msgID] -> responseCount++;
			
			if((incomingRequest[msgID] -> responseCount == serverCount - 1))
			{
				// *********** need to take care of bad parameters here *************************
				if(incomingRequest[msgID] -> negResponse == serverCount -1)
				{
					outPktHdr.to = incomingRequest[msgID] -> response -> networkID;
					outPktHdr.from = netname;
					outMailHdr.to = incomingRequest[msgID] -> response -> mailboxID;
					outMailHdr.from = incomingRequest[msgID] -> response -> mailboxID;
					
					char res[MaxMailSize];
					char reqType[MaxMailSize];
					
					sscanf(incomingRequest[msgID] -> clientRequest,"%s",reqType);
					
					sprintf(res,"%s 0 BadInput", reqType);
					
					outMailHdr.length = strlen(res) + 1;
					
					postOffice -> Send(outPktHdr, outMailHdr,res);
				}
				else // the input param was correct
				{
				
				
				//outPktHdr.from = incomingRequest[msgID] -> response -> networkID;
				
					outPktHdr.to = incomingRequest[msgID] -> owner -> networkID;
					outPktHdr.from = incomingRequest[msgID] -> response -> networkID;
					outMailHdr.from = incomingRequest[msgID] -> response -> mailboxID;
					outMailHdr.to = 0;
				
				
				//msgStream.clear();
				//msgStream.str("");
				
				//msgStream << incomingRequest[msgID] -> clientRequest << " " << incomingRequest[msgID] -> response -> networkID;
					char res[MaxMailSize]; //= const_cast<char *>(msgStream.str().c_str());
				
					sprintf(res,"%s", incomingRequest[msgID] -> clientRequest);
				//strcat(res,"\0");
					outMailHdr.length = strlen(res) + 1;
					DEBUG('w',"Sending a message from generic parameter handling with NID = %d and MID = %d", outPktHdr.from, outMailHdr.from);
					DEBUG('w',"the response is : %s :", res);
				
					reqLock -> Acquire();
					postOffice -> Send(outPktHdr, outMailHdr, res);
				
					incomingRequest[msgID] -> isPending = false;
					delete incomingRequest[msgID];
					incomingRequest[msgID] = NULL;

					ReqBitMap.Clear(msgID);
					
					reqLock -> Release();
				}
		
			}
		}
		currentThread->Yield();
	}
}




/*
* This is the function which will be pinging the clients - who are lock owners -
* and see if they have fallen asleep or not 
* This thread needs to be such that it has a separate maoilbox number
* We need dummy argument here because Timer functions are such.. 
*
*/
int timeElapsed;

void Timer_PingClients(int dummy)
{
	
	PacketHeader outPktHdrPing, inPktHdrPing;
    MailHeader outMailHdrPing, inMailHdrPing;
    //char buffer[MaxMailSize];
	stringstream msgStreamPing (stringstream::in|stringstream::out);
	stringstream msgStream (stringstream::in|stringstream::out);
	//char *requestType;
	//BitMap lockBitMap(250);
	//BitMap CVBitMap(250);
	//BitMap MVBitMap(250);
	char testMsgPing[MaxMailSize];
	char *response;
	
	
	//Creating a dummy test message to send to our client 
	//msgStreamPing << " Testing... ";	
	//testMsgPing = const_cast<char *>(msgStreamPing.str().c_str());
	
	strcpy(testMsgPing,"Testing");
	
	//initializeReqCb();
	
	//while (true) {
		 /* 
		 * we keep on checking for dead clients right through server's life 
		 *  - we should do that only if atleast if one client is connected
		 */
		 
		//We otain the current time and compare it to the last ping time 
		
		currentTime = time(NULL);
		timeElapsed = currentTime - lastPingTime;
		
		//printf("\n JASPR lastPingTime was %s\n",ctime(&lastPingTime));
		//printf("\n JASPR Time elapsed is %d",timeElapsed);
		
		if (timeElapsed >= 5) {
		//printf("\nInside Timer_PingClients --");
		//printf("-- JASPR currentTime is %s\n",ctime(&currentTime));
		
		//printf("\n time elapsed should have now reached 5");
		lastPingTime = currentTime;
		//printf("\n JASPR lastPingTime was %s\n",ctime(&lastPingTime));

		/*
		* if 5 or more seconds have come and gone since the 
		* since we last checked for dead clients, we shall go 
		* ahead and do it again 
		*/
		
		
			/*
			*	We shall be checking for the list of owners of busy owners to 
			*	find the clients we need to be pinging
			*
			*/
		
			//traversing list of server locks
			int idx = -1;
				
			//we are traversing whole list everytime and not simply till we get an unassigned lockID 
			//printf("\n JASPR Will now acquire lock for reqCb");
			reqCbLock->Acquire();

			for (idx=0; idx<LOCK_SIZE; idx++ )
			{
				//printf("\n JASPR Inside for loop - iteration number %d",idx);
				
				if (reqCb.createdLockID[idx] != UNUSED)
				{
					//printf("\n JASPR Inside if in the for loop - iteration number %d",idx);
					
					if ((ServerLocks[reqCb.createdLockID[idx]] -> GetLockState() == BUSY) )
					//find busy locks  that were created by this server itself
					{
						forceReleaseLock->Acquire();
						printf("\n The lock is busy..");
						//zero in on the lock's owner and ping it
						//lock's acquistion shall only be made at lock creating server
						
						//printf("\n JASPR Owner mailbox id is %d and its network id is %d \n",ServerLocks[reqCb.createdLockID[idx]] -> getOwnerMailBoxID(), ServerLocks[reqCb.createdLockID[idx]] -> getOwnerNetworkID());
						
						outPktHdrPing = reqCb.requestPacketHeader[idx];
						outMailHdrPing = reqCb.requestMailHeader[idx];
						

						//outPktHdrPing.to = outPktHdrPing.from;
						outPktHdrPing.to = ServerLocks[reqCb.createdLockID[idx]] -> getOwnerNetworkID();
						outMailHdrPing.to = clientPingMailBoxId;
						
						outMailHdrPing.from = serverPingMailBoxId; 
						outPktHdrPing.from = netname;//this server's Id
						outMailHdrPing.length = strlen(testMsgPing);
						
						//printf("Sending the ping message to %d on mailbox %d\n", outPktHdrPing.to, outMailHdrPing.to);
						//printf("\n Am sending request to %d",outMailHdrPing.to);
						if ((postOffice -> Send(outPktHdrPing, outMailHdrPing, testMsgPing))== false) { 
						//We failed to send our message - we must now change lock ownership or free the lock 
							printf("\nOwner of lock_%d (client with network_id=%d) was found to be dead..\n",reqCb.createdLockID[idx],outPktHdrPing.to);
							ServerLock *lock = ServerLocks[idx];
							//Response *resp = ServerLocks[idx] -> Release(outPktHdrPing.to, reqCb.requestMailHeader[idx].from);
							Response *resp = lock -> Release(outPktHdrPing.to, reqCb.requestMailHeader[idx].to);
							//Response *resp = lock -> Release(outPktHdrPing.to, outMailHdrPing.to);
							//printf("\n\n@@@@@@@@@reqCb.requestMailHeader[idx].from=%d@@@@@\n\n",reqCb.requestMailHeader[idx].from);
							//printf("Response is %s", &resp);
							//printf("\n\n\nJASPR Released from network_id=%d on mailbox %d\n", outPktHdrPing.to, outMailHdrPing.to);
							
							reqCb.forciblyReleasedLockID[idx] = 1;//the lock at index idx has been forcibly released
							//reqCb.forciblyReleasedLockID[idx]);
							forceReleaseLock->Release();
							reqCbLock->Release();
							
							/*
							//printf("\nThe response generated at release is\n Network ID : %d MailboxID : %d\n", resp -> networkID, resp -> mailboxID);
							msgStream << requestType << " 1 " << "OK" << endl;
							response = const_cast<char *>(msgStream.str().c_str());
							outPktHdr.to = inPktHdr.from;
							outMailHdr.to = inMailHdr.from;
							outMailHdr.length = strlen(response) + 1;
						
							// send the response to the client telling it to proceed
							postOffice -> Send(outPktHdr, outMailHdr, response);
							
							//printf("\nThe response generated at release is\n Network ID : %d MailboxID : %d\n", resp -> networkID, resp -> mailboxID);
							*/
							if((resp -> networkID != -1) && (resp -> mailboxID != -1))
							{
								//printf("Releasing the response for blocked client");
								PacketHeader pkHdr;
								MailHeader mailHdr;
								
								pkHdr.to = resp -> networkID; // wake up a client blocked on the lock
								mailHdr.to = resp -> mailboxID;
								
								//printf("\n JASPR mailHdr.to=%d, pkHdr.to=%d",mailHdr.to,pkHdr.to);
								
								int status = resp -> isError ? 0 : 1;
								
								msgStream.clear();
								msgStream.str("");
								
								msgStream << "LOCK_ACQ " << status << resp -> response << endl;
								
								
								response = const_cast<char *>(msgStream.str().c_str());
								mailHdr.length = strlen(response) + 1;
						
								// send the response to the client telling it to proceed
								postOffice -> Send(pkHdr, mailHdr, response);
								
							}
							
							delete resp;
							
							/*
							// perform cleanup
							// check if delete was called on the lock. If it was then delete the lock
							if((ServerLocks[lockID] -> deleteCalled) && (ServerLocks[lockID] -> refCount == 0))
							{
								delete ServerLocks[lockID];
								// clear the entry in the lock table
								lockBitMap.Clear(lockID);
								ServerLocks[lockID] = NULL;
							}

								return;
							*/
						}
						
					}
				}
				else
				{
					//printf("no locks created..");
					break;
				}
			}
			forceReleaseLock->Release();
			reqCbLock->Release();
		
		}
	
		//currentThread->Yield();

	//}

}

void Ping_Clients()
{
	Timer *pingTimer = new Timer((VoidFunctionPtr) Timer_PingClients, 0, false);
	currentThread->Yield();
}
void RunServer(int p)
{
	PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char buffer[MaxMailSize];
	stringstream msgStream (stringstream::in|stringstream::out);
	char *requestType;
	char *response;
	
	//For Project 4 - Extra credit 
	/*
		We will start a new thread that will be pinging our lock owners so as to make sure 
		that if any one dies, the lock is transfered to the next one in line..
	*/
	//printf("\n JASPR failsafe is");
	if (failsafe) 
	{
		Thread *newThread = new Thread("pingClients");	
		//newThread -> space = currentThread -> space;
		newThread -> Fork((VoidFunctionPtr) Ping_Clients, 0);
	}
	
	
	// server runs for ever
	while(true)
	{
		// wait for a request to come
		printf("\nMain run waiting on receive\n");
		//buffer = new char[MaxMailSize];
		postOffice -> Receive(0,&inPktHdr, &inMailHdr, buffer);
		printf("\nReceived a message with request %s from NID %d and MID %d\n", buffer, inPktHdr.from, inMailHdr.from);
		// find the request type
		
		msgStream.clear();
		msgStream.str("");
		msgStream << buffer;
		requestType = new char[50];
		msgStream >> requestType;
		
		if(!strcmp(requestType,"LOCK_CREATE"))
		{
			char lockName[20];
			
			msgStream >> lockName;
			DEBUG('w',"The lock name is %s\n", lockName);
			
			int lockID = FindLocks(lockName);
			
			if((serverCount == 1) && (lockID == -1))
			{
				outPktHdr.to = inPktHdr.from;
				outPktHdr.from = netname;
				outMailHdr.to = inMailHdr.from;
				outMailHdr.from = 0;
				
				CreateLock(lockName, outPktHdr, outMailHdr,-1);
			}
			else
			{
				if(lockID == -1)
				{
					// find if a lock is available
				
					int reqID = MatchRequest(buffer); 
					if(reqID == -1)
					{
						reqLock -> Acquire();
						reqID = ReqBitMap.Find();
		
						Request *req = new Request(inPktHdr.from, inMailHdr.from);
						strcpy(req -> clientRequest,buffer);
		
						
						incomingRequest[reqID] = req;
						reqLock -> Release();
					}
					else
					{
						reqLock -> Acquire();
						incomingRequest[reqID] -> queuedList -> Append((void *)new Response(inPktHdr.from, inMailHdr.from, false, ""));
						reqLock -> Release();
					}
				}
				else
				{
					char res[MaxMailSize];
					
					sprintf(res,"LOCK_CREATE 1 %d ", lockID);
					//response = const_cast<char *>(msgStream.str().c_str());
			
					outPktHdr.to = inPktHdr.from;
					outMailHdr.to = inMailHdr.from;
					outMailHdr.length = strlen(res) + 1;
			
					DEBUG('w',"The message was received from machine with network address %d and mailboxid %d\n", outPktHdr.to, outMailHdr.to);
			
					postOffice -> Send(outPktHdr, outMailHdr, res);
				}
			}
			
			//delete[] lockName;
		}	
		else if(!strcmp(requestType,"LOCK_ACQ"))
		{
			char lockName[20];
			
			int networkID = -1;
			msgStream >> lockName;
			
			//if(networkID == -1)
			{
				networkID = inPktHdr.from;
			}
						
			msgStream.clear();
			msgStream.str("");
			
			DEBUG('w',"\nAcquiring the lock for the client with NID = %d and MID = %d", networkID, inMailHdr.from);
			
			LockAcquire(lockName, buffer, networkID, inMailHdr.from);
		}
		
		else if(!strcmp(requestType,"LOCK_REL"))
		{
			char lockName[20];
			
			int networkID = -1;
			int RspCode;
			msgStream >> lockName >> RspCode;
			
			bool sendResponse = true;
			
			if(RspCode == 0)
			{
				sendResponse = false;
			}
			
			//if(networkID == -1)
			{
				networkID = inPktHdr.from;
			}
						
			msgStream.clear();
			msgStream.str("");
			
			LockRelease(lockName, buffer, networkID, inMailHdr.from, sendResponse);
		}
		
		else if(!strcmp(requestType,"LOCK_DES"))
		{
			char lockName[20];
			
			int networkID = -1;
			
			msgStream >> lockName>> networkID;
			
			if(networkID == -1)
			{
				networkID = inPktHdr.from;
			}
						
			msgStream.clear();
			msgStream.str("");
			
			LockDestroy(lockName, buffer, networkID, inMailHdr.from);
		}
		
		else if(!strcmp(requestType,"CV_CREATE"))
		{
			char *cvName = new char[20];
			
			msgStream >> cvName;
			
			// check if the condition variables with that name already exists
			int cvID = FindCondition(cvName);
			
			if((serverCount == 1) && (cvID == -1))
			{
				outPktHdr.to = inPktHdr.from;
				outPktHdr.from = netname;
				outMailHdr.to = inMailHdr.from;
				outMailHdr.from = 0;
				
				CreateCondition(cvName, outPktHdr, outMailHdr,-1);
			}
			else
			{
				if(cvID == -1) // the cv hasn't been created		
				{
					// check with other servers if they have it
				
					int reqID = MatchRequest(buffer);
				
					if(reqID == -1)
					{
						reqLock -> Acquire();
						reqID = ReqBitMap.Find();
		
						Request *req = new Request(inPktHdr.from, inMailHdr.from);
						strcpy(req -> clientRequest, buffer);
		
						
						incomingRequest[reqID] = req;
						reqLock -> Release();
					}
					else
					{
						reqLock -> Acquire();
						incomingRequest[reqID] -> queuedList -> Append((void *)new Response(inPktHdr.from, inMailHdr.from, false, ""));
						reqLock -> Release();
					}
				}
				else
				{
					//msgStream << "CV_CREATE 1 " << cvID;
			
					//delete requestType;
			
					//response = const_cast<char *>(msgStream.str().c_str());
			
					//printf("\nThe response is %s\n", response);
					char res[MaxMailSize];
					
					sprintf(res,"CV_CREATE 1 %d", cvID);
					outPktHdr.to = inPktHdr.from;
					outMailHdr.to = inMailHdr.from;
					outMailHdr.length = strlen(res) + 1;
			
					DEBUG('w',"The message was received from machine with network address %d and mailboxid %d", outPktHdr.to, outMailHdr.to);
			
					postOffice -> Send(outPktHdr, outMailHdr, res);
				}
			}
			delete[] cvName;
		}
		else if(!strcmp(requestType,"CV_WT"))
		{
			//extract the CV and Lock number
			
			char lockName[10],cvName[10];
			
			int networkID = -1;
			
			msgStream >> cvName >> lockName;
			
			networkID = inPktHdr.from;
			
			msgStream.clear();
			msgStream.str("");
			
			ConditionWait(cvName, lockName, buffer, networkID, inMailHdr.from);
			
		}
		else if(!strcmp(requestType, "CV_SIG"))
		{
			char lockName[10],cvName[10];
			
			int networkID = -1;
			msgStream >> cvName >> lockName;
			
			//if(networkID == -1)
			{
				networkID = inPktHdr.from;
			}
			
			msgStream.clear();
			msgStream.str("");
			
			ConditionSignal(cvName, lockName, buffer, networkID, inMailHdr.from);
		}
		else if(!strcmp(requestType, "CV_BCST"))
		{
			char lockName[10],cvName[10];
			
			int networkID = -1;
			msgStream >> cvName >> lockName;
			
			//if(networkID == -1)
			{
				networkID = inPktHdr.from;
			}
			
			msgStream.clear();
			msgStream.str("");
			
			ConditionBroadcast(cvName, lockName, buffer, networkID, inMailHdr.from);
		}
		else if(!strcmp(requestType, "CV_DEL"))
		{
			char cvName[10];
			int networkID = -1;
			msgStream >> cvName >> networkID;
			
			//if(networkID == -1)
			{
				networkID = inPktHdr.from;
			}
			
			msgStream.clear();
			msgStream.str("");
			
			ConditionDestroy(cvName, buffer, networkID , inMailHdr.from);
			
		}
		else if(!strcmp(requestType, "MV_CREATE"))
		{
			char *mvName = new char[20];
			
			msgStream >> mvName;
			
			// check if the monitor variable with that name already exists
			int mvID = FindMV(mvName);
			
			if((serverCount == 1) && (mvID == -1))
			{
				outPktHdr.to = inPktHdr.from;
				outPktHdr.from = netname;
				outMailHdr.to = inMailHdr.from;
				outMailHdr.from = 0;
				
				CreateMonitor(mvName, outPktHdr, outMailHdr,-1);
			}
			else
			{
				if(mvID == -1) // the cv hasn't been created		
				{
					// find if a ServerCondition is available
					int reqID = MatchRequest(buffer);
					if(reqID == -1)
					{
						reqLock -> Acquire();
						reqID = ReqBitMap.Find();
		
						Request *req = new Request(inPktHdr.from, inMailHdr.from);
						strcpy(req -> clientRequest, buffer);
		
						
						incomingRequest[reqID] = req;
						reqLock -> Release();
					}
					else
					{
						reqLock -> Acquire();
						incomingRequest[reqID] -> queuedList -> Append((void *)new Response(inPktHdr.from, inMailHdr.from, false, ""));
						reqLock -> Release();
					}
				}
				else
				{
					//msgStream << requestType << " 1 " << mvID << endl;
					//response = const_cast<char *>(msgStream.str().c_str());
				
					// send te response to the client
					
					char res[MaxMailSize];
					
					sprintf(res,"MV_CREATE 1 %d", mvID);
					
					outPktHdr.from = netname;
					outPktHdr.to = inPktHdr.from;
					outMailHdr.to = inMailHdr.from;
					outMailHdr.length = strlen(res) + 1;
				
					postOffice -> Send(outPktHdr, outMailHdr, res);
				}
			}
			delete[] mvName;
		}		
		else if(!strcmp(requestType, "MV_GET"))
		{
			char mvName[10];
			
			int networkID = -1;
			msgStream >> mvName;
			
			//if(networkID == -1)
			{
				networkID = inPktHdr.from;
			}
			
			msgStream.clear();
			msgStream.str("");
			
			MonitorGet(mvName, buffer, networkID, inMailHdr.from);
		}
		
		else if(!strcmp(requestType, "MV_SET"))
		{
			char mvName[10];
			int value;
			
			int networkID = -1;
			msgStream >> mvName >> value;
			
			//if(networkID == -1)
			{
				networkID = inPktHdr.from;
			}
			
			msgStream.clear();
			msgStream.str("");
			
			MonitorSet(mvName, value, buffer, networkID , inMailHdr.from);
		}
		if(!strcmp(requestType, "MV_DES"))
		{
			char mvName[10];
			
			int networkID = -1;
			msgStream >> mvName;
			
			//if(networkID == -1)
			{
				networkID = inPktHdr.from;
			}
			
			msgStream.clear();
			msgStream.str("");
			
			MonitorDestroy(mvName, buffer, networkID, inMailHdr.from);
		//parse the content of the message
		// send a reply if required
		}
		
		delete[] requestType;
		//delete[] buffer;
		currentThread->Yield();
	}
}

void InitializeServers()
{
	netname = postOffice -> getNetworkAddress();
	
	Thread *th = new Thread("MainServerThread");
	Thread *th1 = new Thread("ServerSendThread");
	Thread *th2 = new Thread("ServerReceiveThread");
	
	th -> Fork((VoidFunctionPtr) RunServer, 0);
	th2 -> Fork((VoidFunctionPtr) ServerReceive, 0);
	th1 -> Fork((VoidFunctionPtr) ServerSend, 0);
}

