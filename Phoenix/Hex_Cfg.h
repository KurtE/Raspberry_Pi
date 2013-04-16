//====================================================================
//Project Lynxmotion Phoenix
//Description:
//    This is the hardware configuration file for the Hex Robot.
//    This Header file is specific for T-Hex with 3 DOF
//
//Programmer: Kurt (aka KurtE)
//
//NEW IN V1.0
//   - First Release
//
//====================================================================
#ifndef HEX_CFG_H
#define HEX_CFG_H
#include "ArduinoDefs.h"

//==================================================================================================================================
// Define which input classes we will use. If we wish to use more than one we need to define USEMULTI - This will define a forwarder
//    type implementation, that the Inputcontroller will need to call.  There will be some negotion for which one is in contol.
//
//  If this is not defined, The included Controller should simply implement the InputController Class...
//==================================================================================================================================
//#define USEMULTI
//#define USEXBEE            // only allow to be defined on Megas...
//#define USEPS2
#define USECOMMANDER

// Global defines to control which configuration we are using.  Note: Only define one of these...
//
// Which type of control(s) do you want to compile in
#ifdef USEXBEE                                    // some options only valid if running with XBEE stuff
#define XBEE_DEBUG_OUTPUT                         // use our xbee serial class to do debug stuff
#define DBGSerial XBDSerial
#endif

#define DBGSerial         DBGSerialWrapper

// Define other optional compnents to be included or not...
#define OPT_ESPEAK
#define OPT_PCMSOUND

//===================================================================
// Debug Options
#ifdef DBGSerial
#define OPT_TERMINAL_MONITOR
#define OPT_FIND_SERVO_OFFSETS                    // Only useful if terminal monitor is enabled
#endif

//#define DEBUG_IOPINS
#ifdef DEBUG_IOPINS
#define DebugToggle(pin)  {digitalWrite(pin, !digitalRead(pin));}
#define DebugWrite(pin, state) {digitalWrite(pin, state);}
#else
#define DebugToggle(pin)  {;}
#define DebugWrite(pin, state) {;}
#endif

#define USE_SSC32
#define OPT_GPPLAYER
//#define	cSSC_BINARYMODE	1			// Define if your SSC-32 card supports binary mode.

//==================================================================================================================================
//==================================================================================================================================
//==================================================================================================================================
//  PhantomX
//==================================================================================================================================
//[SERIAL CONNECTIONS]

//====================================================================
// XBEE on non mega???
#define XBeeSerial Serial
#define XBEE_BAUD        38400
//#define DISP_VOLTAGE    // User wants the Battery voltage to be displayed...
//#define DISP_VOLTAGE_TIME  1000  // how often to check and report in millis
#define XBEE_RTSCTS                               // Tell serial system we want RTS enabled...

// Define Analog pin and minimum voltage that we will allow the servos to run
//#define cVoltagePin  7      // Use our Analog pin jumper here...
#define CVADR1      1000                          // VD Resistor 1 - reduced as only need ratio... 20K and 4.66K
#define CVADR2      233                           // VD Resistor 2
//#define cTurnOffVol  1000     // 10v
//#define cTurnOnVol   1100     // 11V - optional part to say if voltage goes back up, turn it back on...

//====================================================================
#define  DEFAULT_GAIT_SPEED 50                    // Default gait speed  - Will depend on what Servos you are using...
#define  DEFAULT_SLOW_GAIT  50                    // Had a couple different speeds...

//====================================================================
// Will use Device redirection to handle talking to ssc-32...

//====================================================================
//[SSC PIN NUMBERS]
#define cRFCoxaPin      0                         //Front Right leg Hip Horizontal
#define cRFFemurPin     1                         //Front Right leg Hip Vertical
#define cRFTibiaPin     2                         //Front Right leg Knee
#define cRFTarsPin      3                         // Tar

#define cRMCoxaPin      4                         //Middle Right leg Hip Horizontal
#define cRMFemurPin     5                         //Middle Right leg Hip Vertical
#define cRMTibiaPin     6                         //Middle Right leg Knee
#define cRMTarsPin      7                         // Tar

#define cRRCoxaPin      8                         //Rear Right leg Hip Horizontal
#define cRRFemurPin     9                         //Rear Right leg Hip Vertical
#define cRRTibiaPin     10                        //Rear Right leg Knee
#define cRRTarsPin      11                        // Tar

#define cLFCoxaPin      16                        //Front Left leg Hip Horizontal
#define cLFFemurPin     17                        //Front Left leg Hip Vertical
#define cLFTibiaPin     18                        //Front Left leg Knee
#define cLFTarsPin      19                        // Tar

#define cLMCoxaPin      20                        //Middle Left leg Hip Horizontal
#define cLMFemurPin     21                        //Middle Left leg Hip Vertical
#define cLMTibiaPin     22                        //Middle Left leg Knee
#define cLMTarsPin      23                        // Tar

#define cLRCoxaPin      24                        //Rear Left leg Hip Horizontal
#define cLRFemurPin     25                        //Rear Left leg Hip Vertical
#define cLRTibiaPin     26                        //Rear Left leg Knee
#define cLRTarsPin      27                        // Tar

//--------------------------------------------------------------------
//[MIN/MAX ANGLES]
#define cRRCoxaMin1     -550                      //Mechanical limits of the Right Rear Leg
#define cRRCoxaMax1     550
#define cRRFemurMin1    -900
#define cRRFemurMax1    550
#define cRRTibiaMin1    -400
#define cRRTibiaMax1    750
#define cRRTarsMin1     -1300                     //4DOF ONLY - In theory the kinematics can reach about -160 deg
#define cRRTarsMax1 500                           //4DOF ONLY - The kinematics will never exceed 23 deg though..

#define cRMCoxaMin1     -550                      //Mechanical limits of the Right Middle Leg
#define cRMCoxaMax1     550
#define cRMFemurMin1    -900
#define cRMFemurMax1    550
#define cRMTibiaMin1    -400
#define cRMTibiaMax1    750
#define cRMTarsMin1     -1300                     //4DOF ONLY - In theory the kinematics can reach about -160 deg
#define cRMTarsMax1 500                           //4DOF ONLY - The kinematics will never exceed 23 deg though..

#define cRFCoxaMin1     -550                      //Mechanical limits of the Right Front Leg
#define cRFCoxaMax1     550
#define cRFFemurMin1    -900
#define cRFFemurMax1    550
#define cRFTibiaMin1    -400
#define cRFTibiaMax1    750
#define cRFTarsMin1     -1300                     //4DOF ONLY - In theory the kinematics can reach about -160 deg
#define cRFTarsMax1 500                           //4DOF ONLY - The kinematics will never exceed 23 deg though..

#define cLRCoxaMin1     -550                      //Mechanical limits of the Left Rear Leg
#define cLRCoxaMax1     550
#define cLRFemurMin1    -900
#define cLRFemurMax1    550
#define cLRTibiaMin1    -400
#define cLRTibiaMax1    750
#define cLRTarsMin1     -1300                     //4DOF ONLY - In theory the kinematics can reach about -160 deg
#define cLRTarsMax1 500                           //4DOF ONLY - The kinematics will never exceed 23 deg though..

#define cLMCoxaMin1     -550                      //Mechanical limits of the Left Middle Leg
#define cLMCoxaMax1     550
#define cLMFemurMin1    -900
#define cLMFemurMax1    550
#define cLMTibiaMin1    -400
#define cLMTibiaMax1    750
#define cLMTarsMin1     -1300                     //4DOF ONLY - In theory the kinematics can reach about -160 deg
#define cLMTarsMax1 500                           //4DOF ONLY - The kinematics will never exceed 23 deg though..

#define cLFCoxaMin1     -550                      //Mechanical limits of the Left Front Leg
#define cLFCoxaMax1     550
#define cLFFemurMin1    -900
#define cLFFemurMax1    550
#define cLFTibiaMin1    -400
#define cLFTibiaMax1    750
#define cLFTarsMin1     -1300                     //4DOF ONLY - In theory the kinematics can reach about -160 deg
#define cLFTarsMax1 500                           //4DOF ONLY - The kinematics will never exceed 23 deg though..

//--------------------------------------------------------------------
//[LEG DIMENSIONS]
//Universal dimensions for each leg in mm
#define cXXCoxaLength     29                      // This is for TH3-R legs
#define cXXFemurLength    76
#define cXXTibiaLength    104
#define cXXTarsLength     85                      // 4DOF only...

#define cRRCoxaLength     cXXCoxaLength           //Right Rear leg
#define cRRFemurLength    cXXFemurLength
#define cRRTibiaLength    cXXTibiaLength
#define cRRTarsLength     cXXTarsLength           //4DOF ONLY

#define cRMCoxaLength     cXXCoxaLength           //Right middle leg
#define cRMFemurLength    cXXFemurLength
#define cRMTibiaLength    cXXTibiaLength
#define cRMTarsLength     cXXTarsLength           //4DOF ONLY

#define cRFCoxaLength     cXXCoxaLength           //Rigth front leg
#define cRFFemurLength    cXXFemurLength
#define cRFTibiaLength    cXXTibiaLength
#define cRFTarsLength     cXXTarsLength           //4DOF ONLY

#define cLRCoxaLength     cXXCoxaLength           //Left Rear leg
#define cLRFemurLength    cXXFemurLength
#define cLRTibiaLength    cXXTibiaLength
#define cLRTarsLength     cXXTarsLength           //4DOF ONLY

#define cLMCoxaLength     cXXCoxaLength           //Left middle leg
#define cLMFemurLength    cXXFemurLength
#define cLMTibiaLength    cXXTibiaLength
#define cLMTarsLength     cXXTarsLength           //4DOF ONLY

#define cLFCoxaLength     cXXCoxaLength           //Left front leg
#define cLFFemurLength    cXXFemurLength
#define cLFTibiaLength    cXXTibiaLength
#define cLFTarsLength     cXXTarsLength           //4DOF ONLY

//--------------------------------------------------------------------
//[BODY DIMENSIONS]
#define cRRCoxaAngle1   -450                      //Default Coxa setup angle, decimals = 1
#define cRMCoxaAngle1    0                        //Default Coxa setup angle, decimals = 1
#define cRFCoxaAngle1    450                      //Default Coxa setup angle, decimals = 1
#define cLRCoxaAngle1    -450                     //Default Coxa setup angle, decimals = 1
#define cLMCoxaAngle1    0                        //Default Coxa setup angle, decimals = 1
#define cLFCoxaAngle1    450                      //Default Coxa setup angle, decimals = 1

#define cRROffsetX      -53                       //Distance X from center of the body to the Right Rear coxa
#define cRROffsetZ      102                       //Distance Z from center of the body to the Right Rear coxa
#define cRMOffsetX      -72                       //Distance X from center of the body to the Right Middle coxa
#define cRMOffsetZ      0                         //Distance Z from center of the body to the Right Middle coxa
#define cRFOffsetX      -60                       //Distance X from center of the body to the Right Front coxa
#define cRFOffsetZ      -102                      //Distance Z from center of the body to the Right Front coxa

#define cLROffsetX      53                        //Distance X from center of the body to the Left Rear coxa
#define cLROffsetZ      102                       //Distance Z from center of the body to the Left Rear coxa
#define cLMOffsetX      72                        //Distance X from center of the body to the Left Middle coxa
#define cLMOffsetZ      0                         //Distance Z from center of the body to the Left Middle coxa
#define cLFOffsetX      60                        //Distance X from center of the body to the Left Front coxa
#define cLFOffsetZ      -102                      //Distance Z from center of the body to the Left Front coxa

//--------------------------------------------------------------------
//[START POSITIONS FEET]
//--------------------------------------------------------------------
//[START POSITIONS FEET]
#define cHexInitXZ   105
#define CHexInitXZCos45  74                       // COS(45) = .7071
#define CHexInitXZSin45  74                       // sin(45) = .7071
#define CHexInitY    26

// Lets try some multi leg positions depending on height settings.
#define CNT_HEX_INITS 3
#define MAX_BODY_Y  90

#define PROGMEM                                   // bugbug:: not in this context, need to find higher level definition...

#ifdef DEFINE_HEX_GLOBALS
const unsigned char g_abHexIntXZ[] PROGMEM =
{
    cHexInitXZ, 99, 86
};
const unsigned char g_abHexMaxBodyY[] PROGMEM =
{
    20, 50, MAX_BODY_Y
};
#else
extern const unsigned char g_abHexIntXZ[] PROGMEM;
extern const unsigned char g_abHexMaxBodyY[] PROGMEM;
#endif

#define cRRInitPosX     CHexInitXZCos45           //Start positions of the Right Rear leg
#define cRRInitPosY     CHexInitY
#define cRRInitPosZ     CHexInitXZSin45

#define cRMInitPosX     cHexInitXZ                //Start positions of the Right Middle leg
#define cRMInitPosY     CHexInitY
#define cRMInitPosZ     0

#define cRFInitPosX     CHexInitXZCos45           //Start positions of the Right Front leg
#define cRFInitPosY     CHexInitY
#define cRFInitPosZ     -CHexInitXZSin45

#define cLRInitPosX     CHexInitXZCos45           //Start positions of the Left Rear leg
#define cLRInitPosY     CHexInitY
#define cLRInitPosZ     CHexInitXZSin45

#define cLMInitPosX     cHexInitXZ                //Start positions of the Left Middle leg
#define cLMInitPosY     CHexInitY
#define cLMInitPosZ     0

#define cLFInitPosX     CHexInitXZCos45           //Start positions of the Left Front leg
#define cLFInitPosY     CHexInitY
#define cLFInitPosZ     -CHexInitXZSin45
//--------------------------------------------------------------------
//[Tars factors used in formula to calc Tarsus angle relative to the ground]
#define cTarsConst  720                           //4DOF ONLY
#define cTarsMulti  2                             //4DOF ONLY
#define cTarsFactorA    70                        //4DOF ONLY
#define cTarsFactorB    60                        //4DOF ONLY
#define cTarsFactorC    50                        //4DOF ONLY
#endif                                            // HEX_CFG_H
