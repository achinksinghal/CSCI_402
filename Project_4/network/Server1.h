#include "system.h"
#include "synch.h"
#include "network.h"
#include "post.h"
#include <sstream>
#include <string.h>
#include "bitmap.h"
#include "Request.h"
#include <sys/time.h>

Request *incomingRequest= new Request[500];
BitMap ReqBitMap(500);

Lock *reqLock = new Lock("RequestLock");

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


void RunServer()
{
	PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char buffer[MaxMailSize];
	stringstream msgStream (stringstream::in|stringstream::out);
	char *requestType;
	BitMap lockBitMap(250);
	BitMap CVBitMap(250);
	BitMap MVBitMap(250);
	char *response;
	
	// server runs for ever
	while(true)
	{
		// wait for a request to come
		postOffice -> Receive(0,&inPktHdr, &inMailHdr, buffer);
		printf("Received a message with request %s\n", buffer);
		// find the request type
		
		msgStream.clear();
		msgStream.str("");
		msgStream << buffer;
		requestType = new char[50];
		msgStream >> requestType;
		
		if(!strcmp(requestType,"LOCK_CREATE"))
		{
			
			char *lockName = new char[20];
			
			msgStream >> lockName;
			bool lockExists = true;
			
			printf("The lock name is %s\n", lockName);
			
			int lockID = FindLocks(lockName);
			
			printf ("The lock id is %d\n", lockID);
			
			if(lockID == -1)
			{
				// find if a lock is available
				lockID = lockBitMap.Find();
				lockExists = false;
			}
			
			response = new char[40];
			msgStream.clear();
			msgStream.str("");
			
			if(lockID != -1) // the lock table hasn't exhausted
			{
				// create a new lock only if it doesn't exist
				if(!lockExists)
				{
					ServerLock *lock = new ServerLock(lockName);
					ServerLocks[lockID] = lock;
				}
				
				printf("The lock is %d\n", lockID);
				
				msgStream << requestType << " 1 " << lockID;
			
				delete requestType;
			
				response = const_cast<char *>(msgStream.str().c_str());
			
				printf("\nThe response is %s\n", response);
				outPktHdr.to = inPktHdr.from;
				outMailHdr.to = inMailHdr.from;
				outMailHdr.length = strlen(response) + 1;
			
				printf("The message was received from machine with network address %d and mailboxid %d\n", outPktHdr.to, outMailHdr.to);
			
				postOffice -> Send(outPktHdr, outMailHdr, response);
			}
			else // the lock table has exhausted
			{
				msgStream << requestType << " 0 Exhausted" << endl;
				
				response = const_cast<char *>(msgStream.str().c_str());
			
				outPktHdr.to = inPktHdr.from;
				outMailHdr.to = inMailHdr.from;
				outMailHdr.length = strlen(response) + 1;
			
				postOffice -> Send(outPktHdr, outMailHdr, response);
			}
		}
		
		if(!strcmp(requestType,"LOCK_ACQ"))
		{
			int lockID;
			msgStream >> lockID;
						
			response = new char[40];
			msgStream.clear();
			msgStream.str("");
			
			if((lockID > LOCK_SIZE - 1) || (lockID < 0)) // lock is an invalid lock
			{
				msgStream << requestType << " 0 " << "BadLock" << endl; // create the response for the client
				response = const_cast<char *>(msgStream.str().c_str());
				
				outPktHdr.to = inPktHdr.from;
				outMailHdr.to = inMailHdr.from;
				outMailHdr.length = strlen(response) + 1;
				
				// send the resposne to the client telling its request cannot be met as the lock is invalid
				postOffice -> Send(outPktHdr, outMailHdr, response);
			}
			else // the lock is in a valid range
			{
				bool sendResponse = false;
				
				//printf("\nThe lock status of lock %d is %d\n", lockID, ServerLocks[lockID] -> GetLockState());
				
				if((ServerLocks[lockID] != NULL) && (ServerLocks[lockID] -> deleteCalled == false)) // the lock was created and the delete was no called 
				{
					if(ServerLocks[lockID] -> GetLockState() == FREE)
					{
						sendResponse = true;
					}
					
					ServerLocks[lockID] -> Acquire(inPktHdr.from, inMailHdr.from);
				}
				else
				{
					// The lock was either deleted or uninitialized
					msgStream << requestType << " 0 " << "BadLock" << endl; // create the response for the client
					response = const_cast<char *>(msgStream.str().c_str());
					
					outPktHdr.to = inPktHdr.from;
					outMailHdr.to = inMailHdr.from;
					outMailHdr.length = strlen(response) + 1;
				
					// send the resposne to the client telling its request cannot be met as the lock is invalid
					postOffice -> Send(outPktHdr, outMailHdr, response);
				}
				
				
				if(sendResponse) // this is the first client interested in this lock and hence should receive a reply
				{
					msgStream << requestType << " 1 " << "OK" << endl;
					response = const_cast<char *>(msgStream.str().c_str());
					outPktHdr.to = inPktHdr.from;
					outMailHdr.to = inMailHdr.from;
					outMailHdr.length = strlen(response) + 1;
				
					// send the response to the client telling it has acquired the lock
					postOffice -> Send(outPktHdr, outMailHdr, response);
				}
			}
		}
		
		if(!strcmp(requestType,"LOCK_REL"))
		{
			int lockID;
			msgStream >> lockID;
			
			response = new char[40];
			msgStream.clear();
			msgStream.str("");
			
			if((lockID > LOCK_SIZE - 1) || (lockID < 0)) // lock is an invalid lock
			{
				msgStream << requestType << " 0 " << "BadLock" << endl; // create the response for the client
				response = const_cast<char *>(msgStream.str().c_str());
				
				outPktHdr.to = inPktHdr.from;
				outMailHdr.to = inMailHdr.from;
				outMailHdr.length = strlen(response) + 1;
				
				// send the resposne to the client telling its request cannot be met as the lock is invalid
				postOffice -> Send(outPktHdr, outMailHdr, response);
			}
			else // the lock is in the valid range
			{
				if(ServerLocks[lockID] == NULL) // validate that the client is no passing invalid data;
				{
					msgStream << requestType << " 0 " << "BadLock" << endl; // create the response for the client
					response = const_cast<char *>(msgStream.str().c_str());
				
					outPktHdr.to = inPktHdr.from;
					outMailHdr.to = inMailHdr.from;
					outMailHdr.length = strlen(response) + 1;
				
					// send the resposne to the client telling its request cannot be met as the lock is invalid
					postOffice -> Send(outPktHdr, outMailHdr, response);
				}
				else
				{// the lock is valid we can release the lock
					Response *resp = ServerLocks[lockID] -> Release(inPktHdr.from, inMailHdr.from);
					
					//printf("\nThe response generated at release is\n Network ID : %d MailboxID : %d\n", resp -> networkID, resp -> mailboxID);
					msgStream << requestType << " 1 " << "OK" << endl;
					response = const_cast<char *>(msgStream.str().c_str());
					outPktHdr.to = inPktHdr.from;
					outMailHdr.to = inMailHdr.from;
					outMailHdr.length = strlen(response) + 1;
				
					// send the response to the client telling it to proceed
					postOffice -> Send(outPktHdr, outMailHdr, response);
					
					//printf("\nThe response generated at release is\n Network ID : %d MailboxID : %d\n", resp -> networkID, resp -> mailboxID);
					
					if((resp -> networkID != -1) && (resp -> mailboxID != -1))
					{
						printf("Releasing the response for blocked client");
						PacketHeader pkHdr;
						MailHeader mailHdr;
						
						pkHdr.to = resp -> networkID; // wake up a client blocked on the lock
						mailHdr.to = resp -> mailboxID;
						
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
			}
		}
		
		if(!strcmp(requestType,"LOCK_DES"))
		{
			int lockID;
			msgStream >> lockID;
			
			response = new char[40];
			msgStream.clear();
			msgStream.str("");
			
			printf("\n Deleting the lock %d\n", lockID);
			
			if((lockID < 0) || (lockID > (LOCK_SIZE - 1)))// check if the lock is in the range
			{
				msgStream << requestType << " 0 " << "BadLock" << endl; // create the response for the client
				response = const_cast<char *>(msgStream.str().c_str());
				
				outPktHdr.to = inPktHdr.from;
				outMailHdr.to = inMailHdr.from;
				outMailHdr.length = strlen(response) + 1;
				
				// send the resposne to the client telling its request cannot be met as the lock is invalid
				postOffice -> Send(outPktHdr, outMailHdr, response);
			}
			else
			{
				if(ServerLocks[lockID] == NULL)
				{
					msgStream << requestType << " 0 " << "BadLock" << endl; // create the response for the client
					response = const_cast<char *>(msgStream.str().c_str());
				
					outPktHdr.to = inPktHdr.from;
					outMailHdr.to = inMailHdr.from;
					outMailHdr.length = strlen(response) + 1;
				
					// send the resposne to the client telling its request cannot be met as the lock is invalid
					postOffice -> Send(outPktHdr, outMailHdr, response);
				}
				else
				{
					if(ServerLocks[lockID] -> refCount == 0)
					{
						delete ServerLocks[lockID];
						// clear the entry in the lock table
						lockBitMap.Clear(lockID);
						ServerLocks[lockID] = NULL;
					}
					else
					{
						ServerLocks[lockID] -> deleteCalled = true;
					}
					// send an ok status to the client
					msgStream << requestType << " 1 OK"  << endl;
					response = const_cast<char *>(msgStream.str().c_str());
					
					outPktHdr.to = inPktHdr.from;
					outMailHdr.to = inMailHdr.from;
					outMailHdr.length = strlen(response) + 1;
					
					// send the response to client to unblock it
					postOffice -> Send(outPktHdr, outMailHdr, response);
				}
			}
		}
		
		if(!strcmp(requestType,"CV_CREATE"))
		{
			response = new char[40];
			char *cvName = new char[20];
			
			msgStream >> cvName;
			
			// check if the condition variables with that name already exists
			int cvID = FindCondition(cvName);
			
			bool cvExists = true;
			if(cvID == -1) // the cv hasn't been created		
			{
				// find if a ServerCondition is available
				cvID = CVBitMap.Find();
				cvExists = false;
			}
			
			//flush the string stream
			msgStream.clear();
			msgStream.str("");
			
			if(cvID != -1) // the condition table hasn't exhausted
			{
				if(!cvExists)
				{
					ServerCondition *cv = new ServerCondition(cvName);
					ServerConditions[cvID] = cv;
				}
				
				printf("The Server Condition is %d\n", cvID);
				
				msgStream << requestType << " 1 " << cvID;
			
				delete requestType;
			
				response = const_cast<char *>(msgStream.str().c_str());
			
				printf("\nThe response is %s\n", response);
				outPktHdr.to = inPktHdr.from;
				outMailHdr.to = inMailHdr.from;
				outMailHdr.length = strlen(response) + 1;
			
				printf("The message was received from machine with network address %d and mailboxid %d\n", outPktHdr.to, outMailHdr.to);
			
				postOffice -> Send(outPktHdr, outMailHdr, response);
			}
			else // the condition table has exhausted
			{
				msgStream << requestType << " 0 Exhausted" << endl;
				
				response = const_cast<char *>(msgStream.str().c_str());
				
				outPktHdr.to = inPktHdr.from;
				outMailHdr.to = inMailHdr.from;
				outMailHdr.length = strlen(response) + 1;
			
				postOffice -> Send(outPktHdr, outMailHdr, response);
			}
			
		}
		if(!strcmp(requestType,"CV_WT"))
		{
			//extract the CV and Lock number
			
			int lockID, cvID;
			
			msgStream >> cvID >> lockID;
			
			msgStream.clear();
			msgStream.str("");
			
			if((cvID < 0) || (cvID > CV_SIZE))
			{
				msgStream << requestType << " 0 BadCV" << endl;
				response = const_cast<char *>(msgStream.str().c_str());
				
				// create an error response and send it to the client
				outPktHdr.to = inPktHdr.from;
				outMailHdr.to = inMailHdr.from;
				outMailHdr.length = strlen(response) + 1;
				
				postOffice -> Send(outPktHdr, outMailHdr, response);
			}
			else if((lockID < 0) || (lockID > LOCK_SIZE -1))
			{
				msgStream << requestType << " 0 BadLock" << endl;
				response = const_cast<char *>(msgStream.str().c_str());
				
				//create an error response and send it to client
				outPktHdr.to = inPktHdr.from;
				outMailHdr.to = inMailHdr.from;
				outMailHdr.length = strlen(response) + 1;
				postOffice -> Send(outPktHdr, outMailHdr, response);
			}
			else
			{ // this is a valid combination of lockID and CV ID
				// check if someone is trying to break the code by passing valid ids which haven't been created yet
				
				if(ServerLocks[lockID] == NULL || ServerConditions[cvID] == NULL)
				{
					// create a generic message
					msgStream << requestType << " 0 BadInput" << endl;
					response = const_cast<char *>(msgStream.str().c_str());
					
					//create an error response and send it to client
					outPktHdr.to = inPktHdr.from;
					outMailHdr.to = inMailHdr.from;
					outMailHdr.length = strlen(response) + 1;
					postOffice -> Send(outPktHdr, outMailHdr, response);
				}
				else
				{ // we have a valid CV and a valid Lock
					if(ServerConditions[cvID] -> deleteCalled == true) 
					{
						msgStream << requestType << " 0 CVDeleted" << endl;
						
						response = const_cast<char *>(msgStream.str().c_str());
						
						//create an error response and send it to client
						outPktHdr.to = inPktHdr.from;
						outMailHdr.to = inMailHdr.from;
						outMailHdr.length = strlen(response) + 1;
						postOffice -> Send(outPktHdr, outMailHdr, response);
					}
					else
					{	// check if delete was called on the CV
						Response *resp = ServerConditions[cvID] -> Wait(lockID, inPktHdr.from, inMailHdr.from);
					
						printf("\nThe response is networkID = %d and mailboxID = %d\n", resp -> networkID, resp -> mailboxID);
						printf("\n the wait was called by network id %d\n", inPktHdr.from);
					
						// there was an error in the Wait call
						if(resp -> isError)
						{
							msgStream << requestType << " 0 " << resp -> response << endl;
							response = const_cast<char *>(msgStream.str().c_str());
						
							//create an error response and send it to client
							outPktHdr.to = inPktHdr.from;
							outMailHdr.to = inMailHdr.from;
							outMailHdr.length = strlen(response) + 1;
							postOffice -> Send(outPktHdr, outMailHdr, response);
						}
						else
						{	
							if((resp -> networkID != -1) && (resp -> networkID != -1))
							{
								msgStream << requestType << " 1 OK" << endl;
								response = const_cast<char *>(msgStream.str().c_str());
							
								// send the response to the blocked client that was waiting to acquire the lock released in the wait function
								outPktHdr.to = resp -> networkID;
								outMailHdr.to = resp -> mailboxID;
								outMailHdr.length = strlen(response) + 1;
								postOffice -> Send(outPktHdr, outMailHdr, response);
							}
						}
					}
					// the wait was successfull. we don't send anything to the client
				}
			}
			
		}
		if(!strcmp(requestType, "CV_SIG"))
		{
			int lockID, cvID;
			
			msgStream >> cvID >> lockID;
			
			msgStream.clear();
			msgStream.str("");
			
			if((cvID < 0) || (cvID > CV_SIZE))
			{
				msgStream << requestType << " 0 BadCV" << endl;
				response = const_cast<char *>(msgStream.str().c_str());
				
				// create an error response and send it to the client
				outPktHdr.to = inPktHdr.from;
				outMailHdr.to = inMailHdr.from;
				outMailHdr.length = strlen(response) + 1;
				
				postOffice -> Send(outPktHdr, outMailHdr, response);
			}
			else if((lockID < 0) || (lockID > LOCK_SIZE -1))
			{
				msgStream << requestType << " 0 BadLock" << endl;
				response = const_cast<char *>(msgStream.str().c_str());
				
				//create an error response and send it to client
				outPktHdr.to = inPktHdr.from;
				outMailHdr.to = inMailHdr.from;
				outMailHdr.length = strlen(response) + 1;
				postOffice -> Send(outPktHdr, outMailHdr, response);
			}
			else
			{
				// this is a valid combination of lockID and CV ID
				// check if someone is trying to break the code by passing valid ids which haven't been created yet
				
				if(ServerLocks[lockID] == NULL || ServerConditions[cvID] == NULL)
				{
					// create a generic message
					msgStream << requestType << " 0 BadInput" << endl;
					response = const_cast<char *>(msgStream.str().c_str());
					
					//create an error response and send it to client
					outPktHdr.to = inPktHdr.from;
					outMailHdr.to = inMailHdr.from;
					outMailHdr.length = strlen(response) + 1;
					postOffice -> Send(outPktHdr, outMailHdr, response);
					
				}
				else
				{	// the lock and condition variable is valid
					// signal the client threads waiting on the CV
					
					bool result = ServerConditions[cvID] -> Signal(lockID);
					
					if(!result) // there was some error while signalling
					{
						// create a generic error message and send it to client
						msgStream << requestType << "0 BadInput" << endl;
					}
					else
					{
						// create an OK status message to send to the client
						msgStream << requestType << "1 OK" << endl;
					}
					
					// send the response to the client
					
					outPktHdr.to = inPktHdr.from;
					outMailHdr.to = inMailHdr.from;
					outMailHdr.length = strlen(response) + 1;
					postOffice -> Send(outPktHdr, outMailHdr, response);
					
					// perform clean up. Check if some one has called delete on the conditiob variable
					if((ServerConditions[cvID] -> refCount == 0) && (ServerConditions[cvID] -> deleteCalled))
					{
						delete ServerConditions[cvID];
						CVBitMap.Clear(cvID);
						ServerConditions[cvID] = NULL;
					}
				}
			}
		}
		
		if(!strcmp(requestType, "CV_BCST"))
		{
			int lockID, cvID;
			
			msgStream >> cvID >> lockID;
			
			msgStream.clear();
			msgStream.str("");
			
			if((cvID < 0) || (cvID > CV_SIZE))
			{
				msgStream << requestType << " 0 BadCV" << endl;
				response = const_cast<char *>(msgStream.str().c_str());
				
				// create an error response and send it to the client
				outPktHdr.to = inPktHdr.from;
				outMailHdr.to = inMailHdr.from;
				outMailHdr.length = strlen(response) + 1;
				
				postOffice -> Send(outPktHdr, outMailHdr, response);
			}
			else if((lockID < 0) || (lockID > LOCK_SIZE -1))
			{
				msgStream << requestType << " 0 BadLock" << endl;
				response = const_cast<char *>(msgStream.str().c_str());
				
				//create an error response and send it to client
				outPktHdr.to = inPktHdr.from;
				outMailHdr.to = inMailHdr.from;
				outMailHdr.length = strlen(response) + 1;
				postOffice -> Send(outPktHdr, outMailHdr, response);
			}
			else
			{
				// this is a valid combination of lockID and CV ID
				// check if someone is trying to break the code by passing valid ids which haven't been created yet
				
				if(ServerLocks[lockID] == NULL || ServerConditions[cvID] == NULL)
				{
					// create a generic message
					msgStream << requestType << " 0 BadInput" << endl;
					response = const_cast<char *>(msgStream.str().c_str());
					
					//create an error response and send it to client
					outPktHdr.to = inPktHdr.from;
					outMailHdr.to = inMailHdr.from;
					outMailHdr.length = strlen(response) + 1;
					postOffice -> Send(outPktHdr, outMailHdr, response);
					
				}
				else
				{	// the lock and condition variable is valid
					// signal the client threads waiting on the CV
					
					bool result = ServerConditions[cvID] -> Broadcast(lockID);
					
					if(!result) // there was some error while signalling
					{
						// create a generic error message and send it to client
						msgStream << requestType << "0 BadInput" << endl;
					}
					else
					{
						// create an OK status message to send to the client
						msgStream << requestType << "1 OK" << endl;
					}
					
					// send the response to the client
					
					outPktHdr.to = inPktHdr.from;
					outMailHdr.to = inMailHdr.from;
					outMailHdr.length = strlen(response) + 1;
					postOffice -> Send(outPktHdr, outMailHdr, response);
					
					// perform clean up. Check if some one has called delete on the conditiob variable
					if((ServerConditions[cvID] -> refCount == 0) && (ServerConditions[cvID] -> deleteCalled))
					{
						delete ServerConditions[cvID];
						CVBitMap.Clear(cvID);
						ServerConditions[cvID] = NULL;
					}
				}
			}
		}
		if(!strcmp(requestType, "CV_DEL"))
		{
			int cvID;
			
			msgStream >> cvID;
			
			msgStream.clear();
			msgStream.str("");
			
			printf("\nDeleting condition variable %d\n", cvID);
			
			if((cvID < 0) || (cvID > CV_SIZE - 1) || (ServerConditions[cvID] == NULL))// if the cv is out of range
			{
				msgStream << requestType << " 0 BadCV" << endl;
				response = const_cast<char *>(msgStream.str().c_str());
				
				// create an error response and send it to the client
				outPktHdr.to = inPktHdr.from;
				outMailHdr.to = inMailHdr.from;
				outMailHdr.length = strlen(response) + 1;
				
				postOffice -> Send(outPktHdr, outMailHdr, response);
			}
			else
			{
				// the cv is a valid CV
				
				// check if the reference count is 0
				if(ServerConditions[cvID] -> refCount == 0)
				{
					delete ServerConditions[cvID];
					CVBitMap.Clear(cvID);
					ServerConditions[cvID] = NULL;
				}
				else
				{
					ServerConditions[cvID] -> deleteCalled = true;
				}

				msgStream << requestType << " 1 OK" << endl;
				response = const_cast<char *>(msgStream.str().c_str());
				
				// create an error response and send it to the client
				outPktHdr.to = inPktHdr.from;
				outMailHdr.to = inMailHdr.from;
				outMailHdr.length = strlen(response) + 1;
				
				postOffice -> Send(outPktHdr, outMailHdr, response);
				
			}
			
		}
		if(!strcmp(requestType, "MV_CREATE"))
		{
			response = new char[40];
			
			char *mvName = new char[20];
			
			msgStream >> mvName;
			
			// check if the monitor variable with that name already exists
			int mvID = FindMV(mvName);
			
			bool mvExists = true;
			if(mvID == -1) // the cv hasn't been created		
			{
				// find if a ServerCondition is available
				mvID = MVBitMap.Find();
				mvExists = false;
			}
			
			//flush the string stream
			msgStream.clear();
			msgStream.str("");
			
			if(mvID != -1)//we have found a valid number
			{
				if(!mvExists)
				{
					MonitorVariables[mvID] = new MonitorVariable(mvName);
				}
				
				//create response to be sent to the client
				msgStream << requestType << " 1 " << mvID << endl;
				response = const_cast<char *>(msgStream.str().c_str());
				
				// send te response to the client
				outPktHdr.to = inPktHdr.from;
				outMailHdr.to = inMailHdr.from;
				outMailHdr.length = strlen(response) + 1;
				
				postOffice -> Send(outPktHdr, outMailHdr, response);
				
			}
			else
			{
				// create the error message
				msgStream << requestType << " 0 -1";
				response = const_cast<char *>(msgStream.str().c_str());
				
				// send te response to the client
				outPktHdr.to = inPktHdr.from;
				outMailHdr.to = inMailHdr.from;
				outMailHdr.length = strlen(response) + 1;
				
				postOffice -> Send(outPktHdr, outMailHdr, response); 
			}
			
		}
		
		if(!strcmp(requestType, "MV_GET"))
		{
			int mvID;
			
			msgStream >> mvID;
			
			msgStream.clear();
			msgStream.str("");
			
			if((mvID < 0) || (mvID >= MV_SIZE) || (MonitorVariables[mvID] == NULL)) // the MV doesn't exist
			{
				msgStream << requestType << " 0 BadMV" << endl;
				response = const_cast<char *>(msgStream.str().c_str());
				
				// send te response to the client
				outPktHdr.to = inPktHdr.from;
				outMailHdr.to = inMailHdr.from;
				outMailHdr.length = strlen(response) + 1;
				
				postOffice -> Send(outPktHdr, outMailHdr, response);
			}
			else // we have a valid MV
			{
				msgStream << requestType << " 1 " << MonitorVariables[mvID] -> getValue() << endl;
				
				response = const_cast<char *>(msgStream.str().c_str());
				
				// send te response to the client
				outPktHdr.to = inPktHdr.from;
				outMailHdr.to = inMailHdr.from;
				outMailHdr.length = strlen(response) + 1;
				
				postOffice -> Send(outPktHdr, outMailHdr, response);
			}
			
			// find if clean up needs to be done
			// if((MonitorVariables[mvID] -> refCount == 0) && ( MonitorVariables[mvID] -> deleteCalled))
			// {
				// delete MonitorVariables[mvID];
				// MVBitMap.Clear(mvID);
				// MonitorVariables[mvID] = NULL;
			// }
		}
		
		if(!strcmp(requestType, "MV_SET"))
		{
			int mvID, val;
			
			msgStream >> mvID >> val;
			
			msgStream.clear();
			msgStream.str("");
			
			if((mvID < 0) || (mvID >= MV_SIZE) || (MonitorVariables[mvID] == NULL)) // the MV doesn't exist
			{
				msgStream << requestType << " 0 BadMV" << endl;
				response = const_cast<char *>(msgStream.str().c_str());
				
				// send te response to the client
				outPktHdr.to = inPktHdr.from;
				outMailHdr.to = inMailHdr.from;
				outMailHdr.length = strlen(response) + 1;
				
				postOffice -> Send(outPktHdr, outMailHdr, response);
			}
			else // we have a valid MV
			{
				msgStream << requestType << " 1 " << MonitorVariables[mvID] -> setValue(val) << endl;
				
				response = const_cast<char *>(msgStream.str().c_str());
				
				// send te response to the client
				outPktHdr.to = inPktHdr.from;
				outMailHdr.to = inMailHdr.from;
				outMailHdr.length = strlen(response) + 1;
				
				postOffice -> Send(outPktHdr, outMailHdr, response);
				
				//find if clean needs to be done
				
				// if((MonitorVariables[mvID] -> refCount == 0) && ( MonitorVariables[mvID] -> deleteCalled))
				// {
					// delete MonitorVariables[mvID];
					// MVBitMap.Clear(mvID);
					// MonitorVariables[mvID] = NULL;
				// }
			}
		}
		if(!strcmp(requestType, "MV_DES"))
		{
			int mvID;
			
			msgStream >> mvID;
			
			msgStream.clear();
			msgStream.str("");
			
			if((mvID < 0) || (mvID >= MV_SIZE) || (MonitorVariables[mvID] == NULL)) // the MV doesn't exist
			{
				msgStream << requestType << " 0 BadMV" << endl;
				response = const_cast<char *>(msgStream.str().c_str());
				
				// send te response to the client
				outPktHdr.to = inPktHdr.from;
				outMailHdr.to = inMailHdr.from;
				outMailHdr.length = strlen(response) + 1;
				
				postOffice -> Send(outPktHdr, outMailHdr, response);
			}
			else // we have a valid MV
			{
				
				delete MonitorVariables[mvID];
				MVBitMap.Clear(mvID);
				MonitorVariables[mvID] = NULL;
				
				msgStream << requestType << " 1 OK" << endl;
				
				response = const_cast<char *>(msgStream.str().c_str());
				
				// send te response to the client
				outPktHdr.to = inPktHdr.from;
				outMailHdr.to = inMailHdr.from;
				outMailHdr.length = strlen(response) + 1;
				
				postOffice -> Send(outPktHdr, outMailHdr, response);
			}
		}
		//parse the content of the message
		// send a reply if required
	}
}

int FindPendingRequest()
{
	reqLock -> Acquire();
	
	for(int idx = 0; (incomingRequest[idx] != NULL) && (idx < 500); idx++)
	{
		if(incomingRequest[idx].isPending)
		{
			incomingRequest[idx].isPending = false;
			reqLock -> Release();
			return idx;
		}
	}
	
	reqLock -> Release();
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
	outMailHdt.from = 2;
	outMailHdr.length = strlen(msg) + 1;
	
	for(int servIdx = 0; servIdx < serverCount; servIdx++)
	{
		// send it to all the servers except yourself
		if(servIdx != netname)
		{
			outPktHdr.to = servIdx; 
			
			Dispatch(outPktHdr, outMailHdr, msg);
		}
	}
}

void ServerSend()
{
	PacketHeader outPktHdr;
    MailHeader outMailHdr;
	struct timeval timestamp;
    char buffer[MaxMailSize];
	char *requestType = new char[30];
	char *msg = new char[40];
	long mseconds;
	
	stringstream msgStream (stringstream::in|stringstream::out);
	
	outMailHdr.to = 2; // The server to server communication is done on mailbox id 2
	outMailHdr.from = 2;
	
	
	while(true)
	{
		//read the request from the request queue and select the first pending request
		
		int currentReq = FindPendingRequest();
		
		msgStream << incomingRequest[currentReq].clientRequest;
		msgStream >> requestType;
		
		//find if the request is a create request
		if(!strcmp(requestType,"CREATE_LOCK") || !strcmp(requestType, "CREATE_CV") || !strcmp(requestType, "CREATE_MV"))
		{
			// add timestamp to the message
			gettimeofday(&timestamp, NULL);
			mseconds = timestamp.tv_usec /  1000.0;
			mseconds = mseconds / (3600 * 1000) // assuming the simulation will not run for more than an hour
			
			//maintain the timestamp record for the message
			incomingRequest[currentReq].timestamp = mseconds;
			
			// prepare the message to the servers
			msgStream.clear();
			msgStream.str("");
			
			// put the new message in the message stream
			
			msgStream << incomingRequest[currrentReq].clientRequest << " " << mseconds << " " << currentReq << endl;
			msgStream >> msg; // we have the desired message here
			
			BroadcastToServers(msg);
			
		}
		//else 
		
	
	
	
	// for all request place the sequence number of the request in the message to the servers.
	
	}
}

long SearchRequestTable(char *msgType, char* paramName)
{
	stringstream msgStream (stringstream::in | stringstream::out);
	
	char *reqType = new char[20];
	char *name = new char[20];
	
	reqLock -> Acquire();
	
	// check if we have a non complete request with the same request type and same parameter name
	for(int idx = 0; (incomingRequest[idx] != NULL) && (idx < 500) && (incomingRequest[idx].responseCount < serverCount - 1); idx++)
	{
		msgStream.clear();
		msgStream.str("");
		
		msgStream << incomingRequest[idx].clientRequest;
		msgStream >> reqType >> name;
		
		if((!strcmp(reqType, msgType)) && (!strcmp(name, paramName))) // we have found an exact request
		{
			// return the timestamp of the request
			reqLock -> Release();
			return incomingRequest[idx].timestamp;
		}
	}
	
	reqLock -> Release();
	// this server doesn't have any request matching the incoming request
	return -1;
}

void ServerReceive()
{
	PacketHeader inPktHdr,outPktHdr;
    MailHeader inMailHdr, outMailHdr;
	
	char buffer[MaxMailSize];
	stringstream msgStream (stringstream::in | stringstream::out);
	char *msgType = char[15];
	
	while(true)
	{
		postOffice -> Receive(2, &inPktHdr, &inMailHdr, buffer);
	
		// figure out if it is a response to a request or a request itself
		// response will begin with key word RSP
		
		msgStream << buffer;
		msgStream >> msgType;
		
		if(strcmp(msgType,"RSP") || strcmp(msgType,"RSPC")) // its not a response, its a request
		{
			// check the type of request. Broadly categorized into create and other requests
			
			// check if its a create request
			if(!strcmp(msgType,"CREATE_LOCK") || (!strcmp(msgType, "CREATE_CV")) || (!strcmp(msgType, "CREATE_MV")))
			{	
				char *paramName = new char[20];
				msgStream >> paramName;
				
				bool sendResponse = false;
				
				int pos = -1;
				
				if(!strcmp(msgType, "CREATE_LOCK"))
				{
					pos = FindLocks(paramName);
				}
				else if((!strcmp(msgType, "CREATE_CV"))
				{
					pos = FindCondition(paramName);
				}
				else if(!strcmp(msgType, "CREATE_MV"))
				{
					pos = FindMV(paramName);
				}
				
				if(pos == -1) // we don't have the synch param created
				{
					// check if we have a request for the same synch param . If we have fetch the timestamp of the request
					
					long reqTimestamp = SearchRequestTable(msgTable, lockName)
					
					if(reqTimestamp != -1) // this server has a similar non-completed request
					{
						long timestamp;
						int msgID;
						
						msgStream >> timestamp >> msgID;
						
						msgStream.clear();
						msgStream.str("");
						
						if(timestamp < reqTimestamp) // the incoming request has a lower timestamp than the stored request
						{
							msgStream << "RSPC 1 " << msgID << endl;
							//sendResponse = true;
						}
						else if(timestamp > reqTimestamp)
						{
							msgStream << "RSP 0 " << msgID << endl; 
							// we should queue the response here.
							// This is a hack but I couldn't figure out a better way to accomplish it
							
							//Response *queueResponse = new Response(inPktHdr.from, inMailHdr.from, false, "");
							//incomingRequest[msgID].rsp -> Append((void *) queueResponse);
						}
						else if(timestamp == reqTimestamp)
						{
							if(inPktHdr.from < netname) // the machine id of this server is greater than the requesting server
							{
								msgStream << "RSPC 1 " << msgID << endl;
								sendResponse = true;
							}							
							else // else this server has a priority over requesting server
							{
								msgStream << "RSP 0 " << msgID << endl;
								// queue the response here
								//Response *queueResponse = new Response(inPktHdr.from, inMailHdr.from, false, "");
								//incomingRequest[msgID].rsp -> Append((void *) queueResponse);
							}
						}
						
						char *response = new char[40];
						response = const_cast<char *>(msgStream.str().c_str());
						
						outPktHdr.to = inPktHdr.from;
						outMailHdr.to = inMailHdr.from;
						outMailHdr.from = 2;
						outMailHdr.length = strlen(response);
						
						Dispatch(outPktHdr, outMailHdr, response);
						
					}
					
				}
				
			}
			else
			{ // its a generic "do you have request"
			
				char *name = new char[20];
					
				msgStream >> name;
				int msgID;
				msgStream >> msgID;

				msgStream.clear();
				msgStream.str("");
					
				if(!strcmp(msgType, "LOCK"))
				{
					if(FindLocks(name) == -1) // the lock is not available
					{
						msgStream << "RSP 0 " << msgID << endl;
					}
					else
					{
						msgStream << "RSP 1 " << msgID << endl;
					}
				}
				else if(!strcmp(msgType, "CV"))
				{
					if(FindCondition(name) == -1) // the CV is not available
					{
						msgStream << "RSP 0 " << msgID << endl;
					}
					else
					{
						msgStream << "RSP 1 " << msgID << endl;
					}
				}
				else if(!strcmp(msgType, "MV"))
				{
					if(FindMV(name) == -1) // the MV is not available
					{
						msgStream << "RSP 0 " << msgID << endl;
					}
					else
					{
						msgStream << "RSP 1 " << msgID << endl;
					}
				}
				
				char *response = new char[40];
				response = const_cast<char *>(msgStream.str().c_str());
						
				outPktHdr.to = inPktHdr.from;
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
		  
		  msgStream >> status >> msgID;
		  
		  HandleCreateRequest(status, msgID, inPktHdr.from, inMailHdr.from);
		}
		else if(!strcmp(msgType,"RSP"))
		{
			int status, msgID
			
			msgStream >> status >> msgID;
			
			if(status == 1)
			{
				incomingRequest[msgID].owner = new Response(inPktHdr.from, inMailHdr.from, false, "");
			}
			
			incomingRequest[msgID].responseCount++;
			
			if(incomingRequest[msgID].responseCount == serverCount - 1)
			{
				outPktHdr.from = incomingRequest[msgID].response -> networkID;
				outPktHdr.to = incomingRequest[msgID].owner -> networkID;
				
				outMailHdr.from = incomingRequest[msgID].response -> mailboxID;
				outMailHdr.to = 0;
				outMailHdr.length = strlen(incomingRequest[msgID].clientRequest) + 1;
				
				postOffice -> Send(outPktHdr, outMailHdr, incomingRequest[msgID.clientRequest]);
				
				delete incomingRequest[msgID];
				incomingRequest[msgID] == NULL;

				ReqBitMap.Clear(msgID);
			}
		}
	
	}
}