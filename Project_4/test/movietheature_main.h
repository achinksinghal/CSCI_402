/*
#ifndef __MOVIETHEATURE_MAIN_C__
#define __MOVIETHEATURE_MAIN_C__
*/
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
#define MAX_CUST 5 
#define MAX_CUST_GRP 6
#define MAX_NUM_QUEUE 5
#define CUST_MONEY 5000
#define PER_TICKET_COST 12
#define PER_POPCORN_COST 5
#define PER_SODA_COST 4
#define MAX_SEATS 25
#define MAX_ROWS 5
#define MAX_COLS 5
#define MAX_NAME_LENGTH 25
#define MAX_MV_NAME_LENGTH 10


typedef void* Employee;
typedef void* CUST;
void initializeAllLocks();

typedef struct _mv
{
	char name[MAX_MV_NAME_LENGTH];/*name of the moniter variable*/
	int index;
	int value;
}MV;

MV num_tc;
MV num_cc;
MV num_tt;
MV num_cust;
MV cust_num;
MV num_custgrp;
int cust_number;

/*various states in simulation */
#define	INITIAL_STATE 0 /*for customer*/
#define	IN_TICKET_QUEUE 1 /*for customer*/
#define	SELLING_TICKETS 2 /*for employee(ticket clerk)*/
#define	WAIT_FOR_CUST 3 /*for employee(ticket clerk)*/
#define	ENGAGED_WITH_CUST 4 /*for employee(ticket clerk)*/
#define	QUEUE_BUILDING 5 /*for employee(ticket clerk)*/
#define	WAIT 6 /*for customer grp*/
#define	WAIT_FOR_TC 7 /*for customer*/
#define	WAIT_FOR_TC_BEING_FIRST  8 /*for customer*/
#define	WAIT_FOR_CC_BEING_FIRST 9 /*for customer*/
#define	WAIT_FOR_CC 10 /*for customer*/
#define	WAIT_FOR_TICKET_BUYER_FROM_TC 11 /*for customer*/
#define	WAIT_FOR_TICKET_BUYER_FROM_CC 12 /*for customer*/
#define	WAIT_FOR_TC_ORDERED_PLACED 13 /*for customer*/
#define	SIGNAL_BY_CUST_ORDERED_PLACED 14 /*for ticket clerk*/
#define	SIGNAL_TO_CHANGE_TO_ANY_OTHER_QUEUE 15
#define	SIGNAL_ENGAGED_WITH_CUST 16 /*for ticket clerk*/
#define	SIGNAL_ENGAGED_WITH_TC 17 /*for cust*/
#define	SIGNAL_ENGAGED_WITH_CC 18 /*for cust*/
#define	WAIT_ENGAGED_WITH_CUST 19 /*for ticket clerk*/
#define	WAIT_ENGAGED_WITH_TC 20 /*for cst*/
#define	WAIT_ENGAGED_WITH_CC 21 /*for cust*/
#define	SIGNAL 22 /*for cust*/
#define	SIGNAL_BY_TC 23 /*for cust*/
#define	SIGNAL_BY_CC 24 /*for cust*/
#define	SIGNAL_BY_CUST 25 /*for ticket clerk*/
#define	STARTED_BY_MAN 26/*for employee(ticket clerk)*/
#define	STARTED_BY_MAIN 27 /*for cust(ticket clerk)*/
#define	GOT_TICKET 28 /*for group and cust*/
#define	GOT_FOOD 29 /*for group and cust*/
#define	LOOKING_FOR_QUEUE 30 /* for customer*/
#define	LOOKING_FOR_TC_QUEUE 31 /* for customer*/
#define	LOOKING_FOR_CC_QUEUE 32 /* for customer*/
#define	FREE_AND_TAKING_TICKET_BEING_FIRST 33 /* for ticket taker*/
#define	FREE_AND_TAKING_TICKET 34 /* for ticket taker*/
#define	FREE_AND_SELLING_FOOD_BEING_FIRST 35 /* concession clerk*/
#define	FREE_AND_SELLING_FOOD 36/* for concession clerk*/
#define	MAN_STARTING_MOVIE 37 /* for manager*/
#define	LOOKING_FOR_TT 38 /* for customer*/
#define	WAIT_FOR_TICKET_BUYER_FROM_TT 39/* for customer*/
#define	BUSY_WITH_CUST 40 /* for employees*/
#define	WAIT_IN_LOBBY_AS_TICKET_NOT_TAKEN_BY_TT 41 /* for customers*/
#define	TICKET_TAKEN 42 /* for customers*/
#define	WAIT_FOR_TICKET_BUYER_TILL_SEATS_TAKEN 43 /* for customers*/
#define	LOOKING_FOR_SEATS 44/* for customers*/
#define	SEATS_READY 45/* for customers*/
#define	SEAT_TAKEN 46/* for customers*/
#define	MOVIE_IS_PLAYING 47 /* for movie technician*/
#define	MOVIE_IS_NOT_PLAYING 48 /*for movie technician*/
#define	NEW_MOVIE_STARTING_TICKET_MAY_BE_TAKEN 49 /* for customers*/
#define	READY_TO_LEAVE_MOVIE_ROOM 50 /*for customers*/
#define	LEFT_MOVIE_ROOM_AFTER_MOVIE 51 /* for customers*/
#define	HEADING_FOR_BATHROOM 52 /* for customers*/
#define	WAIT_AS_TICKET_BUYER_HEADING_FOR_BATHROOM 53/* for customers*/
#define	NO_MOVIE_REQD 54 /* for movie technician*/

#define	USING_BATHROOM 55 /* for customers*/
#define	READY_TO_GO_OUT_OF_MT 56 /* for movie technician*/
#define	CUSTOMER_SIMULATION_COMPLETED 57 /* for customer*/

#define	IN_FOOD_QUEUE 58 /*for customer*/
#define	SELLING_FOOD 59 /*for employee(concession clerk)*/
#define	WAIT_IN_LOBBY_MOVIE_NOT_STARTED 60 /*for customer*/
#define	WAIT_IN_LOBBY_AS_SOME_ONE_BATHROOM 61 /*for customer*/
#define	AT_BATHROOM 62 /*for customer*/
#define	AT_MOVIE_ROOM 63 /*for customer*/
#define	AT_MOVIE_ROOM_SEAT 64 /*for customer*/
#define	ON_BREAK 65 /*for employee*/
#define	OFF_BREAK 66 /*for employee*/
#define	WITH_TICKET_CLERK 67 /*for tickets*/
#define	WITH_CUSTOMER 68 /*for tickets*/
#define	WITH_TICKET_TAKER 69 /*for tickets*/
#define	AVAILABLE_TO_BE_SOLD_OUT 70 /*for tickets*/
#define	OUT_OF_THEATURE 71 /*for customer, customer_group*/
#define	NO_CUSTOMER_NO_ADDRESS 72 /*for queue*/
#define	NO_CUSTOMER 73 /*for queue*/
#define	NO_ADDRESS 74 /*for queue*/
#define	ROK 75 /*for queue*/
#define	NOK 76 /*for queue*/
#define	INVALID_STATE 77 /* for all */
#define	FREE_AND_SELLING_TICKET_BEING_FIRST 78 /* for ticket clerk*/
#define	FREE_AND_SELLING_TICKET 79 /*for ticket clerk*/
#define	WAIT_AS_OVER 80 /*for customer grp*/

/*
 * This are the states 
 * prints for logging 
 * debug prints 
 * */
char strState[][50]=
{
	"INITIAL_STATE\0", 
	"IN_TICKET_QUEUE\0", /*for customer*/
	"SELLING_TICKETS\0", /*for employee(ticket clerk)*/
	"WAIT_FOR_CUST\0", /*for employee(ticket clerk)*/
	"ENGAGED_WITH_CUST\0", /*for employee(ticket clerk)*/
	"QUEUE_BUILDING\0", /*for employee(ticket clerk)*/
	"WAIT", /*for customer grp*/
	"WAIT_FOR_TC\0", /*for customer*/
	"WAIT_FOR_TC_BEING_FIRST\0", /*for customer*/
	"WAIT_FOR_CC_BEING_FIRST\0", /*for customer*/
	"WAIT_FOR_CC\0", /*for customer*/
	"WAIT_FOR_TICKET_BUYER_FROM_TC\0", /*for customer*/
	"WAIT_FOR_TICKET_BUYER_FROM_CC\0", /*for customer*/
	"WAIT_FOR_TC_ORDERED_PLACED\0", /*for customer*/
	"SIGNAL_BY_CUST_ORDERED_PLACED\0", /*for ticket clerk*/
	"SIGNAL_TO_CHANGE_TO_ANY_OTHER_QUEUE\0",
	"SIGNAL_ENGAGED_WITH_CUST\0", /*for ticket clerk*/
	"SIGNAL_ENGAGED_WITH_TC\0", /*for cust*/
	"SIGNAL_ENGAGED_WITH_CC\0", /*for cust*/
	"WAIT_ENGAGED_WITH_CUST\0", /*for ticket clerk*/
	"WAIT_ENGAGED_WITH_TC\0", /*for cust*/
	"WAIT_ENGAGED_WITH_CC\0", /*for cust*/
	"SIGNAL", /*for cust*/
	"SIGNAL_BY_TC\0", /*for cust*/
	"SIGNAL_BY_CC\0", /*for cust*/
	"SIGNAL_BY_CUST\0", /*for ticket clerk*/
	"STARTED_BY_MAN\0", /*for employee(ticket clerk)*/
	"STARTED_BY_MAIN\0", /*for cust(ticket clerk)*/
	"GOT_TICKET\0", /*for group and cust*/
	"GOT_FOOD\0", /*for group and cust*/
	"LOOKING_FOR_QUEUE\0", /* for customer*/
	"LOOKING_FOR_TC_QUEUE\0", /* for customer*/
	"LOOKING_FOR_CC_QUEUE\0", /* for customer*/
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
	"IN_FOOD_QUEUE\0", /*for customer*/
	"SELLING_FOOD\0", /*for employee(concession clerk)*/
	"WAIT_IN_LOBBY_MOVIE_NOT_STARTED\0", /*for customer*/
	"WAIT_IN_LOBBY_AS_SOME_ONE_BATHROOM\0", /*for customer*/
	"AT_BATHROOM\0", /*for customer*/
	"AT_MOVIE_ROOM\0", /*for customer*/
	"AT_MOVIE_ROOM_SEAT\0", /*for customer*/
	"ON_BREAK\0", /*for employee*/
	"OFF_BREAK\0", /*for employee*/
	"WITH_TICKET_CLERK\0", /*for tickets*/
	"WITH_CUSTOMER\0", /*for tickets*/
	"WITH_TICKET_TAKER\0", /*for tickets*/
	"AVAILABLE_TO_BE_SOLD_OUT\0", /*for tickets*/
	"OUT_OF_THEATURE\0", /*for customer, customer_group*/
	"NO_CUSTOMER_NO_ADDRESS\0", /*for queue*/
	"NO_CUSTOMER\0", /*for queue*/
	"NO_ADDRESS\0", /*for queue*/
	"ROK\0", /*for queue*/
	"NOK\0", /*for queue*/
	"INVALID_STATE\0"
};

/* 
 * These are the messages strings 
 * for all message for logging prints.
 * */
char strMsg[][20]=
{
	"INITIAL_MSG\0", /*for customer*/
	"CUST_REMOVED\0", /*for customer*/
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
	"SELL_TICKETS\0", /*for employee(ticket clerk)*/
	"SELL_FOOD\0", /*for employee(ticket clerk)*/
	"INVALID_MSG\0"
};

/*
 * Various Messages
 *
 * */
#define	INITIAL_MSG 0, /*for customer*/
#define	CUST_REMOVED 1 /*for customer*/
#define	GO_ON_BREAK 2/*for the clerks*/
#define	GO_OFF_BREAK 3 /*for the clerks*/
#define	YES 4 /*for ticket taker and customer interaction*/
#define	NO 5/*for ticket taker and customer interaction*/
#define	FILL_CUST 6/*for ticket taker and manager interaction*/
#define	START_MOVIE 7/*for movie technician*/
#define	MOVIE_RUNNING 8/*for movie technician and customers*/
#define	MOVIE_NOT_RUNNING 9/*for movie technician*/
#define	MOVIE_OVER 10/*for movie technician and customers*/
#define	STARTING_MOVIE 11/*for movie technician*/
#define	SELL_TICKETS 12 /*for employee(ticket clerk)*/
#define	SELL_FOOD 13 /*for employee(ticket clerk)*/
#define	INVALID_MSG 14/*to nullify a received msg, ensuring that 1 msg is received only once*/
/*to keep track of where the customer and manager are  currently */
#define	START 0
#define	CONCESSION_CLERK 1
#define	TICKET_CLERK 2
#define	LOBBY 3
#define	BATHROOM 4
#define	OUT_OF_MOVIEROOM 5
#define	OUT_OF_MOVIETHEATURE 6
#define	MOVIEROOM 7

/*type of possible queues*/
#define	TC_QUEUE 0
#define	CC_QUEUE 1
#define	TT_QUEUE 2
#define	MAX_QUEUE_TYPE 3
#define	INVALID_QUEUE 4

typedef struct _mtQueue
{/* Movie theater queue - a general*/
	int queueType;
	char name[MAX_NAME_LENGTH];
        /*List *queue;*/
        int  queueId;
	MV numCust;
	Employee queueAddress;
	MV state;
}mtQueue;

typedef struct _Manager
{
	/*Thread *selfThread; // a pointer to manager's own thread*/
	char name[MAX_NAME_LENGTH];
	int location;/*where the manageris right now*/
	MV  state;/*state of manager*/
	MV msgToAll;/*transmit message to all entitites*/
	unsigned int money;/*money collected*/
}Manager;


typedef struct _Ticket
{
	int roomNum;/*room number of movie*/
}Ticket;

typedef struct _Seat
{
	int roomNum;/*room number of where seat is located*/
	CUST cust;/*customer who occupies this seat*/
	MV custId;/*customer id who occupies this seat*/
	MV custGrpId;/*customer grp id who occupies this seat*/
	MV isTaken;/*whether somebody is sat now*/
}Seat;

typedef struct _MvRoom
{
	Seat seat[MAX_ROWS][MAX_COLS];/*total number of seats in a movie room*/
}MvRoom;

typedef struct _Cust
{
	int selfId;/*the customer's number in the list*/
	int grpId;/*the grp to which customer belongs*/
	char name[MAX_NAME_LENGTH];
        /*Thread *selfThread;//a pointer to that customer itself*/
	MV location;/*current location of customer*/
	MV  state;/*state of customer*/
	unsigned int money;/*money paid*/
	Seat *seat;/*the customer's seat*/
	MV seatI;/*the customer's seat row*/
	MV seatJ;/*the customer's seat column*/
	MV seatTaken;/*has taken seat or not*/
	int takeSoda;/*will take Soda or not*/
	int IAMTicketBuyer;/*the head customer who interacts with clerk*/
	int takePopcorn;/* will take popcorn or not*/
	int goBathroom;/* will use bathroom or not*/
	MV msgBuffer;/*to receive messages*/
	mtQueue *queue;/*to keep track of which all queues customer gets in*/
}Cust;

typedef struct _Food
{
/*to show buying of food*/
}Food;

typedef struct _CustGrp
{/*to manage customer group*/
	Cust cust[MAX_CUST];
	char name[MAX_NAME_LENGTH];
        int grpId;
	MV location;
	MV  state;
	Cust *tBuyer;
	Ticket ticket[MAX_CUST];
	Food food;
        int numCust;
}CustGrp;

typedef struct _TC
{/*ticket clerk*/
	Manager *man;/*a thread to manager*/
	MV money;/*money made*/
	char name[MAX_NAME_LENGTH];
        /*Thread *selfThread;// a thread to self*/
	int queueType;/*type of queue for clerk*/
	mtQueue *queue;/*the actual queue*/
	Cust *currentCust;/*pointer to current dealing customer*/
	MV currentCustId;/*pointer to current dealing customer*/
	MV currentCustGrpId;/*pointer to current dealing customer group*/
	MV msgBuffer;/*to interact with customer*/
	MV msgByMan;/*message from Manager*/
	MV msgFromCust;/*message from customer*/
	int location;/*place where he is*/
	MV  state;/*state he is in*/
}TC;

typedef struct _CC
{/*concession clerk*/
	Manager *man;/*a thread to manager*/
	char name[MAX_NAME_LENGTH];
	MV money;/*money made*/
        /*Thread *selfThread;//a thread to manager*/
	MV msgByMan;/*message from Manager*/
	MV msgBuffer;/*to interact with customer*/
	MV msgFromCust;/*message from customer*/
	Cust *currentCust;/*pointer to current dealing customer*/
	MV currentCustId;/*pointer to current dealing customer*/
	MV currentCustGrpId;/*pointer to current dealing customer group*/
	int queueType;/*type of queue for clerk*/
	mtQueue *queue;/*the actual queue*/
	int location;/*place where he is*/
	MV  state;/*state he is in*/
}CC;

typedef struct _TT
{/*ticket taker*/
	Cust *currentCust;/*pointer to current dealing customer*/
	MV currentCustId;/*pointer to current dealing customer*/
	MV currentCustGrpId;/*pointer to current dealing customer group*/
	char name[MAX_NAME_LENGTH];
	Manager *man;/*a thread to manager*/
        /*Thread *selfThread;// a thread to self*/
	MV msgByMan;/*message from Manager*/
	MV msgToMan;/* send message to manager*/
	MV msgToCust;/*notify customer*/
	MV msgFromCust;/*interact with customer*/
	int queueType;/*type of queue for clerk*/
	mtQueue *queue;/*the actual queue*/
	int location;/*place where he is*/
	MV  state;/*state he is in*/
}TT;

typedef  struct _MovieTechnician
{/*movie technician*/
	Manager *man;/* pointer to manager thread*/
	char name[MAX_NAME_LENGTH];
	/*Thread *selfThread;//a thread to itself*/
	MV  state;/*state he is in*/
	MV msgByMan;/*message from Manager*/
	MV msgToMan;/* send message to manager*/
	int location;/*place where he is*/
}MT;

typedef struct _MtCb/*Movie theater control block*/
{/*The main control block of theater - all entities are member of this*/
	int numTC;/*number of ticket clerks*/
	int numCC;/*number of concession clerks*/
	int numTT;/*number of ticket takers*/
	TC tc[MAX_TC];/*setting up pointers to ticket clerks*/
	CC cc[MAX_CC];/*setting up pointers to concession clerks*/
	TT tt[MAX_TT];/*setting up pointers to ticket takers*/
	MT mt;/*pointer to theater*/
	MvRoom mvRoom;/*pointer to movie room*/
	int numCustGrp;/*number of customer groups*/
	MV numOfTicketsTaken;/*number of tickets sold in all*/
	CustGrp custGrp[MAX_CUST_GRP];/*setting up pointers to customer group*/
	Manager man; /*setting up pointer to manager*/
	mtQueue queue[MAX_QUEUE_TYPE][MAX_NUM_QUEUE];/*setting up pointers to all different queues*/
	unsigned int numCust;/*total number of customers*/
	char ticketTaking_name[MAX_NAME_LENGTH];
	char seats_name[MAX_NAME_LENGTH];
}MtCb;
/*__MOVIETHEATURE_MAIN_C__*/
