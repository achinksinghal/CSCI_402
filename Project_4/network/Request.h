#ifndef REQUEST_H
#define REQUEST_H

#include "synch.h"
#include "list.h"

class Request
{
	public:
		int responseCount;
		char clientRequest[40];
		List *queuedList;
		bool isPending;
		bool isCompleted;
		int negResponse;
		long timestamp;
		Response *response;
		Response *owner;
		
		Request(int,int);
		~Request();
	
};
#endif
