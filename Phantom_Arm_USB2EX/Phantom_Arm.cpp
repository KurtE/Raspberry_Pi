#define DEBUG
//=============================================================================
//Project Kurt's PhantomX Reactor Arm
// This code is setup to control a PhantomX Reactor Arm which is sold by Trossen
// Robotics: http:www.trossenrobotics.com/p/phantomx-ax-12-reactor-robot-arm.aspx
// This code uses a Arbotix Commander 2 to control it, also sold by Trossen 
// Robotics: http:www.trossenrobotics.com/p/arbotix-commander-gamepad-v2.aspx
//
//  There are a couple of buttons that I am currently using for all modes, which include:
//    Turning Commander on/off - When the Arm starts receiving valid packets, it assumes
//    the commander is on and it turns on the servos moves the arm to the home position.  If
//    the arm does not receive a valid packet for a time out period of time it assumes the 
//    commander has been turned off and it moves the arm to a park position and then frees
//    the servos.
//  
//    R1 - Cycles through the different Modes.
//    R2 - Moves the arm to its home position.
//    R3 - Toggles debug mode on or off (optional, turn off by un-defining DEBUG).  When on, it outputs
//		debug information to the Serial port.  I use a simple VB forwarding app to forward messages from 
//		commander to robot, and robot sent messages show up on terminal...
//
//  The code currently has a few different modes of operation.
//  Mode 1: 3d Cartesian IK code, which is based in part off of code by Michael E. Ferguson
//    x is controlled by left joystick horizontal
//    y by left joystick vertical
//    z by right joystick vertical
//    Grip angle by right joystick horizontal
//    Gripper and Wrist rotate are controlled by holding L6 and right joystick.
//  Mode 2: 3/2D cylindrical coordinate system.  More or less like 1: except
//    Base Rotation control by left joystick horizontal
//    Distance from base by left vertical
//    ..
//  Mode 3: Backhoe operation
//    Left Horizontal - Rotate
//    Left Vertical - Controls the dipper (elbow to wrist)
//    Right Veritical - Controls Boom (Shoulder to Elbow)
//    Right Horizontal - Bucket Curl...
//    ...
//
//  This code is a Work In Progress and is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
//  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
//  
//=============================================================================

//=============================================================================
// Define Options
//=============================================================================
#define OPT_WRISTROT  
#define ARBOTIX_TO  1000      // if no message for a second probably turned off...
#define DEADZONE    3        // deadzone around center of joystick values
#define SOUND_PIN    1      // Tell system we have added speaker to IO pin 1
#define MAX_SERVO_DELTA_PERSEC 512
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
#include "CommanderEx.h"

//=============================================================================
//=============================================================================
/* Servo IDs */
enum {
   // Warning changed to Index instead of ID.  ID=Index+1;
  SID_BASE=0, SID_RSHOULDER, SID_LSHOULDER, SID_RELBOW, SID_LELBOW, SID_WRIST, SID_WRISTROT, SID_GRIP};

enum {
  IKM_IK3D_CARTESIAN, IKM_CYLINDRICAL, IKM_BACKHOE};

// status messages for IK return codes..
enum {
  IKS_SUCCESS=0, IKS_WARNING, IKS_ERROR};


#define CNT_SERVOS  8 //(sizeof(pgm_axdIDs)/sizeof(pgm_axdIDs[0]))

// Define some Min and Maxs for IK Movements...
//                y   Z
//                |  /
//                |/
//            ----+----X (X and Y are flat on ground, Z is up in air...
//                |
//                |
#define IK_MAX_X  300
#define IK_MIN_X  -300

#define IK_MAX_Y  350
#define IK_MIN_Y  50

#define IK_MAX_Z  300
#define IK_MIN_Z  20

#define IK_MAX_GA  90
#define IK_MIN_GA   -90

// Define Ranges for the different servos...
#define BASE_MIN    0
#define BASE_MAX    1023

#define SHOULDER_MIN  205 
#define SHOULDER_MAX  810

#define ELBOW_MIN    210
#define ELBOW_MAX    900

#define WRIST_MIN    200
#define WRIST_MAX    830

#define WROT_MIN     0
#define WROT_MAX     1023

#define GRIP_MIN     0
#define GRIP_MAX     512

// Define some lengths and offsets used by the arm
#define BaseHeight          110L   // (L0)about 120mm 
#define ShoulderLength      150L   // (L1)Not sure yet what to do as the servo is not directly in line,  Probably best to offset the angle?
//                                 // X is about 140, y is about 40 so sqrt is Hyp is about 155, so maybe about 21 degrees offset
#define ShoulderServoOffset 72L    // should offset us some...
#define ElbowLength         147L   //(L2)Length of the Arm from Elbow Joint to Wrist Joint
#define WristLength         137L   // (L3)Wrist length including Wrist rotate
#define G_OFFSET            0      // Offset for static side of gripper?

#define IK_FUDGE            5     // How much a fudge between warning and error

//=============================================================================
// Global Objects
//=============================================================================
Commander       command = Commander();
BioloidControllerEx bioloid = BioloidControllerEx(1000000);
WrapperSerial   Serial = WrapperSerial();

//=============================================================================
// Global Variables...
//=============================================================================
boolean         g_fArmActive = false;   // Is the arm logically on?
byte            g_bIKMode = IKM_IK3D_CARTESIAN;   // Which mode of operation are we in...
uint8_t         g_bIKStatus = IKS_SUCCESS;   // Status of last call to DoArmIK;
boolean         g_fServosFree = true;

// Current IK values
int            g_sIKGA;                  // IK Gripper angle..
int            g_sIKX;                  // Current X value in mm
int            g_sIKY;                  //
int            g_sIKZ;

// Values for current servo values for the different joints
int             g_sBase;                // Current Base servo value
int             g_sShoulder;            // Current shoulder target 
int             g_sElbow;               // Current
int             g_sWrist;               // Current Wrist value
int             g_sWristRot;            // Current Wrist rotation
int             g_sGrip;                // Current Grip position

// BUGBUG:: I hate too many globals...
int sBase, sShoulder, sElbow, sWrist, sWristRot, sGrip;


// Message informatino
unsigned long   ulLastMsgTime;          // Keep track of when the last message arrived to see if controller off
byte            buttonsPrev;            // will use when we wish to only process a button press once

//
#ifdef DEBUG
boolean        g_fDebugOutput = false;
#endif

// Forward references
extern void MSound(byte cNotes, ...);
extern void setup(void);
extern void loop();
extern boolean ProcessUserInput3D(void);
extern boolean ProcessUserInputCylindrical();
extern boolean ProcessUserInputBackHoe();
extern void MoveArmToHome(void);
extern void PutArmToSleep(void);
extern void MoveArmTo(int sBase, int sShoulder, int sElbow, int sWrist, int sWristRot, int sGrip, int wTime, boolean fWait);
extern uint8_t doArmIK(boolean fCartesian, int sIKX, int sIKY, int sIKZ, int sIKGA);

//--------------------------------------------------------------------------
// SignalHandler - Try to free up things like servos if we abort.
//--------------------------------------------------------------------------
void SignalHandler(int sig){
    printf("Caught signal %d\n", sig);

    // Stop motors if they are active
    if (g_fArmActive)
		PutArmToSleep();
    command.end();	// tell the commander to release stuff...

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

//===================================================================================================
// Setup 
//====================================================================================================
void setup() {
  // Lets initialize the Commander
  Serial.begin();
  
	if (!command.begin("/dev/ttyXBEE", B38400))
	{
		printf("Commander Begin failed\n");
		return;
	}    

  // Next initialize the Bioloid
  printf("Before poseSize\n");
  bioloid.poseSize(CNT_SERVOS);

  // Read in the current positions...
  printf("Before readPose\n");
    bioloid.readPose();
  printf("After readPose\n");
  for (int i=1; i <= CNT_SERVOS; i++) {
    Serial.print(bioloid.getCurPoseByIndex(i), DEC);
    Serial.print("=?");
    Serial.println(dxl_read_word(i, AX_PRESENT_POSITION_L), DEC);
  }  
    
  // Start off to put arm to sleep...
  Serial.println("Kurt's Arm");

  PutArmToSleep();

  MSound(3, 60, 2000, 80, 2250, 100, 2500);

}
//===================================================================================================
// loop: Our main Loop!
//===================================================================================================
void loop() {
  boolean fChanged = false;
  if (command.ReadMsgs()) {
    // See if the Arm is active yet...
    if (g_fArmActive) {
      sBase = g_sBase;
      sShoulder = g_sShoulder;
      sElbow = g_sElbow; 
      sWrist = g_sWrist;
      sGrip = g_sGrip;
      sWristRot = g_sWristRot;

      if ((command.buttons & BUT_R1) && !(buttonsPrev & BUT_R1)) {
        if (++g_bIKMode > IKM_BACKHOE)
          g_bIKMode = 0; 

        // For now lets always move arm to the home position of the new input method...
        // Later maybe we will get the current position and covert to the coordinate system
        // of the current input method.
        MoveArmToHome();      

      }
      else if ((command.buttons & BUT_R2) && !(buttonsPrev & BUT_R2)) {
        MoveArmToHome();      
      }
#ifdef DEBUG
      if ((command.buttons & BUT_R3) && !(buttonsPrev & BUT_R3)) {
        g_fDebugOutput = !g_fDebugOutput;
        MSound( 1, 45, g_fDebugOutput? 3000 : 2000);
      }
#endif
      // Going to use L6 in combination with the right joystick to control both the gripper and the 
      // wrist rotate...
      else if (command.buttons & BUT_L6) {
        sGrip = min(max(sGrip + command.rightV/2, GRIP_MIN), GRIP_MAX);
        sWristRot = min(max(g_sWristRot + command.rightH/6, WROT_MIN), WROT_MAX);
        fChanged = (sGrip != g_sGrip) || (sWristRot != g_sWristRot);
      }
      else {
        switch (g_bIKMode) {
        case IKM_IK3D_CARTESIAN:
          fChanged |= ProcessUserInput3D();
          break;
        case IKM_CYLINDRICAL:
          fChanged |= ProcessUserInputCylindrical();
          break;

        case IKM_BACKHOE:
          fChanged |= ProcessUserInputBackHoe();
          break;
        }
      }
      // If something changed and we are not in an error condition
      if (fChanged && (g_bIKStatus != IKS_ERROR)) {
        MoveArmTo(sBase, sShoulder, sElbow, sWrist, sWristRot, sGrip, 100, true);
      }
//      else if (bioloid.interpolating() > 0) {
//        bioloid.interpolateStep();
//      }
    }
    else {
      g_fArmActive = true;
      MoveArmToHome();      
    }

    buttonsPrev = command.buttons;
    ulLastMsgTime = millis();    // remember when we last got a message...
  }
  else {
//    if (bioloid.interpolating() > 0) {
//      bioloid.interpolateStep();
//    }
    // error see if we exceeded a timeout
    if (g_fArmActive && ((millis() - ulLastMsgTime) > ARBOTIX_TO)) {
      PutArmToSleep();
    }
  }
} 
//===================================================================================================
// ProcessUserInput3D: Process the Userinput when we are in 3d Mode
//===================================================================================================
boolean ProcessUserInput3D(void) {
  // We Are in IK mode, so figure out what position we would like the Arm to move to.
  // We have the Coordinates system like:
  //
  //                y   Z
  //                |  /
  //                |/
  //            ----+----X (X and Y are flat on ground, Z is up in air...
  //                |
  //                |
  //
  boolean fChanged = false;
  int   sIKX;                  // Current X value in mm
  int   sIKY;                  //
  int   sIKZ;
  int   sIKGA;
  // Limit how far we can go by checking the status of the last move.  If we are in a warning or error
  // condition, don't allow the arm to move farther away...
  if (g_bIKStatus == IKS_SUCCESS) {
    sIKX = min(max(g_sIKX + command.leftH/6, IK_MIN_X), IK_MAX_X);
    sIKY = min(max(g_sIKY + command.leftV/6, IK_MIN_Y), IK_MAX_Y);
    sIKZ = min(max(g_sIKZ + command.rightV/6, IK_MIN_Z), IK_MAX_Z);
    sIKGA = min(max(g_sIKGA + command.rightH/10, IK_MIN_GA), IK_MAX_GA);  // Currently in Servo coords...
  } 
  else {
    // In an Error/warning condition, only allow things to move in closer...
    sIKX = g_sIKX;
    sIKY = g_sIKY;
    sIKZ = g_sIKZ;
    sIKGA = g_sIKGA;

    if (((g_sIKX > 0) && (command.leftH < 0)) || ((g_sIKX < 0) && (command.leftH > 0)))
      sIKX = min(max(g_sIKX + command.leftH/6, IK_MIN_X), IK_MAX_X);
    if (((g_sIKY > 0) && (command.leftV < 0)) || ((g_sIKY < 0) && (command.leftV > 0)))
      sIKY = min(max(g_sIKY + command.leftV/6, IK_MIN_Y), IK_MAX_Y);
    if (((g_sIKZ > 0) && (command.rightV < 0)) || ((g_sIKZ < 0) && (command.rightV > 0)))
      sIKZ = min(max(g_sIKZ + command.rightV/6, IK_MIN_Z), IK_MAX_Z);
    if (((g_sIKGA > 0) && (command.rightH < 0)) || ((g_sIKGA < 0) && (command.rightH > 0)))
      sIKGA = min(max(g_sIKGA + command.rightH/6, IK_MIN_GA), IK_MAX_GA);  // Currently in Servo coords...
  }
  fChanged = (sIKX != g_sIKX) || (sIKY != g_sIKY) || (sIKZ != g_sIKZ) || (sIKGA != g_sIKGA) ;

  if (fChanged) {
    g_bIKStatus = doArmIK(true, sIKX, sIKY, sIKZ, sIKGA);
  }
  return fChanged;
}

//===================================================================================================
// ProcessUserInputCylindrical: Process the Userinput when we are in 3d Mode
//===================================================================================================
boolean ProcessUserInputCylindrical() {
  // We Are in IK mode, so figure out what position we would like the Arm to move to.
  // We have the Coordinates system like:
  //
  //                y   Z
  //                |  /
  //                |/
  //            ----+----X (X and Y are flat on ground, Z is up in air...
  //                |
  //                |
  //
  boolean fChanged = false;
  int   sIKY;                  // Distance from base in mm
  int   sIKZ;
  int   sIKGA;

  // Will try combination of the other two modes.  Will see if I need to do the Limits on the IK values
  // or simply use the information from the Warning/Error from last call to the IK function...
  sIKY = g_sIKY;
  sIKZ = g_sIKZ;
  sIKGA = g_sIKGA;

  // The base rotate is real simple, just allow it to rotate in the min/max range...
  sBase = min(max(g_sBase - command.leftH/6, BASE_MIN), BASE_MAX);

  // Limit how far we can go by checking the status of the last move.  If we are in a warning or error
  // condition, don't allow the arm to move farther away...
  // Use Y for 2d distance from base
  if ((g_bIKStatus == IKS_SUCCESS) || ((g_sIKY > 0) && (command.leftV < 0)) || ((g_sIKY < 0) && (command.leftV > 0)))
    sIKY += command.leftV/6;

  // Now Z coordinate...
  if ((g_bIKStatus == IKS_SUCCESS) || ((g_sIKZ > 0) && (command.rightV < 0)) || ((g_sIKZ < 0) && (command.rightV > 0)))
    sIKZ += command.rightV/6;

  // And gripper angle.  May leave in Min/Max here for other reasons...   
  if ((g_bIKStatus == IKS_SUCCESS) || ((g_sIKGA > 0) && (command.rightH < 0)) || ((g_sIKGA < 0) && (command.rightH > 0)))
    sIKGA = min(max(g_sIKGA + command.rightH/6, IK_MIN_GA), IK_MAX_GA);  // Currently in Servo coords...

  fChanged = (sBase != g_sBase) || (sIKY != g_sIKY) || (sIKZ != g_sIKZ) || (sIKGA != g_sIKGA) ;

  if (fChanged) {
    g_bIKStatus = doArmIK(false, sBase, sIKY, sIKZ, sIKGA);
  }
  return fChanged;
}
//===================================================================================================
// ProcessUserInputBackHoe: Process the Userinput when we are in 3d Mode
//===================================================================================================
boolean ProcessUserInputBackHoe() {
  // lets update positions with the 4 joystick values
  // First the base
  boolean fChanged = false;
  sBase = min(max(g_sBase - command.leftH/6, BASE_MIN), BASE_MAX);
  if (sBase != g_sBase)
    fChanged = true;

  // Now the Boom
  sShoulder = min(max(g_sShoulder + command.rightV/6, SHOULDER_MIN), SHOULDER_MAX);
  if (sShoulder != g_sShoulder)
    fChanged = true;

  // Now the Dipper 
  sElbow = min(max(g_sElbow + command.leftV/6, ELBOW_MIN), ELBOW_MAX);
  if (sElbow != g_sElbow)
    fChanged = true;

  // Bucket Curl
  sWrist = min(max(g_sWrist + command.rightH/6, WRIST_MIN), WRIST_MAX);
  if (sWrist != g_sWrist)
    fChanged = true;
  return fChanged;
}
//===================================================================================================
// MoveArmToHome
//===================================================================================================
void MoveArmToHome(void) {

  if (g_bIKMode != IKM_BACKHOE) {
    g_bIKStatus = doArmIK(true, 0, (2*ElbowLength)/3+WristLength, BaseHeight+(2*ShoulderLength)/3, 0);
    MoveArmTo(sBase, sShoulder, sElbow, sWrist, 512, 256, 2000, true);
  }
  else {
    g_bIKStatus = IKS_SUCCESS;  // assume sucess so we will continue to move...
    MoveArmTo(512, 512, 330, 690, 512, 256, 2000, true);
  }
}

//===================================================================================================
// PutArmToSleep
//===================================================================================================
void PutArmToSleep(void) {
  g_fArmActive = false;
  MoveArmTo(512, 212, 212, 512, 512, 256, 1000, true);

  // And Relax all of the servos...
  for(uint8_t i=1; i <= CNT_SERVOS; i++) {
    Relax(i);
  }
  g_fServosFree = true;
}

//===================================================================================================
// MoveArmTo
//===================================================================================================
void MoveArmTo(int sBase, int sShoulder, int sElbow, int sWrist, int sWristRot, int sGrip, int wTime, boolean fWait) {

  int sMaxDelta;
  int sDelta;

  // First make sure servos are not free...
  if (g_fServosFree) {
    g_fServosFree = false;

    for(uint8_t i=1; i <= CNT_SERVOS; i++) {
      TorqueOn(i);
    }
    bioloid.readPose();
  }


#ifdef DEBUG
  if (g_fDebugOutput) {
    Serial.print("[");
    Serial.print(sBase, DEC);
    Serial.print(" ");
    Serial.print(sShoulder, DEC);
    Serial.print(" ");
    Serial.print(sElbow, DEC);
    Serial.print(" ");
    Serial.print(sWrist, DEC);
    Serial.print(" ");
    Serial.print(sWristRot, DEC);
    Serial.print(" ");
    Serial.print(sGrip, DEC);
    Serial.println("]");
  }
#endif
  // Make sure the previous movement completed.
  // Need to do it before setNextPos calls as this
  // is used in the interpolating code...
  while (bioloid.interpolating() > 0) {
//    bioloid.interpolateStep();
    delay(3);
  }

  // Also lets limit how fast the servos will move as to not get whiplash.
  bioloid.setNextPoseByIndex(SID_BASE, sBase);

  sMaxDelta = abs(bioloid.getNextPoseByIndex(SID_RSHOULDER) - sShoulder);
  bioloid.setNextPoseByIndex(SID_RSHOULDER, sShoulder);
  bioloid.setNextPoseByIndex(SID_LSHOULDER, 1024-sShoulder);

  sDelta = abs(bioloid.getNextPoseByIndex(SID_RELBOW) - sElbow);
  if (sDelta > sMaxDelta)
    sMaxDelta = sDelta;
  bioloid.setNextPoseByIndex(SID_RELBOW, sElbow);
  bioloid.setNextPoseByIndex(SID_LELBOW, 1024-sElbow);

  sDelta = abs(bioloid.getNextPoseByIndex(SID_WRIST) - sWrist);
  if (sDelta > sMaxDelta)
    sMaxDelta = sDelta;
  bioloid.setNextPoseByIndex(SID_WRIST, sWrist);

#ifdef OPT_WRISTROT
  bioloid.setNextPoseByIndex(SID_WRISTROT, sWristRot); 
#endif  

  bioloid.setNextPoseByIndex(SID_GRIP, sGrip);


  // Save away the current positions...
  g_sBase = sBase;
  g_sShoulder = sShoulder;
  g_sElbow = sElbow;
  g_sWrist = sWrist;
  g_sWristRot = sWristRot;
  g_sGrip = sGrip;

  // Now start the move - But first make sure we don't move too fast.  
  if (((long)sMaxDelta*wTime/1000L) > MAX_SERVO_DELTA_PERSEC) {
    wTime = ((long)sMaxDelta*1000L)/ MAX_SERVO_DELTA_PERSEC;
  }

  int err = bioloid.interpolateSetup(wTime);
  printf("Interpolate Setup returned: %d\n\r", err);
  // Do at least the first movement
//  bioloid.interpolateStep();

  // And if asked to, wait for the previous move to complete...
  if (fWait) {
    while (bioloid.interpolating() > 0) {
//      bioloid.interpolateStep();
      delay(3);
    }
  }
}

//===================================================================================================
// Convert radians to servo position offset. 
//===================================================================================================
int radToServo(float rads){
  float val = (rads*100)/51 * 100;
  return (int) val;
}


//===================================================================================================
// Compute Arm IK for 3DOF+Mirrors+Gripper - was based on code by Michael E. Ferguson
// Hacked up by me, to allow different options...
//===================================================================================================
#define sq(x) ((x)*(x))
uint8_t doArmIK(boolean fCartesian, int sIKX, int sIKY, int sIKZ, int sIKGA)
{
  int t;
  int sol0;
  uint8_t bRet = IKS_SUCCESS;  // assume success
#ifdef DEBUG
  if (g_fDebugOutput) {
    Serial.print("(");
    Serial.print(sIKX, DEC);
    Serial.print(",");
    Serial.print(sIKY, DEC);
    Serial.print(",");
    Serial.print(sIKZ, DEC);
    Serial.print(",");
    Serial.print(sIKGA, DEC);
    Serial.print(")=");
  }
#endif
  if (fCartesian) {
    // first, make this a 2DOF problem... by solving base
    sol0 = radToServo(atan2(sIKX,sIKY));
    // remove gripper offset from base
    t = sqrt(sq((long)sIKX)+sq((long)sIKY));
    // BUGBUG... Not sure about G here
#define G 30   
    sol0 -= radToServo(atan2((G/2)-G_OFFSET,t));
  }
  else {
    // We are in cylindrical mode, probably simply set t to the y we passed in...
    t = sIKY;
#ifdef DEBUG
    sol0 = 0;
#endif
  }
  // convert to sIKX/sIKZ plane, remove wrist, prepare to solve other DOF           
  float flGripRad = (float)(sIKGA)*3.14159/180.0;
  long trueX = t - (long)((float)WristLength*cos(flGripRad));   
  long trueZ = sIKZ - BaseHeight - (long)((float)WristLength*sin(flGripRad));

  long im = sqrt(sq(trueX)+sq(trueZ));        // length of imaginary arm
  float q1 = atan2(trueZ,trueX);              // angle between im and X axis
  long d1 = sq(ShoulderLength) - sq(ElbowLength) + sq((long)im);
  long d2 = 2*ShoulderLength*im;
  float q2 = acos((float)d1/float(d2));
  q1 = q1 + q2;
  int sol1 = radToServo(q1-1.57);

  d1 = sq(ShoulderLength)-sq((long)im)+sq(ElbowLength);
  d2 = 2*ElbowLength*ShoulderLength;
  q2 = acos((float)d1/(float)d2);
  int sol2 = radToServo(3.14-q2);

  // solve for wrist rotate
  int sol3 = radToServo(3.2 + flGripRad - q1 - q2 );

#ifdef DEBUG
  if (g_fDebugOutput) {
    Serial.print("<");
    Serial.print(sol0, DEC);
    Serial.print(",");
    Serial.print(trueX, DEC);
    Serial.print(",");
    Serial.print(trueZ, DEC);
    Serial.print(",");
    Serial.print(sol1, DEC);
    Serial.print(",");
    Serial.print(sol2, DEC);
    Serial.print(",");
    Serial.print(sol3, DEC);
    Serial.print(">");
  }
#endif   

  // Lets calculate the actual servo values.

  if (fCartesian) {
    sBase = min(max(512 - sol0, BASE_MIN), BASE_MAX);
  }
  sShoulder = min(max(512 - sol1, SHOULDER_MIN), SHOULDER_MAX);

  // Magic Number 819???
  sElbow = min(max(819 - sol2, SHOULDER_MIN), SHOULDER_MAX);

#define Wrist_Offset 512
  sWrist = min(max(Wrist_Offset + sol3, WRIST_MIN), WRIST_MAX);

  // Remember our current IK positions
  g_sIKX = sIKX; 
  g_sIKY = sIKY;
  g_sIKZ = sIKZ;
  g_sIKGA = sIKGA;
  // Simple test im can not exceed the length of the Shoulder+Elbow joints...

  if (im > (ShoulderLength + ElbowLength)) {
    if (g_bIKStatus != IKS_ERROR) {
#ifdef DEBUG
      if (g_fDebugOutput) {
        Serial.println("IK Error");
      }
#endif
      MSound(2, 50, 3000, 50, 3000);
    }
    bRet = IKS_ERROR;  
  }
  else if(im > (ShoulderLength + ElbowLength-IK_FUDGE)) {
    if (g_bIKStatus != IKS_WARNING) {
#ifdef DEBUG
      if (g_fDebugOutput) {
        Serial.println("IK Warning");
      }
#endif
      MSound(1, 75, 2500);
    }
    bRet = IKS_WARNING;  
  }

  return bRet;
}

void MSound(byte cNotes, ...)
{
	// start off without doing sound
}







