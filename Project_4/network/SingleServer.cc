#include "system.h"
#include "synch.h"
#include "network.h"
#include "post.h"
#include <sstream>
#include <string.h>
#include "bitmap.h"

#define LOCK_SIZE 500
#define CV_SIZE 500
#define MV_SIZE 500



int FindSLocks(char *lockName)
{
	for(int idx = 0; idx < LOCK_SIZE; idx++)
	{
		// check if all the created locks have been checked
		if(SServerLocks[idx] == NULL)
		{
			return -1;
		}
		else
		{// there are locks in the lock table check if the lock with a name sent has already been created
			if(!strcmp(SServerLocks[idx] -> getName(), lockName))
			{ 
				return idx;
			}
		}
	}
	
	// the entire lock table was scanned and the lock was not found
	return -1;
}

int FindSCondition(char *cvName)
{
	for(int idx = 0; idx < CV_SIZE; idx++)
	{
		// check if all the created condition variables have been checked
		if(SServerConditions[idx] == NULL)
		{
			return -1;
		}
		else
		{
			// there are CVs in the CV table, check if the CV with the name sent has already been created
			if(!strcmp(SServerConditions[idx] -> getName(), cvName))
			{
				return idx;
			}
		}
	}
	
	// the entire table has been scanned 
	return -1;
}

int FindSMV(char *mvName)
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


void RunSingleServer()
{
	PacketHeader outPktHdr, inPktHdr;
	MailHeader outMailHdr, inMailHdr;
	char buffer[MaxMailSize];
	stringstream msgStream (stringstream::in|stringstream::out);
	char *requestType;
	BitMap lockBitMap(500);
	BitMap CVBitMap(500);
	BitMap MVBitMap(500);
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
			
			int lockID = FindSLocks(lockName);
			
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
					SServerLock *lock = new SServerLock(lockName);
					SServerLocks[lockID] = lock;
				}
				
				printf("The lock is %d\n", lockID);
				
				msgStream << requestType << " 1 " << lockID;
			
				delete requestType;
			
				response = const_cast<char *>(msgStream.str().c_str());
			
				printf("\nThe response is %s\n", response);
				outPktHdr.to = inPktHdr.from;
				outMailHdr.to = inMailHdr.from;
				outMailHdr.from = 0;
				outMailHdr.length = strlen(response) + 1;
			
				printf("The message was received from machine with network address %d and mailboxid %d\n", outPktHdr.to, outMailHdr.to);
				printf("ACHINTYA send done\n");	
				postOffice -> Send(outPktHdr, outMailHdr, response);
			}
			else // the lock table has exhausted
			{
				msgStream << requestType << " 0 Exhausted" << endl;
				
				response = const_cast<char *>(msgStream.str().c_str());
			
				outPktHdr.to = inPktHdr.from;
				outMailHdr.to = inMailHdr.from;
				outMailHdr.from = 0;
				outMailHdr.length = strlen(response) + 1;
			
				postOffice -> Send(outPktHdr, outMailHdr, response);
			}
		}
		
		if(!strcmp(requestType,"LOCK_ACQ"))
		{
			int lockID;
			char *lockName = new char[20];
			msgStream >> lockName;

			lockID = FindSLocks(lockName);
						
			response = new char[40];
			msgStream.clear();
			msgStream.str("");
			
			if((lockID > LOCK_SIZE - 1) || (lockID < 0)) // lock is an invalid lock
			{
				msgStream << requestType << " 0 " << "BadLock" << endl; // create the response for the client
				response = const_cast<char *>(msgStream.str().c_str());
				
				outPktHdr.to = inPktHdr.from;
				outMailHdr.to = inMailHdr.from;
				outMailHdr.from = 0;
				outMailHdr.length = strlen(response) + 1;
				
				// send the resposne to the client telling its request cannot be met as the lock is invalid
				postOffice -> Send(outPktHdr, outMailHdr, response);
			}
			else // the lock is in a valid range
			{
				bool sendResponse = false;
				
				//printf("\nThe lock status of lock %d is %d\n", lockID, SServerLocks[lockID] -> GetLockState());
				
				if((SServerLocks[lockID] != NULL) && (SServerLocks[lockID] -> deleteCalled == false)) // the lock was created and the delete was no called 
				{
					if(SServerLocks[lockID] -> GetLockState() == FREE)
					{
						sendResponse = true;
					}
					
					SServerLocks[lockID] -> Acquire(inPktHdr.from, inMailHdr.from);
				}
				else
				{
					// The lock was either deleted or uninitialized
					msgStream << requestType << " 0 " << "BadLock" << endl; // create the response for the client
					response = const_cast<char *>(msgStream.str().c_str());
					
					outPktHdr.to = inPktHdr.from;
					outMailHdr.to = inMailHdr.from;
					outMailHdr.from = 0;
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
					outMailHdr.from = 0;
					outMailHdr.length = strlen(response) + 1;
				
					// send the response to the client telling it has acquired the lock
					postOffice -> Send(outPktHdr, outMailHdr, response);
				}
			}

			//delete[] lockName;
		}
		
		if(!strcmp(requestType,"LOCK_REL"))
		{
			int lockID;
			char *lockName = new char[20];
			msgStream >> lockName;

			lockID = FindSLocks(lockName);

			response = new char[40];
			msgStream.clear();
			msgStream.str("");
			
			if((lockID > LOCK_SIZE - 1) || (lockID < 0)) // lock is an invalid lock
			{
				msgStream << requestType << " 0 " << "BadLock" << endl; // create the response for the client
				response = const_cast<char *>(msgStream.str().c_str());
				
				outPktHdr.to = inPktHdr.from;
				outMailHdr.to = inMailHdr.from;
				outMailHdr.from = 0;
				outMailHdr.length = strlen(response) + 1;
				
				// send the resposne to the client telling its request cannot be met as the lock is invalid
				postOffice -> Send(outPktHdr, outMailHdr, response);
			}
			else // the lock is in the valid range
			{
				if(SServerLocks[lockID] == NULL) // validate that the client is no passing invalid data;
				{
					msgStream << requestType << " 0 " << "BadLock" << endl; // create the response for the client
					response = const_cast<char *>(msgStream.str().c_str());
				
					outPktHdr.to = inPktHdr.from;
					outMailHdr.to = inMailHdr.from;
					outMailHdr.from = 0;
					outMailHdr.length = strlen(response) + 1;
				
					// send the resposne to the client telling its request cannot be met as the lock is invalid
					postOffice -> Send(outPktHdr, outMailHdr, response);
				}
				else
				{// the lock is valid we can release the lock
					Response *resp = SServerLocks[lockID] -> Release(inPktHdr.from, inMailHdr.from);
					
					//printf("\nThe response generated at release is\n Network ID : %d MailboxID : %d\n", resp -> networkID, resp -> mailboxID);
					msgStream << requestType << " 1 " << "OK" << endl;
					response = const_cast<char *>(msgStream.str().c_str());
					outPktHdr.to = inPktHdr.from;
					outMailHdr.to = inMailHdr.from;
					outMailHdr.from = 0;
					outMailHdr.length = strlen(response) + 1;
				
					// send the response to the client telling it to proceed
					postOffice -> Send(outPktHdr, outMailHdr, response);
					
					//printf("\nThe response generated at release is\n Network ID : %d MailboxID : %d\n", resp -> networkID, resp -> mailboxID);
					
					if((resp -> networkID != -1) && (resp -> mailboxID != -1) && !((resp->networkID == inPktHdr.from) && (resp->mailboxID == inMailHdr.from)))
					{
						printf("Releasing the response for blocked client");
						PacketHeader pkHdr;
						MailHeader mailHdr;
						
						pkHdr.to = resp -> networkID; // wake up a client blocked on the lock
						mailHdr.to = resp -> mailboxID;
						mailHdr.from = 0;
						
						int status = resp -> isError ? 0 : 1;
						
						msgStream.clear();
						msgStream.str("");
						
						msgStream << "LOCK_ACQ " << status << " " << resp -> response << " Z" << endl;
						
						
						response = const_cast<char *>(msgStream.str().c_str());
						mailHdr.length = strlen(response) + 1;
				
						// send the response to the client telling it to proceed
						postOffice -> Send(pkHdr, mailHdr, response);
						
					}
					
					delete resp;
					
					// perform cleanup
					// check if delete was called on the lock. If it was then delete the lock
					if((SServerLocks[lockID] -> deleteCalled) && (SServerLocks[lockID] -> refCount == 0))
					{
						delete SServerLocks[lockID];
						// clear the entry in the lock table
						lockBitMap.Clear(lockID);
						SServerLocks[lockID] = NULL;
					}
				}
			}

			//delete[] lockName;
		}
		
		if(!strcmp(requestType,"LOCK_DES"))
		{
			int lockID;
			char *lockName = new char[20];
			msgStream >> lockName;

			lockID = FindSLocks(lockName);

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
				outMailHdr.from = 0;
				outMailHdr.length = strlen(response) + 1;
				
				// send the resposne to the client telling its request cannot be met as the lock is invalid
				postOffice -> Send(outPktHdr, outMailHdr, response);
			}
			else
			{
				if(SServerLocks[lockID] == NULL)
				{
					msgStream << requestType << " 0 " << "BadLock" << endl; // create the response for the client
					response = const_cast<char *>(msgStream.str().c_str());
				
					outPktHdr.to = inPktHdr.from;
					outMailHdr.to = inMailHdr.from;
					outMailHdr.from = 0;
					outMailHdr.length = strlen(response) + 1;
				
					// send the resposne to the client telling its request cannot be met as the lock is invalid
					postOffice -> Send(outPktHdr, outMailHdr, response);
				}
				else
				{
					if(SServerLocks[lockID] -> refCount == 0)
					{
						delete SServerLocks[lockID];
						// clear the entry in the lock table
						lockBitMap.Clear(lockID);
						SServerLocks[lockID] = NULL;
					}
					else
					{
						SServerLocks[lockID] -> deleteCalled = true;
					}
					// send an ok status to the client
					msgStream << requestType << " 1 OK"  << endl;
					response = const_cast<char *>(msgStream.str().c_str());
					
					outPktHdr.to = inPktHdr.from;
					outMailHdr.to = inMailHdr.from;
					outMailHdr.from = 0;
					outMailHdr.length = strlen(response) + 1;
					
					// send the response to client to unblock it
					postOffice -> Send(outPktHdr, outMailHdr, response);
				}
			}

			//delete[] lockName;
		}
		
		if(!strcmp(requestType,"CV_CREATE"))
		{
			response = new char[40];
			char *cvName = new char[20];
			
			msgStream >> cvName;
			
			// check if the condition variables with that name already exists
			int cvID = FindSCondition(cvName);
			
			bool cvExists = true;
			if(cvID == -1) // the cv hasn't been created		
			{
				// find if a SServerCondition is available
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
					SServerCondition *cv = new SServerCondition(cvName);
					SServerConditions[cvID] = cv;
				}
				
				printf("The Server Condition is %d\n", cvID);
				
				msgStream << requestType << " 1 " << cvID;
			
				delete requestType;
			
				response = const_cast<char *>(msgStream.str().c_str());
			
				printf("\nThe response is %s\n", response);
				outPktHdr.to = inPktHdr.from;
				outMailHdr.to = inMailHdr.from;
				outMailHdr.from = 0;
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
				outMailHdr.from = 0;
				outMailHdr.length = strlen(response) + 1;
			
				postOffice -> Send(outPktHdr, outMailHdr, response);
			}
			
		}
		if(!strcmp(requestType,"CV_WT"))
		{
			//extract the CV and Lock number
			
			int lockID, cvID;

			char *cvName = new char[20];
			char *lockName = new char[20];
			
			msgStream >> cvName >> lockName;

			cvID = FindSCondition(cvName);
			lockID = FindSLocks(lockName);
			
			msgStream.clear();
			msgStream.str("");
			
			if((cvID < 0) || (cvID > CV_SIZE))
			{
				msgStream << requestType << " 0 BadCV" << endl;
				response = const_cast<char *>(msgStream.str().c_str());
				
				// create an error response and send it to the client
				outPktHdr.to = inPktHdr.from;
				outMailHdr.to = inMailHdr.from;
				outMailHdr.from = 0;
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
				outMailHdr.from = 0;
				outMailHdr.length = strlen(response) + 1;
				postOffice -> Send(outPktHdr, outMailHdr, response);
			}
			else
			{ // this is a valid combination of lockID and CV ID
				// check if someone is trying to break the code by passing valid ids which haven't been created yet
				
				if(SServerLocks[lockID] == NULL || SServerConditions[cvID] == NULL)
				{
					// create a generic message
					msgStream << requestType << " 0 BadInput" << endl;
					response = const_cast<char *>(msgStream.str().c_str());
					
					//create an error response and send it to client
					outPktHdr.to = inPktHdr.from;
					outMailHdr.to = inMailHdr.from;
					outMailHdr.from = 0;
					outMailHdr.length = strlen(response) + 1;
					postOffice -> Send(outPktHdr, outMailHdr, response);
				}
				else
				{ // we have a valid CV and a valid Lock
					if(SServerConditions[cvID] -> deleteCalled == true) 
					{
						msgStream << requestType << " 0 CVDeleted" << endl;
						
						response = const_cast<char *>(msgStream.str().c_str());
						
						//create an error response and send it to client
						outPktHdr.to = inPktHdr.from;
						outMailHdr.to = inMailHdr.from;
						outMailHdr.from = 0;
						outMailHdr.length = strlen(response) + 1;
						postOffice -> Send(outPktHdr, outMailHdr, response);
					}
					else
					{	// check if delete was called on the CV
						Response *resp = SServerConditions[cvID] -> Wait(lockID, inPktHdr.from, inMailHdr.from);
					
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
							outMailHdr.from = 0;
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
								outMailHdr.from = 0;
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
			
			char *cvName = new char[20];
			char *lockName = new char[20];
			
			msgStream >> cvName >> lockName;

			cvID = FindSCondition(cvName);
			lockID = FindSLocks(lockName);

			msgStream.clear();
			msgStream.str("");
			
			if((cvID < 0) || (cvID > CV_SIZE))
			{
				msgStream << requestType << " 0 BadCV" << endl;
				response = const_cast<char *>(msgStream.str().c_str());
				
				// create an error response and send it to the client
				outPktHdr.to = inPktHdr.from;
				outMailHdr.to = inMailHdr.from;
				outMailHdr.from = 0;
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
				outMailHdr.from = 0;
				outMailHdr.length = strlen(response) + 1;
				postOffice -> Send(outPktHdr, outMailHdr, response);
			}
			else
			{
				// this is a valid combination of lockID and CV ID
				// check if someone is trying to break the code by passing valid ids which haven't been created yet
				
				if(SServerLocks[lockID] == NULL || SServerConditions[cvID] == NULL)
				{
					// create a generic message
					msgStream << requestType << " 0 BadInput" << endl;
					response = const_cast<char *>(msgStream.str().c_str());
					
					//create an error response and send it to client
					outPktHdr.to = inPktHdr.from;
					outMailHdr.to = inMailHdr.from;
					outMailHdr.from = 0;
					outMailHdr.length = strlen(response) + 1;
					postOffice -> Send(outPktHdr, outMailHdr, response);
					
				}
				else
				{	// the lock and condition variable is valid
					// signal the client threads waiting on the CV
					
					bool result = SServerConditions[cvID] -> Signal(lockID);
					
					if(!result) // there was some error while signalling
					{
						// create a generic error message and send it to client
						msgStream << requestType << "0 BadInput" << endl;
					}
					else
					{
						// create an OK status message to send to the client
						msgStream << requestType << " 1 OK" << endl;
					}
					
					// send the response to the client
					response = const_cast<char *>(msgStream.str().c_str());
					
					outPktHdr.to = inPktHdr.from;
					outMailHdr.to = inMailHdr.from;
					outMailHdr.from = 0;
					outMailHdr.length = strlen(response) + 1;
					postOffice -> Send(outPktHdr, outMailHdr, response);
					
					// perform clean up. Check if some one has called delete on the conditiob variable
					if((SServerConditions[cvID] -> refCount == 0) && (SServerConditions[cvID] -> deleteCalled))
					{
						delete SServerConditions[cvID];
						CVBitMap.Clear(cvID);
						SServerConditions[cvID] = NULL;
					}
				}
			}
		}
		
		if(!strcmp(requestType, "CV_BCST"))
		{
			int lockID, cvID;
			
			char *cvName = new char[20];
			char *lockName = new char[20];
			
			msgStream >> cvName >> lockName;

			cvID = FindSCondition(cvName);
			lockID = FindSLocks(lockName);
			
			msgStream.clear();
			msgStream.str("");
			
			if((cvID < 0) || (cvID > CV_SIZE))
			{
				msgStream << requestType << " 0 BadCV" << endl;
				response = const_cast<char *>(msgStream.str().c_str());
				
				// create an error response and send it to the client
				outPktHdr.to = inPktHdr.from;
				outMailHdr.to = inMailHdr.from;
				outMailHdr.from = 0;
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
				outMailHdr.from = 0;
				postOffice -> Send(outPktHdr, outMailHdr, response);
			}
			else
			{
				// this is a valid combination of lockID and CV ID
				// check if someone is trying to break the code by passing valid ids which haven't been created yet
				
				if(SServerLocks[lockID] == NULL || SServerConditions[cvID] == NULL)
				{
					// create a generic message
					msgStream << requestType << " 0 BadInput" << endl;
					response = const_cast<char *>(msgStream.str().c_str());
					
					//create an error response and send it to client
					outPktHdr.to = inPktHdr.from;
					outMailHdr.to = inMailHdr.from;
					outMailHdr.length = strlen(response) + 1;
					outMailHdr.from = 0;
					postOffice -> Send(outPktHdr, outMailHdr, response);
					
				}
				else
				{	// the lock and condition variable is valid
					// signal the client threads waiting on the CV
					
					bool result = SServerConditions[cvID] -> Broadcast(lockID);
					
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
					response = const_cast<char *>(msgStream.str().c_str());
					
					outPktHdr.to = inPktHdr.from;
					outMailHdr.to = inMailHdr.from;
					outMailHdr.length = strlen(response) + 1;
					outMailHdr.from = 0;
					postOffice -> Send(outPktHdr, outMailHdr, response);
					
					// perform clean up. Check if some one has called delete on the conditiob variable
					if((SServerConditions[cvID] -> refCount == 0) && (SServerConditions[cvID] -> deleteCalled))
					{
						delete SServerConditions[cvID];
						CVBitMap.Clear(cvID);
						SServerConditions[cvID] = NULL;
					}
				}
			}
		}
		if(!strcmp(requestType, "CV_DEL"))
		{
			int cvID;
			
			char *cvName = new char[20];
			
			msgStream >> cvName;
			
			// check if the condition variables with that name already exists
			cvID = FindSCondition(cvName);
						
			msgStream.clear();
			msgStream.str("");
			
			printf("\nDeleting condition variable %d\n", cvID);
			
			if((cvID < 0) || (cvID > CV_SIZE - 1) || (SServerConditions[cvID] == NULL))// if the cv is out of range
			{
				msgStream << requestType << " 0 BadCV" << endl;
				response = const_cast<char *>(msgStream.str().c_str());
				
				// create an error response and send it to the client
				outPktHdr.to = inPktHdr.from;
				outMailHdr.to = inMailHdr.from;
				outMailHdr.from = 0;
				outMailHdr.length = strlen(response) + 1;
				
				postOffice -> Send(outPktHdr, outMailHdr, response);
			}
			else
			{
				// the cv is a valid CV
				
				// check if the reference count is 0
				if(SServerConditions[cvID] -> refCount == 0)
				{
					delete SServerConditions[cvID];
					CVBitMap.Clear(cvID);
					SServerConditions[cvID] = NULL;
				}
				else
				{
					SServerConditions[cvID] -> deleteCalled = true;
				}

				msgStream << requestType << " 1 OK" << endl;
				response = const_cast<char *>(msgStream.str().c_str());
				
				// create an error response and send it to the client
				outPktHdr.to = inPktHdr.from;
				outMailHdr.to = inMailHdr.from;
				outMailHdr.length = strlen(response) + 1;
				outMailHdr.from = 0;
				
				postOffice -> Send(outPktHdr, outMailHdr, response);
				
			}
			
		}
		if(!strcmp(requestType, "MV_CREATE"))
		{
			response = new char[40];
			
			char *mvName = new char[20];
			
			msgStream >> mvName;
			
			// check if the monitor variable with that name already exists
			int mvID = FindSMV(mvName);
			
			bool mvExists = true;
			if(mvID == -1) // the cv hasn't been created		
			{
				// find if a SServerCondition is available
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
				outMailHdr.from = 0;
				
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
				outMailHdr.from = 0;
				
				postOffice -> Send(outPktHdr, outMailHdr, response); 
			}
			
		}
		
		if(!strcmp(requestType, "MV_GET"))
		{
			int mvID;
			
			char *mvName = new char[20];
			
			msgStream >> mvName;
			
			// check if the monitor variable with that name already exists
			mvID = FindSMV(mvName);
			
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
				outMailHdr.from = 0;
				
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
				outMailHdr.from = 0;
				
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
			
			char *mvName = new char[20];
			
			msgStream >> mvName >> val;
			
			// check if the monitor variable with that name already exists
			mvID = FindSMV(mvName);
			
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
				outMailHdr.from = 0;
				
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
				outMailHdr.from = 0;
				
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
			
			char *mvName = new char[20];
			
			msgStream >> mvName;
			
			// check if the monitor variable with that name already exists
			mvID = FindSMV(mvName);
			
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
				outMailHdr.from = 0;
				
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
				outMailHdr.from = 0;
				
				postOffice -> Send(outPktHdr, outMailHdr, response);
			}
		}
		//parse the content of the message
		// send a reply if required
	}
}
