/**
 *	@file
 *	@author	Daniel Harmsworth <atrophy@artifactory.org.au>
 *	@author	Brett Downing <brett@artifactory.org.au>
 *	@version 2.0
 *
 *	@section LICENSE
 *	
 *	This file is part of OpenDoorControl.
 *
 *	OpenDoorControl is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	OpenDoorControl is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OpenDoorControl.  If not, see <http://www.gnu.org/licenses/>.
 *
 *	@section DESCRIPTION
 *
 *	This file contains the setup and definitions for the finite state machine running the door.
 *	
 *
**/

// FSM
// a generic event driven finite state machine designed for the Artifactory Door among other things.

/*
     _  _(o)_(o)_  _
   ._\`:_ F S M _:' \_,
       / (`---'\ `-.
    ,-`  _)    (_, 
*/

//there should only be one FSM struct.
#define FSM_NUM_STATES 5
#define FSM_NUM_INPUTS 8

typedef void (* FPtr) ();


//can we assume these will be enumerated in order and from zero?
//enum STATE {CLOSED, LOCKUP, OPEN, GUEST, RESTRICTED};	//states

#define CLOSED 0
#define LOCKUP 1
#define OPEN 2
#define GUEST 3
#define RESTRICTED 4


//can we assume these will be enumerated in order and from zero?
//enum INPUT {DOORBELL, RED_BUTTON, GREEN_BUTTON, LOCKUP_TIMER, RFID_AUTH, RFID_NAUTH, RFID_RAUTH, RFID_RNAUTH};	//inputs//

#define DOORBELL 0
#define RED_BUTTON 1
#define GREEN_BUTTON 2
#define LOCKUP_TIMER 3
#define RFID_AUTH 4
#define RFID_NAUTH 5
#define RFID_RAUTH 6
#define RFID_RNAUTH 7

struct FSM{
//	STATE state;
//	const STATE trans[FSM_NUM_INPUTS][FSM_NUM_STATES];	//the index of the state to transition to on a given input event.

	int state;
	const int trans[FSM_NUM_INPUTS][FSM_NUM_STATES];

	const FPtr loop[FSM_NUM_STATES];	//the functions to execute continuously for a given state.
	const FPtr event[FSM_NUM_INPUTS][FSM_NUM_STATES];	//should only do one thing on any given transition. (a lot of functions)
};

void fsmtrans(int input);	//called by an event handler

/******************functions within the FSM (user defined)*******************/
void closedLoop();
void lockupLoop();
void openLoop();
void guestLoop();
void restrictLoop();

void noEvent();	//deliberately no event
void someEvent();	//arbitrary place holder

FSM fsm = {CLOSED,	//starting state

			/*state table		CLOSED,			LOCKUP,			OPEN,			GUEST,			RESTRICTED		*/
			/*DOORBELL*/	{{	CLOSED,			OPEN,			OPEN,			OPEN, 			RESTRICTED		},
			/*RED_BUTTON*/	{	CLOSED,			CLOSED, 		LOCKUP,			LOCKUP,			LOCKUP			},
			/*GREEN_BUTTON*/{	CLOSED,			OPEN, 			GUEST,			OPEN,			RESTRICTED		},
			/*LOCKUP_TIMER*/{	CLOSED, 		CLOSED,			OPEN,			GUEST,			RESTRICTED		},
			/*RFID_AUTH*/	{	OPEN,			OPEN,			OPEN,			OPEN,			OPEN			},
			/*RFID_NAUTH*/	{	CLOSED,			LOCKUP,			OPEN,			GUEST,			RESTRICTED		},
			/*RFID_RAUTH*/	{	RESTRICTED,		RESTRICTED,		OPEN,			GUEST, 			RESTRICTED		},
			/*RFID_RNAUTH*/	{	CLOSED,			LOCKUP,			OPEN,			GUEST,			RESTRICTED		}},
			
			/*main-loops*/	{	closedLoop,		lockupLoop,		openLoop, 		guestLoop, 		restrictLoop	},
			
			/*state change events*/
			/*DOORBELL*/	{{	someEvent,		someEvent,		someEvent,		someEvent,		someEvent		},
			/*RED_BUTTON*/	{	noEvent,		someEvent,		someEvent,		someEvent,		someEvent		},
			/*GREEN_BUTTON*/{	noEvent,		someEvent,		someEvent,		someEvent,		someEvent		},
			/*LOCKUP_TIMER*/{	noEvent,		someEvent,		noEvent,		noEvent,		noEvent			},
			/*RFID_AUTH*/	{	someEvent,		someEvent,		someEvent,		someEvent,		someEvent		},
			/*RFID_NAUTH*/	{	someEvent,		noEvent,		noEvent,		noEvent,		noEvent			},
			/*RFID_RAUTH*/	{	someEvent,		someEvent,		someEvent,		someEvent,		someEvent		},
			/*RFID_RNAUTH*/	{	someEvent,		noEvent,		noEvent,		noEvent,		someEvent		}}
};




