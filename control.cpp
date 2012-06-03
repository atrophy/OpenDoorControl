/*
  $Id$

  OpenDoorControl
  Copyright (c) 2012 The Perth Artifactory by
    Brett R. Downing <brett@artifactory.org.au>
    Daniel Harmsworth <atrophy@artifactory.org.au>
    Sebastian Southen <southen@gmail.com>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "OpenDoorControl.h"
#include "SDfiles.h"
#include "control.h"
#include "aux.h"
#include "Ethernet/Ethernet.h"

void pollTimers() {  //see setup for the default values
//  oldMicros = microsHolder;
//  microsHolder = micros();
  for (int i = 0; i < NUMFASTTIMERS; i++) {
    while ((fastTimers[i].active) && ((micros() - fastTimers[i].start) > fastTimers[i].period)) {
      fastTimers[i].start += fastTimers[i].period;
      fastTimers[i].expire();
    }
  }
}

void theTimeIncrement() {
  theTime++;
}

void pollSlowTimers() {
  for (int i = 0; i < NUMSLOWTIMERS; i++){
    int ohShit = 0;
    while ((slowTimers[i].active) && ((theTime - slowTimers[i].start) > slowTimers[i].period)) {
      slowTimers[i].start += slowTimers[i].period;
      slowTimers[i].expire();
      if ( ohShit >= 100 ) {
          fileWrite(logFile, "Runaway Loop on timer ", &"123456"[i], true);
      }
      ohShit++;
    }
  }
}

void dumpLogs() {
//  if(dumpFile(logFile)){  //used to send the file to the server, now just opens a new one.
  char oldLogName[BUFSIZ];
  //stringCopy(logFile, oldLogName);
	memcpy(&oldLogName, logFile, BUFSIZ);
  fileWrite(logFile, "File Dumped: ",logFile, true);
  nextFileName(logFile, logFilePrefix, logFileSuffix);
  fileWrite(logFile, "Dumped Log: ", oldLogName, true);
  //}
}

void openSpace() {
  fileWrite(logFile, "Opening Space", "", true);
  // TODO: tell the server the space is open
  spaceOpen = true;
  spaceGrace = false;
}

void closeSpace() {
  fileWrite(logFile, "Space closing, grace period started", "", true);
  slowTimers[TIMEREXITGRACE].start = theTime;
  slowTimers[TIMEREXITGRACE].active = true;

  fadePins[0] = STATUS_R;
  fadePins[1] = LOCKUPLED;

  fastTimers[TIMERLEDFADER].start = micros();
  fastTimers[TIMERLEDFADER].active = true;

  spaceOpen = false;
  guestAccess = false;
  spaceGrace = true;
}

void closeSpaceFinal() {
  fileWrite(logFile, "Closing Space", "", true);
  slowTimers[TIMEREXITGRACE].active = false;
  fastTimers[TIMERLEDFADER].active = false;

  clearFade();

  // TODO: tell the server the space is closed
  DoorStatus(1, 0, 1);
  spaceOpen = false;
  guestAccess = false;
  spaceGrace = false;
}

//void induceDeath() {
//  unsigned long diffTime = theTime;
//  fileWrite(logFile, "Manually Shifting: ", "+20", true);
//  theTime += 20;
//  fileWrite(logFile, "Time Adjusted", "", true);
//  diffTime = theTime - diffTime;
//  for(int i = 0; i < NUMSLOWTIMERS; i++){
//    slowTimers[i].start +=diffTime;
//  }

 // fileWrite(logFile, "Timers Updated", "", true);
  //fetchTime();
//}



void openTheDoor() {
  // Trigger the door strike
  fileWrite(logFile, "Unlocking Door", "", true);
  digitalWrite(DOORSTRIKE, HIGH);
  fastTimers[TIMERSTRIKE].start = micros();
  fastTimers[TIMERSTRIKE].active = true;
  //DoorStatus(0, 1, 0);
  digitalWrite(STATUS_R, LOW);
  digitalWrite(STATUS_G, HIGH);
  digitalWrite(STATUS_B, LOW);
}

void closeTheDoor() {
  fileWrite(logFile, "Locking Door", "", true);
  digitalWrite(DOORSTRIKE, LOW);
  fastTimers[TIMERSTRIKE].active = false;
  DoorStatusRefresh();
}

void LCDrefresh() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Artifactory Door");

  // There are some states that require ongoing display of data.

  // If the space is in the lockup grace period, show the countdown
  if (spaceGrace) {
    lcd.setCursor(0, 1);
    char output[17];
    int remainingTime = slowTimers[TIMEREXITGRACE].period - ( theTime - slowTimers[TIMEREXITGRACE].start );
    sprintf(output, "Lock in %d Sec\0", remainingTime);
    lcd.print(output);
    return;
  }

  // By default just display the time (unless the RTC is borked)
  if (RTC.isrunning()) {
    lcdDisplayTime(1);
  }
  else {
    lcd.setCursor(0, 1);
    lcd.print("WARN:RTC Offline\0");
  } 
}

void DoorBell() {
	digitalWrite(DOORBELLLED, HIGH);
	slowTimers[TIMERDOORBELL].start = theTime;
	slowTimers[TIMERDOORBELL].active = true;
}

void DoorBellRefresh() {
	digitalWrite(DOORBELLLED, LOW);
	slowTimers[TIMERDOORBELL].active = false;
}

void DoorStatus(bool r, bool g, bool b) {
	digitalWrite(STATUS_R, (r ? HIGH : LOW));
	digitalWrite(STATUS_G, (g ? HIGH : LOW));
	digitalWrite(STATUS_B, (b ? HIGH : LOW));
	slowTimers[TIMERDOORSTATUS].start = theTime;
	slowTimers[TIMERDOORSTATUS].active = true;
}

void DoorStatusRefresh() {
	digitalWrite(STATUS_R, LOW);
	digitalWrite(STATUS_G, LOW);
	digitalWrite(STATUS_B, LOW);
	slowTimers[TIMERDOORSTATUS].active = false;
}

void ledBlink(){
  if ( blinkPin <= 0) { return; }
  digitalWrite(blinkPin, ( blinkStatus ? HIGH : LOW ));
  blinkStatus = !blinkStatus;
}

void ledFade(){
  int f = 0;
  int fadePin = 0;
  float faderSeed = millis()/fadeTime;
  int fVal = 127 + (127 * sin( faderSeed * 2.0 * 3.14159 ));

  while (f < LEDFADERCOUNT) {
    fadePin = fadePins[f];
    if ( fadePin > 0) {
      analogWrite(fadePin, fVal);
    }
    f++;
  }
}

void clearFade() {
  int f = 0;
  while (f < LEDFADERCOUNT) {
    fadePins[f] = 0;
    f++;
  }
}

// Here be dragons (and networking code)

// DH:  My current rule of thumb is to turn the external status LED yellow to indicate
//      network activity, should help prevent confusion if the door won't scan for a 
//      second or two during a network operation (and confuse the hell out of the neighbours).

// This runs regularly to make sure the DHCP lease is up to date (should probably run every 15 mins or so)
void DHCPRefresh() {
  int ethStatus = 0;

  // Turn the status LED yellow whilst we're renewing the lease, should help with debugging...
  digitalWrite(STATUS_R, HIGH);
  digitalWrite(STATUS_G, HIGH);

  // Try to renew the lease
  ethStatus = Ethernet.maintain();

  // Now that the refresh has (hopefully) succeeded turn the LED's off.
  digitalWrite(STATUS_R, LOW);
  digitalWrite(STATUS_G, LOW);

  if (ethStatus == 1) // Oh shiiiii........
  {
    fileWrite(logFile, "Failed to renew DHCP lease.", "", true);
  }
}

void UpdateAuthLists() {
  authFileCurrent = 0;
  slowTimers[TIMERUPDATEPOLLER].active = true;
}

// Ticks through the files that need updating
// Called by the TIMERUPDATEPOLLER timer.
void UpdatePoller() {
  if ( authFileCurrent < AUTHFILECOUNT )
  {
    if (client.connect(server, 80)) {
      client.print("GET /");
      client.println(authFiles[authFileCurrent]);
      client.println();
      authRetrieveAttempts = 0;
      slowTimers[TIMERUPDATERECEIVE].active = true;
    }
    else {
      fileWrite(logFile, "Update server connection failure", "", true);
    }
    authFileCurrent++; 
  }
  slowTimers[TIMERUPDATEPOLLER].active = false;
}

// Receives data waiting on the HTTP client
// This is called by the TIMERUPDATERECEIVE timer
void ReceiveUpdateRequest()
{
  char line[HASHLENGTH] = {0};
  int lPos = 0;
  if (authRetrieveAttempts < 10){
    if (client.connected()){
      slowTimers[TIMERUPDATERECEIVE].active = false;
      while (client.available()) {
        char c = client.read();
        if (c == '\n') {
          fileWrite(tempFile, line, "", false);
          lPos = 0;
        }
        else {
          line[lPos] = c;
          lPos++;
        }
      }
      client.stop();
      slowTimers[TIMERUPDATEPOLLER].active = true;

      // At this juncture we should have a new file in tempFile that should be verified somehow...

      
    }
  }
  else {
    slowTimers[TIMERUPDATERECEIVE].active = false;
    client.stop();
    fileWrite(logFile, "Update receive timout", "", true);
  }
}