
#include "FSM.h"

void fsmtrans(INPUT input){	//often occurs inside interrupt
	fsm.event[fsm.state][input]();	//before change of state so the trans table still has meaning.
	fsm.state = fsm.trans[fsm.state][input];
}

void noEvent(){}	//deliberately no event
void someEvent(){};	//arbitrary place holder

void closedloop(){}
void lockuploop(){}
void openloop(){}
void guestloop(){}
void restrictloop(){}

int main(){
	
	while(1){
		fsm.loop[fsm.state]();
	}
	return 0;
}
