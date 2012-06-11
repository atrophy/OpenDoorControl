//FSM
//a generic event driven state machine designed for the Artifactory Door among other things.


//there should only be one FSM struct.
#define FSM_NUM_STATES 5
#define FSM_NUM_INPUTS 8

typedef void (* FPtr) ();


//can we assume these will be enumerated in order and from zero?
enum STATE {CLOSED, LOCKUP, OPEN, GUEST, RESTRICTED};	//states
/*
#define CLOSED 0
#define LOCKUP 1
#define OPEN 2
#define GUEST 3
#define RESTRICTED 4
*/

//can we assume these will be enumerated in order and from zero?
enum INPUT {DOORBELL, RED_BUTTON, GREEN_BUTTON, LOCKUP_TIMER, RFID_AUTH, RFID_NAUTH, RFID_RAUTH, RFID_RNAUTH};	//inputs//
/*
#define DOORBELL 0
#define RED_BUTTON 1
#define GREEN_BUTTON 2
#define LOCKUP_TIMER 3
#define RFID_AUTH 4
#define RFID_NAUTH 5
#define RFID_RAUTH 6
#define RFID_RNAUTH 7
*/

struct FSM{
	STATE state;
	const STATE trans[FSM_NUM_INPUTS][FSM_NUM_STATES];	//the index of the state to transition to on a given input event.
//int state;
//const int trans[FSM_NUM_INPUTS][FSM_NUM_STATES];

	const FPtr loop[FSM_NUM_STATES];	//the functions to execute continuously for a given state.
	const FPtr event[FSM_NUM_INPUTS][FSM_NUM_STATES];	//should only do one thing on any given transition. (a lot of functions)
};

void fsmtrans(INPUT input);	//called by an event handler

/******************functions within the FSM (user defined)*******************/
void closedloop();
void lockuploop();
void openloop();
void guestloop();
void restrictloop();

void noEvent();	//deliberately no event
void someEvent();	//arbitrary place holder

FSM fsm = {CLOSED,	//starting state

			/*state table			CLOSED,			LOCKUP,			OPEN,				GUEST,			RESTRICTED*/
			/*DOORBELL*/	{{	CLOSED,			OPEN,				OPEN,				OPEN, 			RESTRICTED},
			/*RED_BUTTON*/	{	CLOSED,			CLOSED, 		LOCKUP,			LOCKUP,			LOCKUP},
			/*GREEN_BUTTON*/{	CLOSED,			OPEN, 			GUEST,			OPEN,				RESTRICTED},
			/*LOCKUP_TIMER*/{	CLOSED, 		CLOSED,			OPEN,				GUEST,			RESTRICTED},
			/*RFID_AUTH*/		{	OPEN,				OPEN,				OPEN,				OPEN,				OPEN},
			/*RFID_NAUTH*/	{	CLOSED,			LOCKUP,			OPEN,				GUEST,			RESTRICTED},
			/*RFID_RAUTH*/	{	RESTRICTED,	RESTRICTED,	OPEN,				GUEST, 			RESTRICTED},
			/*RFID_RNAUTH*/	{	CLOSED,			LOCKUP,			OPEN,				GUEST,			RESTRICTED}},
			
			/*main-loops*/	{	closedloop, lockuploop, openloop, 	guestloop, restrictloop},
			
			/*state change events*/
			/*DOORBELL*/	{{	someEvent,	someEvent,	someEvent,	someEvent,	someEvent},
			/*RED_BUTTON*/	{	noEvent,		someEvent,	someEvent,	someEvent,	someEvent},
			/*GREEN_BUTTON*/{	noEvent,		someEvent,	someEvent,	someEvent,	someEvent},
			/*LOCKUP_TIMER*/{	noEvent,		someEvent,	noEvent,		noEvent,		noEvent},
			/*RFID_AUTH*/		{	someEvent,	someEvent,	someEvent,	someEvent,	someEvent},
			/*RFID_NAUTH*/	{	someEvent,	noEvent,		noEvent,		noEvent,		noEvent},			//logging is handled by RFID read func
			/*RFID_RAUTH*/	{	someEvent,	someEvent,	someEvent,	someEvent,	someEvent},
			/*RFID_RNAUTH*/	{	someEvent,	noEvent,		noEvent,		noEvent,		someEvent}}
};




