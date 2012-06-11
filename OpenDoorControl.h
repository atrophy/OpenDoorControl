/**
 * @file
 * @author Daniel Harmsworth <atrophy@artifactory.org.au>
 * @author Brett Downing <brett@artifactory.org.au>
 * @version 2.0
 *
 * @section LICENSE
 *	
 * This file is part of OpenDoorControl.
 *
 * OpenDoorControl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenDoorControl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenDoorControl.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 *
 * This file contains the setup and definitions for the finite state machine running the door.
 *	
 *
**/

/*
FSM
A generic event driven finite state machine designed for the Artifactory Door among other things.

     _  _(o)_(o)_  _
   ._\`:_ F S M _:' \_,
       / (`---'\ `-.
    ,-`  _)    (_,

*/

typedef void (* FPtr) ();

/** 
 * The states that the system can be in. The system can only be in one state at
 * any given time.
**/
enum STATE {
				CLOSED,
				LOCKUP,
				OPEN,
				GUEST,
				RESTRICTED,
				FSM_NUM_STATES	// This always has to be the last member
};

/**
 * Inputs are how the outside world communicates with the system. The result of
 * an input is dependent on the current state.
**/
enum INPUT {
				DOORBELL,
				RED_BUTTON,
				GREEN_BUTTON,
				LOCKUP_TIMER,
				RFID_AUTH,
				RFID_NAUTH,
				RFID_RAUTH,
				RFID_RNAUTH,
				FSM_NUM_INPUTS
};

/**
 * @stuct
 * @brief Finite State Machine
 *
 * Finite state machine structure maintains the current state of the system, determining
 * what function is treated as the main loop and manages the transition between states.
 *
**/

struct FSM{
	/** The current state of the system. **/
	STATE state;

	/** Transition functions for moving between states. **/
	const STATE trans[FSM_NUM_INPUTS][FSM_NUM_STATES];

	/** The functions to execute continuously for a given state. **/
	const FPtr loop[FSM_NUM_STATES];

	/** Should only do one thing on any given transition. (a lot of functions) **/
	const FPtr event[FSM_NUM_INPUTS][FSM_NUM_STATES];
};

/**
* Called by an event handler
*
* @param input The input that triggered the event
**/
void fsmtrans(INPUT input);

/******************functions within the FSM (user defined)*******************/
void closedLoop();
void lockupLoop();
void openLoop();
void guestLoop();
void restrictLoop();

void noEvent();
void someEvent();


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
