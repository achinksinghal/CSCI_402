#include "Request.h"

//=========================================================
// Request :: Request
// Creates queued Requests sent to servers
//=========================================================

Request :: Request(int netID, int mailboxID)
{
	responseCount = 0;
	//clientRequest = new char[40];
	queuedList = new List();
	isPending = true;
	isCompleted = false;
	negResponse = 0;
	timestamp = 0;
	printf("\n****************IN request Constructor****************\n");
	response = new Response(netID, mailboxID, false, "");
}

//========================================================
//Request :: ~Request
// 
// Performs the cleans up for Request message
//========================================================
Request :: ~Request()
{
	queuedList = NULL;
	//delete clientRequest;
	delete response;
}
