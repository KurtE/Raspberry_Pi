//====================================================================
//Project Lynxmotion Phoenix
//Description:
//    This is the hardware configuration file for the Hex Robot.
//    This Header file is specific for a Lynxmotion Robot like CHR
//    That is using 4DOF T-Hex legs
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


#define DBGSerial         DBGSerialWrapper

// Define other optional compnents to be included or not...
// Speach options
#define OPT_ESPEAK

// Else we can try to find by attributes
// If VoiceName is defined it is used can be found by using espeak --voices command
//#define ESPEAK_VOICENAME "en-french"

// If language must be defined if you also wish to set gender or age...
#define ESPEAK_LANGUAGE	"en-us"
#define ESPEAK_GENDER	2		// 0=dont care, 1=male, 2=female
#define ESPEAK_AGE		0		//0=dont care

// Use PCM Sound to play notes
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
#define	cSSC_BINARYMODE	1			// Define if your SSC-32 card supports binary mode.


//====================================================================
// XBEE 
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
#define USE_SSC32
//#define	cSSC_BINARYMODE	1			// Define if your SSC-32 card supports binary mode.
#define c4DOF    

//[SSC PIN NUMBERS]
#define cRRCoxaPin      0   //Rear Right leg Hip Horizontal
#define cRRFemurPin     1   //Rear Right leg Hip Vertical
#define cRRTibiaPin     2   //Rear Right leg Knee
#define cRRTarsPin      3   // Tar

#define cRMCoxaPin      4   //Middle Right leg Hip Horizontal
#define cRMFemurPin     5   //Middle Right leg Hip Vertical
#define cRMTibiaPin     6   //Middle Right leg Knee
#define cRMTarsPin      7   // Tar

#define cRFCoxaPin      8   //Front Right leg Hip Horizontal
#define cRFFemurPin     9   //Front Right leg Hip Vertical
#define cRFTibiaPin     10   //Front Right leg Knee
#define cRFTarsPin      11   // Tar

#define cLRCoxaPin      16   //Rear Left leg Hip Horizontal
#define cLRFemurPin     17   //Rear Left leg Hip Vertical
#define cLRTibiaPin     18   //Rear Left leg Knee
#define cLRTarsPin      19   // Tar

#define cLMCoxaPin      20   //Middle Left leg Hip Horizontal
#define cLMFemurPin     21   //Middle Left leg Hip Vertical
#define cLMTibiaPin     22   //Middle Left leg Knee
#define cLMTarsPin      23   // Tar

#define cLFCoxaPin      24   //Front Left leg Hip Horizontal
#define cLFFemurPin     25   //Front Left leg Hip Vertical
#define cLFTibiaPin     26   //Front Left leg Knee
#define cLFTarsPin      27   // Tar

//--------------------------------------------------------------------
//[MIN/MAX ANGLES]
#define cRRCoxaMin     -65      //Mechanical limits of the Right Rear Leg
#define cRRCoxaMax     65
#define cRRFemurMin    -105
#define cRRFemurMax    75
#define cRRTibiaMin    -53
#define cRRTibiaMax    90
#define cRRTarsMin     -130	//4DOF ONLY - In theory the kinematics can reach about -160 deg
#define cRRTarsMax	    50	    //4DOF ONLY - The kinematics will never exceed 23 deg though..

#define cRMCoxaMin     -65      //Mechanical limits of the Right Middle Leg
#define cRMCoxaMax      65
#define cRMFemurMin    -105
#define cRMFemurMax    75
#define cRMTibiaMin    -53
#define cRMTibiaMax    90
#define cRMTarsMin     -130	//4DOF ONLY - In theory the kinematics can reach about -160 deg
#define cRMTarsMax	    50	//4DOF ONLY - The kinematics will never exceed 23 deg though..

#define cRFCoxaMin     -65      //Mechanical limits of the Right Front Leg
#define cRFCoxaMax     65
#define cRFFemurMin    -105
#define cRFFemurMax    75
#define cRFTibiaMin    -53
#define cRFTibiaMax    90
#define cRFTarsMin     -130	//4DOF ONLY - In theory the kinematics can reach about -160 deg
#define cRFTarsMax	    50	//4DOF ONLY - The kinematics will never exceed 23 deg though..

#define cLRCoxaMin     -65      //Mechanical limits of the Left Rear Leg
#define cLRCoxaMax     65
#define cLRFemurMin    -105
#define cLRFemurMax    75
#define cLRTibiaMin    -53
#define cLRTibiaMax    90
#define cLRTarsMin     -130	//4DOF ONLY - In theory the kinematics can reach about -160 deg
#define cLRTarsMax	    50	//4DOF ONLY - The kinematics will never exceed 23 deg though..

#define cLMCoxaMin     -65      //Mechanical limits of the Left Middle Leg
#define cLMCoxaMax     65
#define cLMFemurMin    -105
#define cLMFemurMax    75
#define cLMTibiaMin    -53
#define cLMTibiaMax    90
#define cLMTarsMin     -130	//4DOF ONLY - In theory the kinematics can reach about -160 deg
#define cLMTarsMax	    50	//4DOF ONLY - The kinematics will never exceed 23 deg though..

#define cLFCoxaMin     -65      //Mechanical limits of the Left Front Leg
#define cLFCoxaMax      65
#define cLFFemurMin    -105
#define cLFFemurMax    75
#define cLFTibiaMin    -53
#define cLFTibiaMax    90
#define cLFTarsMin     -130	//4DOF ONLY - In theory the kinematics can reach about -160 deg
#define cLFTarsMax	    50	//4DOF ONLY - The kinematics will never exceed 23 deg though..

//--------------------------------------------------------------------
// Define which servos should have their values inverted before passing to servo driver.
#define cRRCoxaInv 1 
#define cRMCoxaInv 1 
#define cRFCoxaInv 1 
#define cLRCoxaInv 0 
#define cLMCoxaInv 0 
#define cLFCoxaInv 0 

#define cRRFemurInv 1 
#define cRMFemurInv 1 
#define cRFFemurInv 1 
#define cLRFemurInv 0 
#define cLMFemurInv 0 
#define cLFFemurInv 0 

#define cRRTibiaInv 1 
#define cRMTibiaInv 1 
#define cRFTibiaInv 1 
#define cLRTibiaInv 0 
#define cLMTibiaInv 0 
#define cLFTibiaInv 0 

#define cRRTarsInv 1 
#define cRMTarsInv 1 
#define cRFTarsInv 1 
#define cLRTarsInv 0 
#define cLMTarsInv 0 
#define cLFTarsInv 0 

//--------------------------------------------------------------------


//--------------------------------------------------------------------
//[Joint offsets]
// This allows us to calibrate servos to some fixed position, and then adjust them by moving theim
// one or more servo horn clicks.  This requires us to adjust the value for those servos by 15 degrees
// per click.  This is used with the T-Hex 4DOF legs
//First calibrate the servos in the 0 deg position using the SSC-32 reg offsets, then:
#define cFemurHornOffset1	150
#define cTarsHornOffset1	150	


//--------------------------------------------------------------------
//[LEG DIMENSIONS]
//Universal dimensions for each leg in mm
#define cXXCoxaLength     29
#define cXXFemurLength    75
#define cXXTibiaLength    71
#define cXXTarsLength     85    // 4DOF only...

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
//[BODY Leg Angles andd offsets]
#define cRRCoxaAngle   -60     //Default Coxa setup angle, decimals = 1
#define cRMCoxaAngle    0      //Default Coxa setup angle, decimals = 1
#define cRFCoxaAngle    60     //Default Coxa setup angle, decimals = 1
#define cLRCoxaAngle    -60    //Default Coxa setup angle, decimals = 1
#define cLMCoxaAngle    0      //Default Coxa setup angle, decimals = 1
#define cLFCoxaAngle    60     //Default Coxa setup angle, decimals = 1

#define cRROffsetX      -69     //Distance X from center of the body to the Right Rear coxa
#define cRROffsetZ      119     //Distance Z from center of the body to the Right Rear coxa
#define cRMOffsetX      -138    //Distance X from center of the body to the Right Middle coxa
#define cRMOffsetZ      0       //Distance Z from center of the body to the Right Middle coxa
#define cRFOffsetX      -69     //Distance X from center of the body to the Right Front coxa
#define cRFOffsetZ      -119    //Distance Z from center of the body to the Right Front coxa

#define cLROffsetX      69      //Distance X from center of the body to the Left Rear coxa
#define cLROffsetZ      119     //Distance Z from center of the body to the Left Rear coxa
#define cLMOffsetX      138     //Distance X from center of the body to the Left Middle coxa
#define cLMOffsetZ      0       //Distance Z from center of the body to the Left Middle coxa
#define cLFOffsetX      69      //Distance X from center of the body to the Left Front coxa
#define cLFOffsetZ      -119    //Distance Z from center of the body to the Left Front coxa

//--------------------------------------------------------------------
//[START POSITIONS FEET]
//--------------------------------------------------------------------
//[START POSITIONS FEET]
#define cHexInitXZ	 80
#define CHexInitXZCos60  40        // COS(60) = .5
#define CHexInitXZSin60  69    // sin(60) = .866
#define CHexInitY	 30


#define cRRInitPosX     CHexInitXZCos60      //Start positions of the Right Rear leg
#define cRRInitPosY     CHexInitY
#define cRRInitPosZ     CHexInitXZSin60

#define cRMInitPosX     cHexInitXZ      //Start positions of the Right Middle leg
#define cRMInitPosY     CHexInitY
#define cRMInitPosZ     0

#define cRFInitPosX     CHexInitXZCos60      //Start positions of the Right Front leg
#define cRFInitPosY     CHexInitY
#define cRFInitPosZ     -CHexInitXZSin60

#define cLRInitPosX     CHexInitXZCos60      //Start positions of the Left Rear leg
#define cLRInitPosY     CHexInitY
#define cLRInitPosZ     CHexInitXZSin60

#define cLMInitPosX     cHexInitXZ      //Start positions of the Left Middle leg
#define cLMInitPosY     CHexInitY
#define cLMInitPosZ     0

#define cLFInitPosX     CHexInitXZCos60      //Start positions of the Left Front leg
#define cLFInitPosY     CHexInitY
#define cLFInitPosZ     -CHexInitXZSin60

//--------------------------------------------------------------------
// Lets try some multi leg positions depending on height settings.
#define CNT_HEX_INITS 2
#define MAX_BODY_Y  150

#ifdef DEFINE_HEX_GLOBALS
const unsigned char g_abHexIntXZ[] =
{
    cHexInitXZ, 99, 86
};
const unsigned char g_abHexMaxBodyY[] =
{
    20, 50, MAX_BODY_Y
};
#else
extern const unsigned char g_abHexIntXZ[];
extern const unsigned char g_abHexMaxBodyY[];
#endif

//--------------------------------------------------------------------
//[Tars factors used in formula to calc Tarsus angle relative to the ground]
#define cTarsConst     720                        //4DOF ONLY
#define cTarsMulti       2                        //4DOF ONLY
#define cTarsFactorA    70                        //4DOF ONLY
#define cTarsFactorB    60                        //4DOF ONLY
#define cTarsFactorC    50                        //4DOF ONLY
#endif                                            // HEX_CFG_H
