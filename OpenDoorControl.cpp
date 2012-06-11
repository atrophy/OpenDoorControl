
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
 *	This is where our main loop lives and we do all of the initial setup.
 *
**/

#include <avr/wdt.h>
#include "OpenDoorControl.h"

void fsmtrans(int input){	//often occurs inside interrupt
	fsm.event[fsm.state][input]();	//before change of state so the trans table still has meaning.
	fsm.state = fsm.trans[fsm.state][input];
}

void noEvent(){}	//deliberately no event
void someEvent(){};	//arbitrary place holder

void closedLoop(){}
void lockupLoop(){}
void openLoop(){}
void guestLoop(){}
void restrictLoop(){}

int main(){

	// Watchdog timer setup
	wdt_enable(WDTO_8S);

	while(1){
		wdt_reset();			// Pat the watchdog
		fsm.loop[fsm.state]();	// Run the state machine loop
	}

	return 0;

}