//====================================================================================================
// Kurts Test program to try out different ways to manipulate the AX12 servos on the PhantomX
// This is a test, only a test...  
//====================================================================================================
//============================================================================
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
// Options...
//=============================================================================

// Uncomment the next line if building for a Quad instead of a Hexapod.
//#define HROS1_MODE
//#define QUAD_MODE
#define TURRET
#define SERVO1_SPECIAL

#define MAX_SERVOS  32
//=============================================================================
// Define differnt robots..
//=============================================================================

// Constants
/* Servo IDs */
// ID's for hexapod and quad
#define     RF_COXA       2
#define     RF_FEMUR      4
#define     RF_TIBIA      6

#define     RM_COXA      14
#define     RM_FEMUR     16
#define     RM_TIBIA     18

#define     RR_COXA       8
#define     RR_FEMUR     10
#define     RR_TIBIA     12

#ifdef SERVO1_SPECIAL
#define     LF_COXA       19
#else
#define     LF_COXA       1
#endif
#define     LF_FEMUR      3
#define     LF_TIBIA      5

#define     LM_COXA      13
#define     LM_FEMUR     15
#define     LM_TIBIA     17

#define     LR_COXA       7
#define     LR_FEMUR      9
#define     LR_TIBIA     11

#define     TURRET_ROT    20
#define     TURRET_TILT   21


//=============================================================================
// Hexapod
//=============================================================================
const uint8_t g_hex_pin_table[]  = {
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
const char* g_hex_pin_names[] = {
  "LFC","LFF","LFT",
  "LMC","LMF","LMT",
  "LRC","LRF","LRT",
  "RFC","RFF","RFT",
  "RMC","RMF","RMT",
  "RRC","RRF","RRT",
#ifdef TURRET
  "T-ROT", "T-TILT"
#endif
};

//=============================================================================
// Quad
//=============================================================================

const uint8_t g_quad_pin_table[]  = {
  LF_COXA, LF_FEMUR, LF_TIBIA,    
  LR_COXA, LR_FEMUR, LR_TIBIA,
  RF_COXA, RF_FEMUR, RF_TIBIA, 
  RR_COXA, RR_FEMUR, RR_TIBIA
#ifdef TURRET
  , TURRET_ROT, TURRET_TILT
#endif
};    

const char* g_quad_pin_names[] = {
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

//=============================================================================
// HROS1 
//=============================================================================
		enum
		{
			ID_R_SHOULDER_PITCH     = 1,
			ID_L_SHOULDER_PITCH     = 2,
			ID_R_SHOULDER_ROLL      = 3,
			ID_L_SHOULDER_ROLL      = 4,
			ID_R_ELBOW              = 5,
			ID_L_ELBOW              = 6,
			ID_R_HIP_YAW            = 7,
			ID_L_HIP_YAW            = 8,
			ID_R_HIP_ROLL           = 9,
			ID_L_HIP_ROLL           = 10,
			ID_R_HIP_PITCH          = 11,
			ID_L_HIP_PITCH          = 12,
			ID_R_KNEE               = 13,
			ID_L_KNEE               = 14,
			ID_R_ANKLE_PITCH        = 15,
			ID_L_ANKLE_PITCH        = 16,
			ID_R_ANKLE_ROLL         = 17,
			ID_L_ANKLE_ROLL         = 18,
			ID_HEAD_PAN             = 19,
			ID_HEAD_TILT            = 20
		};
const uint8_t g_hros1_pin_table[]  = {
            1,2,3,4,5,6,7,8,9,10,
            11,12,13,14,15,16,17,18,19,20}; // SHould use names above...

const char* g_hros1_pin_names[] = {
			"R_SHOULDER_PITCH", "L_SHOULDER_PITCH", "R_SHOULDER_ROLL", "L_SHOULDER_ROLL", 
            "R_ELBOW", 	"L_ELBOW", "R_HIP_YAW", "L_HIP_YAW",
			"R_HIP_ROLL", "L_HIP_ROLL", "R_HIP_PITCH", "L_HIP_PITCH",
			"R_KNEE", "L_KNEE", "R_ANKLE_PITCH", "L_ANKLE_PITCH",
			"R_ANKLE_ROLL", "L_ANKLE_ROLL", "HEAD_PAN", "HEAD_TILT",
		};


//=============================================================================
// Globals
//=============================================================================
// Servo Table: start off with Hexapod as default

#if defined (HROS1_MODE)
int            g_count_servos  = (sizeof(g_hros1_pin_table)/sizeof(g_hros1_pin_table[0]));
const uint8_t  * g_servo_id_table = g_hros1_pin_table;
const char     **g_servo_name_table = g_hros1_pin_names;
#elif defined(QUAD_MODE)
int            g_count_servos  = (sizeof(g_quad_pin_table)/sizeof(g_quad_pin_table[0]));
const uint8_t  * g_servo_id_table = g_quad_pin_table;
const char     **g_servo_name_table = g_quad_pin_names;
#else
int            g_count_servos  = (sizeof(g_hex_pin_table)/sizeof(g_hex_pin_table[0]));
const uint8_t  * g_servo_id_table = g_hex_pin_table;
const char     **g_servo_name_table = g_hex_pin_names;
#endif

uint8_t        g_id_controller = AX_ID_DEVICE;

/* IK Engine */
WrapperSerial Serial = WrapperSerial();

// other globals.
word           g_wVoltage;
char           g_aszCmdLine[120];
uint8_t        g_iszCmdLine;
boolean        g_fTrackServos = false;

int            g_asPositionsPrev[MAX_SERVOS];
int            g_asMins[MAX_SERVOS];
int            g_asMaxs[MAX_SERVOS];


// Values to use for servo position...
uint8_t        g_bServoID;
word           g_wServoGoalPos;
word           g_wServoGoalSpeed;

// forward references
extern void setup(void);
extern void loop(void);


extern void AllServosOff(void);
extern uint8_t GetCommandLine(void);
extern boolean FGetNextCmdNum(word *pw );
extern void AllServosCenter(void);
extern void HoldOrFreeServos(uint8_t fHold);
extern void SetServoPosition(void) ;
extern void SetServoID(void);
extern void WaitForMoveToComplete(word wID);
extern void GetServoPositions(void);
extern void SyncReadServoPositions(void);
extern void TrackServos(boolean fInit);
extern void TrackPrintMinsMaxs(void);
extern void PrintServoValues(void);
extern void SetServosReturnDelay(void);
extern void WriteServoRegisters(void);

//====================================================================================================
// SignalHandler - Try to free up things like servos if we abort.
//====================================================================================================
void SignalHandler(int sig){
    printf("Caught signal %d\n", sig);

    // Stop motors if they are active
    AllServosOff();
    printf("All Servos off\n");
    
    // If on 200 we may have to turn power off...
    if (g_id_controller== 200) {
        ax12SetRegister(g_id_controller, AX_TORQUE_ENABLE, 0x0);
        printf("Controller turn power off\n");
    }

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
            // See if hex, quad, biped (only look at first char)
            char c = *(argv[1]);
            if ((c == 'h') || (c == 'H')) 
            {
                printf("Hexapod\n");
                g_count_servos  = (sizeof(g_hex_pin_table)/sizeof(g_hex_pin_table[0]));
                g_servo_id_table = g_hex_pin_table;
                g_servo_name_table = g_hex_pin_names;
                if ((argc > 2) && ((*(argv[2]) == 'n') || (*(argv[2]) == 'N')))
                {
                    printf("No turret\n");
                    g_count_servos -= 2;
                }
            }
            else if ((c == 'q') || (c == 'Q')) 
            {
                printf("Quad\n");
                g_count_servos  = (sizeof(g_quad_pin_table)/sizeof(g_quad_pin_table[0]));
                g_servo_id_table = g_quad_pin_table;
                g_servo_name_table = g_quad_pin_names;
                if ((argc > 2) && ((*(argv[2]) == 'n') || (*(argv[2]) == 'N')))
                {
                    printf("No turret\n");
                    g_count_servos -= 2;
                }
            }
            else if ((c == 'b') || (c == 'B')) 
            {
                printf("Biped - HROS1\n");
                g_count_servos  = (sizeof(g_hros1_pin_table)/sizeof(g_hros1_pin_table[0]));
                g_servo_id_table = g_hros1_pin_table;
                g_servo_name_table = g_hros1_pin_names;
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
    if(dxl_initialize(0, 1) == 0) 
    {
        printf("Failed to open USBDynamixel\n");
    }	

    // First lets try to see what controller we are using, lets first try id 253 which is default USB2AX
    delay(1000);
    word  wModel = ax12GetRegister(g_id_controller, AX_MODEL_NUMBER_L, 2 );

    if (wModel == 0xffff) 
    {
        //Try 200 for CM730 like controller
        g_id_controller = 200;
        wModel = ax12GetRegister(g_id_controller, AX_MODEL_NUMBER_L, 2 );
    }

    if (wModel != 0xffff)
        printf("Controller model %x on %x(%d)\n", wModel, g_id_controller,  g_id_controller);
    else
    {
        printf("Controller not found\n");
        g_id_controller = 0;    // say that we don't have this
    }

    // If on 200 we may have to turn servo power on...
    if (g_id_controller== 200)
        ax12SetRegister(g_id_controller, AX_TORQUE_ENABLE, 0x1);

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
  Serial.println("5 - Sync Read Servo positions");
  Serial.println("7 - Set Servos return delay");
  Serial.println("8 - Set ID: <old> <new>");
  Serial.println("9 - Print Servo Values");
  Serial.println("t - Toggle track Servos");
  Serial.println("w - write <servo> <reg> <val> (can be multiple values...");
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
    case '5':
      SyncReadServoPositions();
      break;
    case '7' :
      SetServosReturnDelay();
      break;
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

    case 'w':
    case 'W':
      WriteServoRegisters();
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
  int cch;

  for(;;) {
    // throw away any thing less than CR character...
    cch = Serial.available();
    while (cch--) {
        ch = Serial.read();
        if ((ch >= 10) && (ch <=15)) {
          g_aszCmdLine[ich] = 0;
          return ich;
        }    
        if (ch != -1) 
          g_aszCmdLine[ich++] = ch;
    }
    if (g_fTrackServos)
      TrackServos(false);
    else
       delay(100);
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
void SetServosReturnDelay(void) {
    // Allow user to pass in, default 0
    word wDelay;
    
    if (!FGetNextCmdNum(&wDelay ))
        wDelay = 0;
    wDelay &= 0xff; // make sure in one byte    

    for (int i = 0; i < g_count_servos; i++) {
        ax12SetRegister(g_servo_id_table[i], AX_RETURN_DELAY_TIME, wDelay);
    }
}


//=======================================================================================
void AllServosOff(void) {
  for (int i = 0; i < g_count_servos; i++) {
    ax12SetRegister(g_servo_id_table[i], AX_TORQUE_ENABLE, 0x0);
  }
}


//=======================================================================================
void AllServosCenter(void) {
  for (int i = 0; i < g_count_servos; i++) {
    // See if this turns the motor off and I can turn it back on...
    ax12SetRegister(g_servo_id_table[i], AX_TORQUE_ENABLE, 0x1);
//    ax12ReadPacket(6);  // git the response...
    ax12SetRegister2(g_servo_id_table[i], AX_GOAL_POSITION_L, 0x1ff);
//    ax12ReadPacket(6);  // git the response...
  }
}
//=======================================================================================
void HoldOrFreeServos(uint8_t fHold) {
  word iServo;

  if (!FGetNextCmdNum(&iServo)) {
    // All servos...
    for (int i = 0; i < g_count_servos; i++) {
      ax12SetRegister(g_servo_id_table[i], AX_TORQUE_ENABLE, fHold);
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
  for (int i = 0; i < g_count_servos; i++) {
    Serial.print((uint8_t)g_servo_id_table[i], DEC);
    Serial.print("(");
    Serial.print(g_servo_name_table[i]);
    Serial.print("):");
    ulBefore = micros();
    w = ax12GetRegister(g_servo_id_table[i], AX_PRESENT_POSITION_L, 2 );
    ulDelta = micros() - ulBefore;
    Serial.print(w, DEC);
    Serial.print(" ");
    Serial.print(ulDelta, DEC);
    Serial.print(" ");
    Serial.println(ax12GetRegister(g_servo_id_table[i], AX_RETURN_DELAY_TIME, 1), DEC);

    if (w == 0xffff) {
      Serial.print("   Retry: ");
      w = ax12GetRegister(g_servo_id_table[i], AX_PRESENT_POSITION_L, 2 );
      Serial.println(w, DEC);
    }    
    delay (100);
  }
}
//=======================================================================================
void SyncReadServoPositions(void) {

  unsigned long ulBefore;
  unsigned long ulDelta;
  int w;
  
#define AX_CMD_SYNC_READ      0x84
#define PARAMETER             (5)  
    // Setup DXL servo packet
    // update each servo
    dxl_set_txpacket_id(BROADCAST_ID);
    dxl_set_txpacket_instruction(INST_SYNC_READ);
    dxl_set_txpacket_parameter(0, AX_GOAL_POSITION_L);
    dxl_set_txpacket_parameter(1, 2);
    
    for (int i = 0; i < g_count_servos; i++) {
        dxl_set_txpacket_parameter(2+i, g_servo_id_table[i]);
    }    
        
    dxl_set_txpacket_length(g_count_servos + 4);
    
    ulBefore = micros();
    dxl_txrx_packet();
    int result = dxl_get_result();   // don't care for now
    ulDelta = micros() - ulBefore;
    Serial.print("Sync Read ");
    Serial.print(result, DEC);
    Serial.print(" time: ");
    Serial.println(ulDelta, DEC);

    // Now print out results
    for (int i = 0; i < g_count_servos; i++) {
        Serial.print((uint8_t)g_servo_id_table[i], DEC);
        Serial.print("(");
        Serial.print(g_servo_name_table[i]);
        Serial.print("):");
        w = dxl_makeword(dxl_get_rxpacket_parameter(i*2), (int)dxl_get_rxpacket_parameter(i*2+1));
        Serial.println(w, DEC);
    }    
    delay (100);
}


//=======================================================================================

void WriteServoRegisters(void) {
    unsigned long ulBefore;
    unsigned long ulDelta;
    word wID;
    word wReg;
    word w;
    uint8_t cBytes = 0;
    uint8_t ab[10];   // maximum 10 registers, probably never more than 2 or 3...

    if (!FGetNextCmdNum(&wID))
        return;    // no parameters so bail.

    if (!FGetNextCmdNum(&wReg))
        return;    // no parameters so bail.

    while (FGetNextCmdNum(&w) && (cBytes < sizeof(ab))) {
        ab[cBytes++] = w & 0xff;
    }    
    
    if (! cBytes)
        return;

    // Now try to set the register(s)... 
    dxl_set_txpacket_id(wID);
    dxl_set_txpacket_instruction(INST_WRITE);
    dxl_set_txpacket_parameter(0, wReg);
    
    for (int i = 0; i < cBytes; i++) {
        dxl_set_txpacket_parameter(i+1, ab[i]);
    }    
        
    dxl_set_txpacket_length(3 + cBytes);
    
    ulBefore = micros();
    dxl_txrx_packet();
    int result = dxl_get_result();   // don't care for now
    ulDelta = micros() - ulBefore;
    printf("Write Register %d %d %d Res=%d time=%lu\n", wID, wReg, cBytes, result, ulDelta);
}

//=======================================================================================

void TrackServos(boolean fInit) {

  int w;
  bool fSomethingChanged = false;
  for (int i = 0; i < g_count_servos; i++) {
    w = ax12GetRegister(g_servo_id_table[i], AX_PRESENT_POSITION_L, 2 );
    if (fInit) {
      g_asMins[i] = w;
      g_asMaxs[i] = w;
    }
    if (w != g_asPositionsPrev[i]) {
      if (!fInit) {
        // only print if we moved more than some delta...
        if (abs(w-g_asPositionsPrev[i]) > 3) {
          Serial.print(g_servo_name_table[i]);
          Serial.print("(");
          Serial.print((uint8_t)g_servo_id_table[i], DEC);
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
  for (int i = 0; i < g_count_servos; i++) {
    Serial.print((uint8_t)g_servo_id_table[i], DEC);
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













