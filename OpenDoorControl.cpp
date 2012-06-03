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

/*
a sketch for the door of the arifactory.
by Brett Downing
using examples from arduino 1.0 and assorted other places.

SD card, sha1, PCINTs, rfid (ID12), latch driver, buttons.

to do:
test

lessons learned:
  SDfatlib can't:
    rename files,
    use file names longer than 8.3,
    hold multiple files open
  don't send serial strings from inside interrupt. EVER!
    
*/
#include "OpenDoorControl.h"
#include <SPI.h>
#include <SD.h>
//#include "Sha/sha1.h"
#include "Ethernet/Ethernet.h"
#include <Wire.h>
#include "aux.h"
#include "SDfiles.h"
#include "control.h"
#include "RFID.h"
#include "interrupts.h"

#include <avr/wdt.h>

LiquidCrystal lcd(32, 30, 28, 26, 24, 22);
RTC_DS1307 RTC;  //using hardware i2c 18, 19

bool SDcardPresent = false;     // true if sd card is accessible and has at least a full members file.
bool spaceOpen = false;         // allows associate members
bool guestAccess = false;       // links doorbell to doorstrike
bool reedswitchState = false;   // TODO
bool onNTPtime = false;         // true after the first NTP success.
bool spaceGrace = false;        // Space is in post-lockup grace period

timer fastTimers[NUMFASTTIMERS];
timer slowTimers[NUMSLOWTIMERS];

//the time for logging purposes *******************************
unsigned long theTime = 0;	//seconds since boot (or 1900 epoch after NTP)

// ---------------------------------------------------
// Network Variables
// ---------------------------------------------------
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};  // MAC address for the controller
byte server[] = { 192, 168, 16, 1 };                // IP of the server running the auth updater
EthernetClient client;

// ---------------------------------------------------
// SD Card & File Settings
// ---------------------------------------------------
File openFile;                        // The open file (the SD lib can't hold two files open?!)
char fullFile[13]  = "fullHash.txt";  // The full members file
char assocFile[13] = "asocHash.txt";  // The associate members file
char restFile[13]  = "restHash.txt";  // The restricted members file
const char* logFilePrefix = "log";    // Prefix for the log files
const char* logFileSuffix = ".txt";   // Suffix for the log files
char logFile[13];                     // Log file currently in use

char tempFile[13] = "tempHash.txt";

char authFiles[AUTHFILECOUNT][13] = { "fullHash.txt", "asocHash.txt", "restHash.txt" };
int authFileCurrent = 0;
int authRetrieveAttempts = 0;

// ---------------------------------------------------
// Blinkenlights Settings
// ---------------------------------------------------
bool blinkStatus = false;         // Is the blinker currently blinking
int blinkPin = 0;                 // What pin should the blinker blink
float fadeTime = 2000.0;          // How long should the fader fade for
int fadePins[LEDFADERCOUNT] = {}; // Pins to fade


void setup() {
  // Pat the watchdog
  wdt_reset();

  // Set the watchdog timer to 8 seconds
  wdt_enable(WDTO_8S);

  // start the serial library:
  Serial.begin(9600);	//debug messages
  Serial1.begin(9600);	//RFID
  Serial.println("Artifactory Door\r\n  Booting");

  auxSetup();
  fetchTime();
  lcdDisplayTime(1);
  
  if (SD.begin(SD_CS_PIN)) {
/*
    if(SD.exists(logFile)){  //if there is an active log file 
      archiveLog();  
    }
*/
    nextFileName(logFile, logFilePrefix, logFileSuffix);
    fileWrite(logFile, "booting", "", true);
    Serial.print("logFile = ");
    Serial.println(logFile);

    openFile = SD.open(fullFile);  //can we open the full members file?
    if (openFile) {
		fileWrite(logFile, "full members file open success: ", fullFile, true);      
		SDcardPresent = true;
    }
    openFile.close();

	openFile = SD.open(restFile);
	if (openFile) {
		fileWrite(logFile, "restricted members file open success: ", restFile, true); 
	}
    openFile.close();

    openFile = SD.open(assocFile);  //can we open the full members file?
    if (openFile) {
		fileWrite(logFile, "associate members file open success: ", assocFile, true);      
    }
    openFile.close();
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Artifactory Door");
  lcd.setCursor(0, 1);
  lcd.print("DHCP Started");

  if (Ethernet.begin(mac, 10000) == 0) {
    fileWrite(logFile, "DHCP failed.", "", true);
  }
  else {
    byte localAddress[4];
    localAddress[0] = Ethernet.localIP()[0];
    localAddress[1] = Ethernet.localIP()[1];
    localAddress[2] = Ethernet.localIP()[2];
    localAddress[3] = Ethernet.localIP()[3];

    char addressPrintable[15] = { localAddress[0], localAddress[1], localAddress[2], localAddress[3] };

    fileWrite(logFile, "Network connected with address: ", addressPrintable, true);
  }
  
  //fast timers are measured against micros()
  fastTimers[TIMERSECOND].period = 1 S;
  fastTimers[TIMERSECOND].start = micros();
  fastTimers[TIMERSECOND].active = true;
  fastTimers[TIMERSECOND].expire = theTimeIncrement;

  fastTimers[TIMERSLOWPOLL].period = 2 S;
  fastTimers[TIMERSLOWPOLL].start = micros();
  fastTimers[TIMERSLOWPOLL].active = true;
  fastTimers[TIMERSLOWPOLL].expire = pollSlowTimers;

  fastTimers[TIMERSTRIKE].period = 5 S;
  fastTimers[TIMERSTRIKE].active = false;
  fastTimers[TIMERSTRIKE].expire = closeTheDoor;

  fastTimers[TIMERLEDBLINK].period = 1 S; // Seconds
  fastTimers[TIMERLEDBLINK].active = false;
  fastTimers[TIMERLEDBLINK].expire = ledBlink;

  fastTimers[TIMERLEDFADER].period = 10 MS;
  fastTimers[TIMERLEDFADER].active = false;
  fastTimers[TIMERLEDFADER].expire = ledFade;


  // Slow timers are measured against theTime, their period is in seconds

  slowTimers[TIMERLOGDUMP].period = 86400;  // 24 hours
  slowTimers[TIMERLOGDUMP].start = theTime;
  slowTimers[TIMERLOGDUMP].active = true;
  slowTimers[TIMERLOGDUMP].expire = dumpLogs;

  slowTimers[TIMERRTCREFRESH].period = 86400;  // 24 hours
  slowTimers[TIMERRTCREFRESH].start = theTime;
  slowTimers[TIMERRTCREFRESH].active = true;
  slowTimers[TIMERRTCREFRESH].expire = fetchTime;

  slowTimers[TIMERLCDTIME].period = 2;  	// seconds
  slowTimers[TIMERLCDTIME].start = theTime;
  slowTimers[TIMERLCDTIME].active = true;
  slowTimers[TIMERLCDTIME].expire = LCDrefresh;

	slowTimers[TIMERDOORBELL].period = 2;	// seconds
	slowTimers[TIMERDOORBELL].active = false;
	slowTimers[TIMERDOORBELL].expire = DoorBellRefresh;

	slowTimers[TIMERDOORSTATUS].period = 2;	// seconds
	slowTimers[TIMERDOORSTATUS].active = false;
	slowTimers[TIMERDOORSTATUS].expire = DoorStatusRefresh;

  slowTimers[TIMEREXITGRACE].period = 60; // Give a 1 minute grace period post lockup
  slowTimers[TIMEREXITGRACE].active = false;
  slowTimers[TIMEREXITGRACE].expire = closeSpaceFinal;

  slowTimers[TIMERDHCPREFRESH].period = 900; // DHCP refresh every 15 mins
  slowTimers[TIMERDHCPREFRESH].active = true;
  slowTimers[TIMERDHCPREFRESH].expire = DHCPRefresh;

  slowTimers[TIMERUPDATELISTS].period = 43200; // 12 hours
  slowTimers[TIMERUPDATELISTS].active = true;
  slowTimers[TIMERUPDATELISTS].expire = UpdateAuthLists;

  slowTimers[TIMERUPDATEPOLLER].period = 5; // seconds
  slowTimers[TIMERUPDATEPOLLER].active = false;
  slowTimers[TIMERUPDATEPOLLER].expire = UpdatePoller;

  slowTimers[TIMERUPDATERECEIVE].period = 5; // seconds
  slowTimers[TIMERUPDATERECEIVE].active = false;
  slowTimers[TIMERUPDATERECEIVE].expire = ReceiveUpdateRequest;

  //slowTimers[TIMERINDUCEDEATH].period = 10;
  //slowTimers[TIMERINDUCEDEATH].active = true;
  //slowTimers[TIMERINDUCEDEATH].expire = induceDeath;

  PCICR |= (1 << PCIE2);  //enable port-change interrupt on port-change-byte 2
  PCMSK2 |= DOORBELLBIT;
  PCMSK2 |= REEDSWITCHBIT;
  PCMSK2 |= GUESTOKBIT;
  PCMSK2 |= LOCKUPBIT;

  pinMode(DOORSTRIKE, OUTPUT);
  pinMode(LCDBACKLIGHT, OUTPUT);
  pinMode(DOORBELL, INPUT);
  pinMode(REEDSWITCH, INPUT);
  pinMode(GUESTOKSWITCH, INPUT);
  pinMode(LOCKUPBUTTON, INPUT);

  digitalWrite(LCDBACKLIGHT, HIGH);
  digitalWrite(DOORBELL, HIGH);  //pullup resistors
  digitalWrite(REEDSWITCH, HIGH);
  digitalWrite(GUESTOKSWITCH, HIGH);
  digitalWrite(LOCKUPBUTTON, HIGH);

//  interruptFlags = 0;


	pinMode(DOORBELLLED, OUTPUT);
	pinMode(GUESTOKLED, OUTPUT);
	pinMode(LOCKUPLED, OUTPUT);

	digitalWrite(DOORBELLLED, LOW);
	digitalWrite(GUESTOKLED, LOW);
	digitalWrite(LOCKUPLED, LOW);

	pinMode(STATUS_R, OUTPUT);
	pinMode(STATUS_G, OUTPUT);
	pinMode(STATUS_B, OUTPUT);
	digitalWrite(STATUS_R, LOW);
	digitalWrite(STATUS_G, LOW);
	digitalWrite(STATUS_B, LOW);

}

void loop() {
  pollRFIDbuffer(); //there is no indication or recovery if the RFID goes down.
  pollTimers();
  runInterruptServices();
  //  pollServerBuffer();
  wdt_reset();  // Pat the watchdog
}

