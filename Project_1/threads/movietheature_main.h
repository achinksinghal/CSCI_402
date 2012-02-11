#ifndef __MOVIETHEATURE_MAIN_C__
#define __MOVIETHEATURE_MAIN_C__

#define MAX_FILE_COMPLETE_NAME_SIZE 2000
#define MAX_NUMBER_OF_DEBUG_THREADS 1405
#define MIN_NUMBER_OF_DEBUG_THREADS 0
#define INVALID_DEBUG_ID 0 
#define MAX_FILE_SIZE 500000
#define IN_USE 1
#define NOT_IN_USE 0
#define RETURN_OK 1
#define RETURN_NOK 0
#define FILENAME_ALREADY_IN_USE 2
#define MAX_DEBUG_THREADS_REACHED 3

#define MAX_TC 5
#define MAX_TT 3
#define MAX_CC 5
#define MAX_CUST 10
#define MAX_CUST_GRP 40
#define MAX_NUM_QUEUE 5
#define CUST_MONEY 5000
#define PER_TICKET_COST 12
#define PER_POPCORN_COST 5
#define PER_SODA_COST 4
#define MAX_SEATS 25
#define MAX_ROWS 5
#define MAX_COLS 5

// This are the debug levels
// for debgu logs.
enum DebugLevel
{
	DEBUG_LEVEL_1 = 1,
	DEBUG_LEVEL_2 = 2,
	DEBUG_LEVEL_3 = 4,
	DEBUG_LEVEL_4 = 8,
	DEBUG_LEVEL_5 = 16,
	DEBUG_LEVEL_6 = 32,
	DEBUG_LEVEL_7 = 64,
        DEBUG_LEVEL_MAX = 128
};

// This the file number for debug 
// logs.
enum DebugFileNo
{
	DEBUG_FILE_NO_1 = 1,
	DEBUG_FILE_NO_2 = 2,
	DEBUG_FILE_NO_3 = 4,
	DEBUG_FILE_NO_4 = 8,
	DEBUG_FILE_NO_5 = 16,
	DEBUG_FILE_NO_6 = 32,
	DEBUG_FILE_NO_7 = 64,
        DEBUG_FILE_NO_MAX = 128
};

//this is control
//structure for 
//debug logging.
struct DebugControlBlock
{
        char debugFileCompleteName[MAX_FILE_COMPLETE_NAME_SIZE][MAX_NUMBER_OF_DEBUG_THREADS];
        FILE *currentDebugFilePtr[MAX_NUMBER_OF_DEBUG_THREADS];
        int recommendedFileSize[MAX_NUMBER_OF_DEBUG_THREADS];
        char inUse[MAX_NUMBER_OF_DEBUG_THREADS];
};

//functions which are going to be called in
//the haeders of logging. 
 int debug_init(const char *file_name, int *debugId);
void writeDebugPrints(int debugId, char *debugLevel, char *debugFileNo, const char *functionName, int lineNumber, char *format, ... );
void debug_close(int debugId);

// each one is to to check whether the 
// debug level set allows the data to 
// be printed
#define CHECK_PRINT_DEBUG_LVL_1(debugLevel)  if(debugLevel & DEBUG_LEVEL_1)
#define CHECK_PRINT_DEBUG_LVL_2(debugLevel)  if(debugLevel & DEBUG_LEVEL_2)
#define CHECK_PRINT_DEBUG_LVL_3(debugLevel)  if(debugLevel & DEBUG_LEVEL_3)
#define CHECK_PRINT_DEBUG_LVL_4(debugLevel)  if(debugLevel & DEBUG_LEVEL_4)
#define CHECK_PRINT_DEBUG_LVL_5(debugLevel)  if(debugLevel & DEBUG_LEVEL_5)
#define CHECK_PRINT_DEBUG_LVL_6(debugLevel)  if(debugLevel & DEBUG_LEVEL_6)
#define CHECK_PRINT_DEBUG_LVL_7(debugLevel)  if(debugLevel & DEBUG_LEVEL_7)

#define CHECK_PRINT_DEBUG_LVL_SYS  CHECK_PRINT_DEBUG_LVL_1 
#define CHECK_PRINT_DEBUG_LVL_ERR  CHECK_PRINT_DEBUG_LVL_2 
#define CHECK_PRINT_DEBUG_LVL_EXCP CHECK_PRINT_DEBUG_LVL_3 
#define CHECK_PRINT_DEBUG_LVL_WARN CHECK_PRINT_DEBUG_LVL_4 
#define CHECK_PRINT_DEBUG_LVL_INFO CHECK_PRINT_DEBUG_LVL_5 
#define CHECK_PRINT_DEBUG_LVL_DATA CHECK_PRINT_DEBUG_LVL_6 
#define CHECK_PRINT_DEBUG_LVL_VERB CHECK_PRINT_DEBUG_LVL_7 

// each one is to check 
// whether file no set allows 
// the data to be printed
#define CHECK_PRINT_DEBUG_FILE_NO_1(debugFileNo) if(debugFileNo & DEBUG_FILE_NO_1)
#define CHECK_PRINT_DEBUG_FILE_NO_2(debugFileNo) if(debugFileNo & DEBUG_FILE_NO_2)
#define CHECK_PRINT_DEBUG_FILE_NO_3(debugFileNo) if(debugFileNo & DEBUG_FILE_NO_3)
#define CHECK_PRINT_DEBUG_FILE_NO_4(debugFileNo) if(debugFileNo & DEBUG_FILE_NO_4)
#define CHECK_PRINT_DEBUG_FILE_NO_5(debugFileNo) if(debugFileNo & DEBUG_FILE_NO_5)

// debug logging printing macro
#define PRINT_DEBUG(debugId, debugLevel, debugFileNo, format, arg... ) \
{   \
    writeDebugPrints(debugId, debugLevel, debugFileNo, __FUNCTION__, __LINE__, format, ##arg );\
}   \

// to initialize logging 
// for movie theater
#define DEBUG_INIT(file_name, debugId) \
{ \
	if(gDebugLevel != 0)\
	{\
		ret_val=debug_init(file_name, debugId); \
		if(ret_val != RETURN_OK) \
		return; \
	}\
}\

//to assign debugId.
#define MAIN_DEBUG_ID(value) \
{   \
    int debugId = value; \
}   \

//to print verbose print
#define MAIN_PRINT_DEBUG_VERB(format, arg... ) \
{   \
	if(gDebugLevel != 0)\
	{\
		CHECK_PRINT_DEBUG_LVL_VERB(gDebugLevel) \
		CHECK_PRINT_DEBUG_FILE_NO_1(gDebugFileNo) \
		PRINT_DEBUG(debugId, "VERB\0", "mt_main\0", format, ##arg ) \
	}\
} 

//to print info print
#define MAIN_PRINT_DEBUG_INFO(format, arg... ) \
{   \
	if(gDebugLevel != 0)\
	{\
		CHECK_PRINT_DEBUG_LVL_INFO(gDebugLevel) \
		CHECK_PRINT_DEBUG_FILE_NO_1(gDebugFileNo) \
		PRINT_DEBUG(debugId, "INFO\0", "mt_main\0", format, ##arg ) \
	}\
}

// to print warning prints
#define MAIN_PRINT_DEBUG_WARN(format, arg... ) \
{   \
	if(gDebugLevel != 0)\
	{\
		CHECK_PRINT_DEBUG_LVL_WARN(gDebugLevel) \
		CHECK_PRINT_DEBUG_FILE_NO_1(gDebugFileNo) \
		PRINT_DEBUG(debugId, "WARN\0", "mt_main\0", format, ##arg ) \
	}\
}

//to print error prints
#define MAIN_PRINT_DEBUG_ERR(format, arg... ) \
{   \
	if(gDebugLevel != 0)\
	{\
		CHECK_PRINT_DEBUG_LVL_ERR(gDebugLevel) \
		CHECK_PRINT_DEBUG_FILE_NO_1(gDebugFileNo) \
		PRINT_DEBUG(debugId, "ERRR\0", "mt_main\0", format, ##arg ) \
	}\
}

// to close the debug logging 
// for a thread with a debugId
#define DEBUG_CLOSE(debugId) \
{ \
	if(gDebugLevel != 0)\
	{\
		debug_close(debugId); \
	}\
} \

typedef void* Employee;
typedef void* CUST;
void initializeAllLocks();

//various states in simulation 
enum State
{
	INITIAL_STATE=0, //for customer
	IN_TICKET_QUEUE, //for customer
	SELLING_TICKETS, //for employee(ticket clerk)
	WAIT_FOR_CUST, //for employee(ticket clerk)
	ENGAGED_WITH_CUST, //for employee(ticket clerk)
	QUEUE_BUILDING, //for employee(ticket clerk)
	WAIT, //for customer grp
	WAIT_FOR_TC, //for customer
	WAIT_FOR_TC_BEING_FIRST, //for customer
	WAIT_FOR_CC_BEING_FIRST, //for customer
	WAIT_FOR_CC, //for customer
	WAIT_FOR_TICKET_BUYER_FROM_TC, //for customer
	WAIT_FOR_TICKET_BUYER_FROM_CC, //for customer
	WAIT_FOR_TC_ORDERED_PLACED, //for customer
	SIGNAL_BY_CUST_ORDERED_PLACED, //for ticket clerk
	SIGNAL_TO_CHANGE_TO_ANY_OTHER_QUEUE,
	SIGNAL_ENGAGED_WITH_CUST, //for ticket clerk
	SIGNAL_ENGAGED_WITH_TC, //for cust
	SIGNAL_ENGAGED_WITH_CC, //for cust
	WAIT_ENGAGED_WITH_CUST, //for ticket clerk
	WAIT_ENGAGED_WITH_TC, //for cust
	WAIT_ENGAGED_WITH_CC, //for cust
	SIGNAL, //for cust
	SIGNAL_BY_TC, //for cust
	SIGNAL_BY_CC, //for cust
	SIGNAL_BY_CUST, //for ticket clerk
	STARTED_BY_MAN, //for employee(ticket clerk)
	STARTED_BY_MAIN, //for cust(ticket clerk)
	GOT_TICKET, //for group and cust
	GOT_FOOD, //for group and cust
	LOOKING_FOR_QUEUE, // for customer
	LOOKING_FOR_TC_QUEUE, // for customer
	LOOKING_FOR_CC_QUEUE, // for customer
	FREE_AND_TAKING_TICKET_BEING_FIRST, // for ticket taker
	FREE_AND_TAKING_TICKET, // for ticket taker
	FREE_AND_SELLING_TICKET_BEING_FIRST, // for ticket clerk
	FREE_AND_SELLING_TICKET, //for ticket clerk
	FREE_AND_SELLING_FOOD_BEING_FIRST, // concession clerk
	FREE_AND_SELLING_FOOD,// for concession clerk
	MAN_STARTING_MOVIE, // for manager
	LOOKING_FOR_TT, // for customer
	WAIT_FOR_TICKET_BUYER_FROM_TT,// for customer
	BUSY_WITH_CUST, // for employees
	WAIT_IN_LOBBY_AS_TICKET_NOT_TAKEN_BY_TT, // for customers
	TICKET_TAKEN, // for customers
	WAIT_FOR_TICKET_BUYER_TILL_SEATS_TAKEN, // for customers
	LOOKING_FOR_SEATS,// for customers
	SEATS_READY,// for customers
	SEAT_TAKEN,// for customers
	MOVIE_IS_PLAYING, // for movie technician
	MOVIE_IS_NOT_PLAYING, //for movie technician
	NEW_MOVIE_STARTING_TICKET_MAY_BE_TAKEN, // for customers
	READY_TO_LEAVE_MOVIE_ROOM, //for customers
	LEFT_MOVIE_ROOM_AFTER_MOVIE, // for customers
	HEADING_FOR_BATHROOM, // for customers
	WAIT_AS_TICKET_BUYER_HEADING_FOR_BATHROOM,// for customers
	NO_MOVIE_REQD, // for movie technician

	USING_BATHROOM, // for customers
	READY_TO_GO_OUT_OF_MT, // for movie technician
	CUSTOMER_SIMULATION_COMPLETED, // for customer

	IN_FOOD_QUEUE, //for customer
	SELLING_FOOD, //for employee(concession clerk)
	WAIT_IN_LOBBY_MOVIE_NOT_STARTED, //for customer
	WAIT_IN_LOBBY_AS_SOME_ONE_BATHROOM, //for customer
	AT_BATHROOM, //for customer
	AT_MOVIE_ROOM, //for customer
	AT_MOVIE_ROOM_SEAT, //for customer
	ON_BREAK, //for employee
	OFF_BREAK, //for employee
	WITH_TICKET_CLERK, //for tickets
	WITH_CUSTOMER, //for tickets
	WITH_TICKET_TAKER, //for tickets
	AVAILABLE_TO_BE_SOLD_OUT, //for tickets
	OUT_OF_THEATURE, //for customer, customer_group
	NO_CUSTOMER_NO_ADDRESS, //for queue
	NO_CUSTOMER, //for queue
	NO_ADDRESS, //for queue
	ROK, //for queue
	NOK, //for queue
	INVALID_STATE // for all 
};

/*
 * This are the states 
 * prints for logging 
 * debug prints 
 * */
char strState[][50]=
{
	"INITIAL_STATE\0", 
	"IN_TICKET_QUEUE\0", //for customer
	"SELLING_TICKETS\0", //for employee(ticket clerk)
	"WAIT_FOR_CUST\0", //for employee(ticket clerk)
	"ENGAGED_WITH_CUST\0", //for employee(ticket clerk)
	"QUEUE_BUILDING\0", //for employee(ticket clerk)
	"WAIT", //for customer grp
	"WAIT_FOR_TC\0", //for customer
	"WAIT_FOR_TC_BEING_FIRST\0", //for customer
	"WAIT_FOR_CC_BEING_FIRST\0", //for customer
	"WAIT_FOR_CC\0", //for customer
	"WAIT_FOR_TICKET_BUYER_FROM_TC\0", //for customer
	"WAIT_FOR_TICKET_BUYER_FROM_CC\0", //for customer
	"WAIT_FOR_TC_ORDERED_PLACED\0", //for customer
	"SIGNAL_BY_CUST_ORDERED_PLACED\0", //for ticket clerk
	"SIGNAL_TO_CHANGE_TO_ANY_OTHER_QUEUE\0",
	"SIGNAL_ENGAGED_WITH_CUST\0", //for ticket clerk
	"SIGNAL_ENGAGED_WITH_TC\0", //for cust
	"SIGNAL_ENGAGED_WITH_CC\0", //for cust
	"WAIT_ENGAGED_WITH_CUST\0", //for ticket clerk
	"WAIT_ENGAGED_WITH_TC\0", //for cust
	"WAIT_ENGAGED_WITH_CC\0", //for cust
	"SIGNAL", //for cust
	"SIGNAL_BY_TC\0", //for cust
	"SIGNAL_BY_CC\0", //for cust
	"SIGNAL_BY_CUST\0", //for ticket clerk
	"STARTED_BY_MAN\0", //for employee(ticket clerk)
	"STARTED_BY_MAIN\0", //for cust(ticket clerk)
	"GOT_TICKET\0", //for group and cust
	"GOT_FOOD\0", //for group and cust
	"LOOKING_FOR_QUEUE\0", // for customer
	"LOOKING_FOR_TC_QUEUE\0", // for customer
	"LOOKING_FOR_CC_QUEUE\0", // for customer
	"FREE_AND_TAKING_TICKET_BEING_FIRST\0",
	"FREE_AND_TAKING_TICKET\0",
	"FREE_AND_SELLING_TICKET_BEING_FIRST\0",
	"FREE_AND_SELLING_TICKET\0",
	"FREE_AND_SELLING_FOOD_BEING_FIRST\0",
	"FREE_AND_SELLING_FOOD\0",
	"MAN_STARTING_MOVIE\0",
	"LOOKING_FOR_TT\0",
	"WAIT_FOR_TICKET_BUYER_FROM_TT\0",
	"BUSY_WITH_CUST\0",
	"WAIT_IN_LOBBY_AS_TICKET_NOT_TAKEN_BY_TT\0",
	"TICKET_TAKEN\0",
	"WAIT_FOR_TICKET_BUYER_TILL_SEATS_TAKEN\0",
	"LOOKING_FOR_SEATS\0",
	"SEATS_READY\0",
	"SEAT_TAKEN\0",
	"MOVIE_IS_PLAYING\0",
	"MOVIE_IS_NOT_PLAYING\0",
	"NEW_MOVIE_STARTING_TICKET_MAY_BE_TAKEN\0",
	"READY_TO_LEAVE_MOVIE_ROOM\0",
	"LEFT_MOVIE_ROOM_AFTER_MOVIE\0",
	"HEADING_FOR_BATHROOM\0",
	"WAIT_AS_TICKET_BUYER_HEADING_FOR_BATHROOM\0",
	"NO_MOVIE_REQD\0",
	"USING_BATHROOM\0",
	"READY_TO_GO_OUT_OF_MT\0",
	"CUSTOMER_SIMULATION_COMPLETED\0",
	"IN_FOOD_QUEUE\0", //for customer
	"SELLING_FOOD\0", //for employee(concession clerk)
	"WAIT_IN_LOBBY_MOVIE_NOT_STARTED\0", //for customer
	"WAIT_IN_LOBBY_AS_SOME_ONE_BATHROOM\0", //for customer
	"AT_BATHROOM\0", //for customer
	"AT_MOVIE_ROOM\0", //for customer
	"AT_MOVIE_ROOM_SEAT\0", //for customer
	"ON_BREAK\0", //for employee
	"OFF_BREAK\0", //for employee
	"WITH_TICKET_CLERK\0", //for tickets
	"WITH_CUSTOMER\0", //for tickets
	"WITH_TICKET_TAKER\0", //for tickets
	"AVAILABLE_TO_BE_SOLD_OUT\0", //for tickets
	"OUT_OF_THEATURE\0", //for customer, customer_group
	"NO_CUSTOMER_NO_ADDRESS\0", //for queue
	"NO_CUSTOMER\0", //for queue
	"NO_ADDRESS\0", //for queue
	"ROK\0", //for queue
	"NOK\0", //for queue
	"INVALID_STATE\0"
};

/* 
 * These are the messages strings 
 * for all message for logging prints.
 * */
char strMsg[][20]=
{
	"INITIAL_MSG\0", //for customer
	"GO_ON_BREAK\0",
	"GO_OFF_BREAK\0",
	"YES\0",
	"NO\0",
	"FILL_CUST\0",
	"START_MOVIE\0",
	"MOVIE_RUNNING\0",
	"MOVIE_NOT_RUNNING\0",
	"MOVIE_OVER\0",
	"STARTING_MOVIE\0",
	"SELL_TICKETS\0", //for employee(ticket clerk)
	"SELL_FOOD\0", //for employee(ticket clerk)
	"INVALID_MSG\0"
};

/*
 * Various Messages
 *
 * */
enum Msg
{
	INITIAL_MSG=0, //for customer
	GO_ON_BREAK,//for the clerks
	GO_OFF_BREAK, //for the clerks
	YES, //for ticket taker and customer interaction
	NO,//for ticket taker and customer interaction
	FILL_CUST,//for ticket taker and manager interaction
	START_MOVIE,//for movie technician
	MOVIE_RUNNING,//for movie technician and customers
	MOVIE_NOT_RUNNING,//for movie technician
	MOVIE_OVER,//for movie technician and customers
	STARTING_MOVIE,//for movie technician
	SELL_TICKETS, //for employee(ticket clerk)
	SELL_FOOD, //for employee(ticket clerk)
	INVALID_MSG //to nullify a received msg, ensuring that 1 msg is received only once
};

enum Location
{//to keep track of where the customer and manager are  currently 
	START,
	CONCESSION_CLERK,
	TICKET_CLERK,
	LOBBY,
	BATHROOM,
	OUT_OF_MOVIEROOM,
	OUT_OF_MOVIETHEATURE,
	MOVIEROOM
};

enum QueueType
{//type of possible queues
	TC_QUEUE=0,
	CC_QUEUE,
	TT_QUEUE,
	MAX_QUEUE_TYPE,
	INVALID_QUEUE
};

typedef struct _mtQueue
{// Movie theater queue - a general
	QueueType queueType;
        List *queue;
        int  queueId;
	unsigned int numCust;
	Employee queueAddress;
	State state;
}mtQueue;

typedef struct _Manager
{
        Thread *selfThread; // a pointer to manager's own thread
	Location location;//where the manageris right now
	State state;//state of manager
	Msg msgToAll;//transmit message to all entitites
	unsigned int money;//money collected
}Manager;


typedef struct _Ticket
{
	int roomNum;//room number of movie
}Ticket;

typedef struct _Seat
{
	int roomNum;//room number of where seat is located
	CUST cust;//customer who occupies this seat
	int isTaken;//whether somebody is sat now
}Seat;

typedef struct _MvRoom
{
	Seat seat[MAX_ROWS][MAX_COLS];//total number of seats in a movie room
}MvRoom;

typedef struct _Cust
{
	int selfId;//the customer's number in the list
	int grpId;//the grp to which customer belongs
        Thread *selfThread;//a pointer to that customer itself
	Location location;//current location of customer
	State state;//state of cutomer
	unsigned int money;//money paid
	Seat *seat;//the customer's seat
	int seatTaken;//has taken seat or not
	int takeSoda;// will take Soda or not
	int IAMTicketBuyer;//the head customer who interacts with clerk
	int takePopcorn;// will take popcorn or not
	int goBathroom;// will use bathroom or not
	int msgBuffer;//to receive messages
	mtQueue *queue;//to keep track of which all queues customer gets in
}Cust;

typedef struct _Food
{
//to show buying of food
}Food;

typedef struct _CustGrp
{//to manage customer group
	Cust *cust[MAX_CUST];
        int grpId;
	Location location;
	State state;
	Cust *tBuyer;
	Ticket *ticket[MAX_CUST];
	Food *food;
        int numCust;
}CustGrp;

typedef struct _TC
{//ticket clerk
	Manager *man;//a thread to manager
	unsigned int money;//money made
        Thread *selfThread;// a thread to self
	QueueType queueType;//type of queue for clerk
	mtQueue *queue;//the actual queue
	Cust *currentCust;//pointer to current dealing customer
	int msgBuffer;//to interact with customer
	Msg msgByMan;//message from Manager
	Location location;//place where he is
	State state;//state he is in
}TC;

typedef struct _CC
{//concession clerk
	Manager *man;//a thread to manager
	unsigned int money;//money made
        Thread *selfThread;//a thread to manager
	Msg msgByMan;//message from Manager
	int msgBuffer;//to interact with customer
	Cust *currentCust;//pointer to current dealing customer
	QueueType queueType;//type of queue for clerk
	mtQueue *queue;//the actual queue
	Location location;//place where he is
	State state;//state he is in
}CC;

typedef struct _TT
{//ticket taker
	Cust *currentCust;//pointer to current dealing customer
	Manager *man;//a thread to manager
        Thread *selfThread;// a thread to self
	Msg msgByMan;//message from Manager
	Msg msgToMan;// send message to manager
	Msg msgToCust;//notify customer
	Msg msgFromCust;//interact with customer
	QueueType queueType;//type of queue for clerk
	mtQueue *queue;//the actual queue
	Location location;//place where he is
	State state;//state he is in
}TT;

typedef  struct _MovieTechnician
{//Movie technician
	Manager *man;// pointer to manager thread
	Thread *selfThread;//a thread to itself
	State state;//state he is in
	Msg msgByMan;//message from Manager
	Msg msgToMan;// send message to manager
	Location location;//place where he is
}MT;

typedef struct _MtCb//Movie theater control block
{//The main control block of theater - all entities are member of this
	int numTC;//number of ticket clerks
	int numCC;//number of concession clerks
	int numTT;//number of ticket takers
	TC *tc[MAX_TC];//setting up pointers to ticket clerks
	CC *cc[MAX_CC];//setting up pointers to concession clerks
	TT *tt[MAX_TT];//setting up pointers to ticket takers
	MT *mt;//pointer to theater
	MvRoom *mvRoom;//pointer to movie room
	int numCustGrp;//number of customer groups
	int numOfTicketsTaken;//number of tickets sold in all
	CustGrp *custGrp[MAX_CUST_GRP];//setting up pointers to customer group
	Manager *man; //setting up pointer to manager
	mtQueue *queue[MAX_QUEUE_TYPE][MAX_NUM_QUEUE];//setting up pointers to all different queues
	unsigned int numCust;//total number of customers

}MtCb;

#endif /*__MOVIETHEATURE_MAIN_C__*/
