//====================================================================================================
// Kurts Test program to try out different ways to manipulate the AX12 servos on the PhantomX
// This is a test, only a test...  
//====================================================================================================
//=============================================================================
// Global Include files
//=============================================================================
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <inttypes.h>
#include <signal.h>

#include "ArduinoDefs.h"
#include "msound.h"
#include "WrapperSerial.h"
#include "ax12.h"
#include "BioloidEX.h"

//=============================================================================
//=============================================================================

// Uncomment the next line if building for a Quad instead of a Hexapod.
//#define QUAD_MODE
#define TURRET

// Header files...

// Constants
/* Servo IDs */
#define     RF_COXA       2
#define     RF_FEMUR      4
#define     RF_TIBIA      6

#define     RM_COXA      14
#define     RM_FEMUR     16
#define     RM_TIBIA     18

#define     RR_COXA       8
#define     RR_FEMUR     10
#define     RR_TIBIA     12

#define     LF_COXA       1
#define     LF_FEMUR      3
#define     LF_TIBIA      5

#define     LM_COXA      13
#define     LM_FEMUR     15
#define     LM_TIBIA     17

#define     LR_COXA       7
#define     LR_FEMUR      9
#define     LR_TIBIA     11

#ifdef TURRET
#define     TURRET_ROT    20
#define     TURRET_TILT   21
#endif

static const byte pgm_axdIDs[] PROGMEM = {
  LF_COXA, LF_FEMUR, LF_TIBIA,    
#ifndef QUAD_MODE
  LM_COXA, LM_FEMUR, LM_TIBIA,    
#endif  
  LR_COXA, LR_FEMUR, LR_TIBIA,
  RF_COXA, RF_FEMUR, RF_TIBIA, 
#ifndef QUAD_MODE
  RM_COXA, RM_FEMUR, RM_TIBIA,    
#endif
  RR_COXA, RR_FEMUR, RR_TIBIA
#ifdef TURRET
  , TURRET_ROT, TURRET_TILT
#endif
};    

#define NUM_SERVOS (sizeof(pgm_axdIDs)/sizeof(pgm_axdIDs[0]))
const char* IKPinsNames[] = {
  "LFC","LFF","LFT",
#ifndef QUAD_MODE
  "LMC","LMF","LMT",
#endif  
  "LRC","LRF","LRT",
  "RFC","RFF","RFT",
#ifndef QUAD_MODE
  "RMC","RMF","RMT",
#endif  
  "RRC","RRF","RRT",
#ifdef TURRET
  "T-ROT", "T-TILT"
#endif
};

// Global objects
/* IK Engine */
WrapperSerial Serial = WrapperSerial();

// other globals.
word           g_wVoltage;
char           g_aszCmdLine[80];
uint8_t        g_iszCmdLine;
boolean        g_fTrackServos = false;

// Values to use for servo position...
byte          g_bServoID;
word          g_wServoGoalPos;
word          g_wServoGoalSpeed;

// forward references
extern void setup(void);
extern void loop(void);


extern void AllServosOff(void);
extern uint8_t GetCommandLine(void);
extern boolean FGetNextCmdNum(word *pw );
extern void AllServosCenter(void);
extern void HoldOrFreeServos(byte fHold);
extern void SetServoPosition(void) ;
extern void SetServoID(void);
extern void WaitForMoveToComplete(word wID);
extern void GetServoPositions(void);
extern void TrackServos(boolean fInit);
extern void TrackPrintMinsMaxs(void);
extern void PrintServoValues(void);


//====================================================================================================
// SignalHandler - Try to free up things like servos if we abort.
//====================================================================================================
void SignalHandler(int sig){
    printf("Caught signal %d\n", sig);

    // Stop motors if they are active
    AllServosOff();
    exit(1); 

}
      
//--------------------------------------------------------------------------
// Main: the main  function.
//--------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    // Install signal handler to allow us to do some cleanup...
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = SignalHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

    char abT[40];        // give a nice large buffer.
    uint8_t cbRead;

    printf("Start\n");
        if (argc > 1)
        {
           for (int i=1; i < argc; i++) 
            {
                    printf("%d - %s\n", i, argv[i]);
            }
        }
    char *pszDevice;    

	setup();
    
    for(;;) 
    {
    //--------------------------------------------------------------------------
    // Loop: the main arduino main Loop function
    //--------------------------------------------------------------------------
		loop();
    }
}


//====================================================================================================
// Setup 
//====================================================================================================
void setup() {
  Serial.begin();  // start off the serial port.  
  if(dxl_initialize(0, 1) == 0) {
	//printf("Failed to open USBDynamixel\n");
  }	

  delay(1000);
  Serial.print("System Voltage in 10ths: ");
  Serial.println(g_wVoltage = ax12GetRegister(1, AX_PRESENT_VOLTAGE, 1), DEC);
}


//====================================================================================================
// Loop
//====================================================================================================
void loop() {
  // Output a prompt
  word wNewVoltage = ax12GetRegister(1, AX_PRESENT_VOLTAGE, 1);
  if (wNewVoltage != g_wVoltage) {
    g_wVoltage = wNewVoltage;
    Serial.print("System Voltage in 10ths: ");
    Serial.println(g_wVoltage, DEC);
  }

  // lets toss any charcters that are in the input queue
  while(Serial.available() )
        Serial.read();

  Serial.println("0 - All Servos off");
  Serial.println("1 - All Servos center");
  Serial.println("2 - Set Servo position [<Servo>] <Position> [<Speed>]");
  Serial.println("3 - Set Servo Angle");
  Serial.println("4 - Get Servo Positions");
#if 0
  Serial.println("5 - Timed Move: <Servo> <From> <to> <speed> <cnt>");
  Serial.println("6 - Timed 2: <Servo> <speed>");
  Serial.println("7 - Timed 3: <Servo> <Dist> <Time>");
#endif
  Serial.println("8 - Set ID: <old> <new>");
  Serial.println("9 - Print Servo Values");
  Serial.println("t - Toggle track Servos");
  Serial.println("h - hold [<sn>]");
  Serial.println("f - free [<sn>]"); 
  Serial.print(":");
  Serial.flush();  // make sure the complete set of prompts has been output...  
  // Get a command
  if (GetCommandLine()) {
    Serial.println("");
    g_iszCmdLine = 1;  // skip over first byte...
    switch (g_aszCmdLine[0]) {
    case '0':
      AllServosOff();
      break;
    case '1':
      AllServosCenter();
      break;
    case '2':
      SetServoPosition();  
      break;
    case '3':
      break;
    case '4':
      GetServoPositions();
      break;
#if 0
    case '5':
      TimedMove();
      break;
    case '6':
      TimedMove2();
      break;
    case '7':
      TimedMove3();
      break;
#endif
    case '8':
      SetServoID();
      break;
    case '9':
      PrintServoValues();
      break;
    case 'f':
    case 'F':
      HoldOrFreeServos(0);
      break;
    case 'h':
    case 'H':
      HoldOrFreeServos(1);
      break;

    case 't':
    case 'T':
      g_fTrackServos = !g_fTrackServos;
      if (g_fTrackServos) {
        Serial.println("Tracking On");
        TrackServos(true);  // call to initialize all of the positions.
      }
      else
        Serial.println("Tracking Off");
      TrackPrintMinsMaxs();
      break;
    }
  }
}

// Helper function to read in a command line
uint8_t GetCommandLine(void) {
  int ch;
  uint8_t ich = 0;
  g_iszCmdLine = 0;

  for(;;) {
    // throw away any thing less than CR character...
    ch = Serial.read();
    if ((ch >= 10) && (ch <=15)) {
      g_aszCmdLine[ich] = 0;
      return ich;
    }    
    if (ch != -1) 
      g_aszCmdLine[ich++] = ch;

    if (g_fTrackServos)
      TrackServos(false);
  }
}

//
boolean FGetNextCmdNum(word *pw ) {
  // Skip all leading num number characters...
  while ((g_aszCmdLine[g_iszCmdLine] < '0') || (g_aszCmdLine[g_iszCmdLine] > '9')) {
    if (g_aszCmdLine[g_iszCmdLine] == 0)
      return false;  // end of the line...
    g_iszCmdLine++;  
  }
  *pw = 0;
  while ((g_aszCmdLine[g_iszCmdLine] >= '0') && (g_aszCmdLine[g_iszCmdLine] <= '9')) {
    *pw = *pw * 10 + (g_aszCmdLine[g_iszCmdLine] - '0');
    g_iszCmdLine++;
  }
  return true;
}

//=======================================================================================
void AllServosOff(void) {
  for (int i = 0; i < NUM_SERVOS; i++) {
    ax12SetRegister(pgm_read_byte(&pgm_axdIDs[i]), AX_TORQUE_ENABLE, 0x0);
  }
}
//=======================================================================================
void AllServosCenter(void) {
  for (int i = 0; i < NUM_SERVOS; i++) {
    // See if this turns the motor off and I can turn it back on...
    ax12SetRegister(pgm_read_byte(&pgm_axdIDs[i]), AX_TORQUE_ENABLE, 0x1);
//    ax12ReadPacket(6);  // git the response...
    ax12SetRegister2(pgm_read_byte(&pgm_axdIDs[i]), AX_GOAL_POSITION_L, 0x1ff);
//    ax12ReadPacket(6);  // git the response...
  }
}
//=======================================================================================
void HoldOrFreeServos(byte fHold) {
  word iServo;

  if (!FGetNextCmdNum(&iServo)) {
    // All servos...
    for (int i = 0; i < NUM_SERVOS; i++) {
      ax12SetRegister(pgm_read_byte(&pgm_axdIDs[i]), AX_TORQUE_ENABLE, fHold);
//      ax12ReadPacket(6);  // git the response...
    }
  } 
  else {
    ax12SetRegister(iServo, AX_TORQUE_ENABLE, fHold);
//    ax12ReadPacket(6);  // git the response...
  }
}

//=======================================================================================

//=======================================================================================
void SetServoPosition(void) {
  word w1;
  word w2;

  if (!FGetNextCmdNum(&w1))
    return;    // no parameters so bail.

  Serial.println("Set Servo Position"); 
  if (FGetNextCmdNum(&w2)) {  // We have at least 2 parameters
    g_bServoID = w1;    // So first is which servo
    g_wServoGoalPos = w2;
    if (FGetNextCmdNum(&w2)) {  // We have at least 3 parameters
      g_wServoGoalSpeed = w2;  
      ax12SetRegister2(g_bServoID, AX_GOAL_SPEED_L, g_wServoGoalSpeed);
//      ax12ReadPacket(6);  // git the response...
      Serial.print("Goal Speed: ");
      Serial.print(g_wServoGoalSpeed, DEC);
    }
  } 
  else 
    g_wServoGoalPos = w1;  // Only 1 paramter so assume it is the new position

  // Now lets try moving that servo there   
  ax12SetRegister2(g_bServoID, AX_GOAL_POSITION_L, g_wServoGoalPos);
//  ax12ReadPacket(6);  // git the response...
  Serial.print(" ID: ");
  Serial.print(g_bServoID, DEC);
  Serial.print(" ");
  Serial.println(g_wServoGoalPos, DEC);
}  

//=======================================================================================
void SetServoID(void) {
  word w1;
  word w2;

  if (!FGetNextCmdNum(&w1))
    return;    // no parameters so bail.

  if (!FGetNextCmdNum(&w2))
    return;    // no parameters so bail.

  Serial.print("Set Servo ID From: ");
  Serial.print(w1, DEC);
  Serial.print(" To: ");
  Serial.println(w2, DEC);

  // Now lets try moving that servo there   
  ax12SetRegister(w1, AX_ID, w2);
//  ax12ReadPacket(6);  // git the response...
}  


//=======================================================================================
void WaitForMoveToComplete(word wID) {
  do {
    //    delay(1);
  } 
  while (ax12GetRegister(wID, AX_MOVING, 1));
}



//=======================================================================================
void GetServoPositions(void) {

  unsigned long ulBefore;
  unsigned long ulDelta;
  int w;
  for (int i = 0; i < NUM_SERVOS; i++) {
    Serial.print((byte)pgm_read_byte(&pgm_axdIDs[i]), DEC);
    Serial.print(":");
    ulBefore = micros();
    w = ax12GetRegister(pgm_read_byte(&pgm_axdIDs[i]), AX_PRESENT_POSITION_L, 2 );
    ulDelta = micros() - ulBefore;
    Serial.print(w, DEC);
    Serial.print(" ");
    Serial.print(ulDelta, DEC);
    Serial.print(" ");
    Serial.println(ax12GetRegister(pgm_read_byte(&pgm_axdIDs[i]), AX_RETURN_DELAY_TIME, 1), DEC);

    if (w == 0xffff) {
      Serial.print("   Retry: ");
      w = ax12GetRegister(pgm_read_byte(&pgm_axdIDs[i]), AX_PRESENT_POSITION_L, 2 );
      Serial.println(w, DEC);
    }    
    delay (100);
  }
}
//=======================================================================================
int g_asPositionsPrev[NUM_SERVOS];
int g_asMins[NUM_SERVOS];
int g_asMaxs[NUM_SERVOS];

void TrackServos(boolean fInit) {

  int w;
  bool fSomethingChanged = false;
  for (int i = 0; i < NUM_SERVOS; i++) {
    w = ax12GetRegister(pgm_read_byte(&pgm_axdIDs[i]), AX_PRESENT_POSITION_L, 2 );
    if (fInit) {
      g_asMins[i] = w;
      g_asMaxs[i] = w;
    }
    if (w != g_asPositionsPrev[i]) {
      if (!fInit) {
        // only print if we moved more than some delta...
        if (abs(w-g_asPositionsPrev[i]) > 3) {
          Serial.print(IKPinsNames[i]);
          Serial.print("(");
          Serial.print((byte)pgm_read_byte(&pgm_axdIDs[i]), DEC);
          Serial.print("):");
          Serial.print(w, DEC);
          Serial.print("(");
          Serial.print((((long)(w-512))*375L)/128L, DEC);
          Serial.print(") ");
          fSomethingChanged = true;
        }
      }
      g_asPositionsPrev[i] = w;
      if (g_asMins[i] > w)
        g_asMins[i] = w;

      if (g_asMaxs[i] < w)
        g_asMaxs[i] = w;
    }  
  }
  if (fSomethingChanged)
    Serial.println();
}

//=======================================================================================
void TrackPrintMinsMaxs(void) {
  for (int i = 0; i < NUM_SERVOS; i++) {
    Serial.print((byte)pgm_read_byte(&pgm_axdIDs[i]), DEC);
    Serial.print(":");
    Serial.print(g_asMins[i], DEC);
    Serial.print("(");
    Serial.print((((long)(g_asMins[i]-512))*375L)/128L, DEC);
    Serial.print(") ");

    Serial.print(g_asMaxs[i], DEC);
    Serial.print("(");
    Serial.print((((long)(g_asMaxs[i]-512))*375L)/128L, DEC);
    Serial.println(")");
  }
}


//=======================================================================================
void PrintServoValues(void) {

  word wID;
  word w;
  if (!FGetNextCmdNum(&wID))
    return;
  for (int i = 0; i < 50; i++) {
    Serial.print(i, DEC);
    Serial.print(":");
    w = ax12GetRegister(wID, i, 1 );
    Serial.print(w, HEX);
    Serial.print(" ");
    if ((i%10) == 9)
      Serial.println("");
    Serial.flush();  // try to avoid any interrupts while processing.
    delay(5);
  }    
}
//=======================================================================================













