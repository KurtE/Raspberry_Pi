//====================================================================
//Project Lynxmotion Phoenix
//
// Linux Joystick device (/dev/input/js0).  currently has support built
// in for Sony Playstation DS3 and DS4
//
//Walk method 1:
//- Left StickWalk/Strafe
//- Right StickRotate
//
//Walk method 2:
//- Left StickDisable
//- Right StickWalk/Rotate
//
//
// Quick and Dirty description of controls... WIP
// Note: Converting from Arbotix Commander code version back to having the
// buttons work like the original PS2 did.
//
// Share (Start)  - Turns the Robot on or off
// L1 - Toggle modes?
//
// Otions/Select (Change walk gait, Change Leg in Single Leg, Change GP sequence)
// L3 - Toggle Debug on and off
// R2 - Toggle walk method...  Run Sequence in GP mode
// R3 - Walk method (Not done yet) - (PS2 R3)
// Square - Ballance mode on and off
// Triangle - Stand/Sit 
// L1+Right Joy UP/DOWN - Body up/down - (PS2 Dpad Up/Down)
// L1+Right Joy Left/Right - Speed higher/lower - (PS2 DPad left/right)
// R1 - Cycle through options of Normal walk/Double Height/Double Travel)
// Left Top(S8) - Cycle through modes (Walk, Translate, Rotate, Single Leg) (PS2: Circle, X, L1, L2)

// Note: Left some descriptions of PS2 stuff, especially parts still left to Map/Implement.
//
//[Walk Controls]
//- selectSwitch gaits
//- Left Stick(Walk mode 1) Walk/Strafe
// (Walk mode 2) Disable
//- Right Stick(Walk mode 1) Rotate,
//(Walk mode 2) Walk/Rotate
//- R1Toggle Double gait travel speed
//- R2Toggle Double gait travel length
//
//[Shift Controls]
//- Left StickShift body X/Z
//- Right StickShift body Y and rotate body Y
//
//[Rotate Controls]
//- Left StickRotate body X/Z
//- Right StickRotate body Y
//
//[Single leg Controls] - Need to check...
//- selectSwitch legs
//- Left StickMove Leg X/Z (relative)
//- Right StickMove Leg Y (absolute)
//- R2Hold/release leg position
//
//[GP Player Controls] - How to do sequences???
//- selectSwitch Sequences
//- R2Start Sequence
//
//====================================================================
#include "Hex_Cfg.h"
#include "Phoenix.h"
#include "JoystickController.h"

#ifdef OPT_ESPEAK
#include "speak.h"
#endif

 
 
//[CONSTANTS]
enum
{
    WALKMODE=0, TRANSLATEMODE, ROTATEMODE, MODECNT
};
enum
{
    NORM_NORM=0, NORM_LONG, HIGH_NORM, HIGH_LONG
};

#ifndef MAX_BODY_Y
#define MAX_BODY_Y 100
#endif


#ifdef OPT_ESPEAK
#define SpeakStr(psz) Speak.Speak((psz), true)
extern "C"
{
    // Move the Gait Names to program space...
    const char s_sGN1[] PROGMEM = "Ripple 12";
    const char s_sGN2[] PROGMEM = "Tripod 8";
    const char s_sGN3[] PROGMEM = "Tripple 12";
    const char s_sGN4[] PROGMEM = "Tripple 16";
    const char s_sGN5[] PROGMEM = "Wave 24";
    const char s_sGN6[] PROGMEM = "Tripod 6";
    PGM_P s_asGateNames[] PROGMEM =
    {
        s_sGN1, s_sGN2, s_sGN3, s_sGN4, s_sGN5, s_sGN6
    };
}


#else
#define SpeakStr(PSZ)
#endif

#define cTravelDeadZone 4                         //The deadzone for the analog input from the remote

#define ARBOTIX_TO  300000                          // if we don't get a valid message in this number of mills turn off

//=============================================================================
// Global - Local to this file only...
//=============================================================================
LinuxJoy ljoy = LinuxJoy();     // create our joystick object

unsigned long g_ulLastMsgTime = (unsigned long)-1;
short  g_sGPSMController;                         // What GPSM value have we calculated. 0xff - Not used yet
boolean g_fDynamicLegXZLength = false;  // Has the user dynamically adjusted the Leg XZ init pos (width)

#define JoystickInputController InputController
// Define an instance of the Input Controller...
InputController  g_InputController;               // Our Input controller

static short   g_BodyYOffset;
static short   g_BodyYShift;
static byte    ControlMode;
static byte    HeightSpeedMode;
//static bool  DoubleHeightOn;
static bool    DoubleTravelOn;
static bool    WalkMethod;
byte           GPSeq;                             //Number of the sequence


// some external or forward function references.
extern void ControllerTurnRobotOff(void);
#define szDevice "/dev/input/js0"
//==============================================================================
// This is The function that is called by the Main program to initialize
//the input controller, which in this case is a generic Linux Joystick object
// For a few of them we will try to detect which one it is and map the buttons
// and axes...
//==============================================================================
void JoystickInputController::Init(void)
{
    g_BodyYOffset = 0;
    g_BodyYShift = 0;
    ljoy.begin(szDevice);
    GPSeq = 0;                                    // init to something...

    ControlMode = WALKMODE;
    HeightSpeedMode = NORM_NORM;
    DoubleTravelOn = false;
    WalkMethod = false;

    SpeakStr("Init");

}

//==============================================================================
// Cleanup - Allows us to cleanup at the end
//==============================================================================
void JoystickInputController::Cleanup(void)
{
    ljoy.end();
}

//==============================================================================
// This function is called by the main code to tell us when it is about to
// do a lot of bit-bang outputs and it would like us to minimize any interrupts
// that we do while it is active...
//==============================================================================
void JoystickInputController::AllowControllerInterrupts(boolean fAllow)
{
    // We don't need to do anything...
}


//==============================================================================
// This is The main code to input function to read inputs from the Joystick and then
//process any commands.
//==============================================================================
void JoystickInputController::ControlInput(void)
{
    // See if we have a new command available...
    if(ljoy.readMsgs() > 0)
    {
       boolean fAdjustLegPositions = false;
        short sLegInitXZAdjust = 0;
        short sLegInitAngleAdjust = 0;

        // See if this is the first message we have received
        if (g_ulLastMsgTime == (unsigned long)-1)
        {
            // First valid message see if we can figure out what it is from...
            printf("Firt JS0 even: %d axes %d buttons Name: %s\n", ljoy.joystickAxisCount(), 
                    ljoy.joystickButtonCount(),
                    ljoy.JoystickName());
            if ((ljoy.joystickAxisCount() == 27) && (ljoy.joystickButtonCount() == 19))
            {
                printf("PS3!\n");
            }
            else if ((ljoy.joystickAxisCount() == 8) && (ljoy.joystickButtonCount() == 14))
            {
                printf("DS4!\n");
            }
        }
        // Save message time so we may detect timeouts.
        g_ulLastMsgTime = millis();

        // We have a message see if we have turned the robot on or not...
        if (ljoy.buttonPressed(JOYSTICK_BUTTONS::START_SHARE)) 
        {
            if (!g_InControlState.fRobotOn)
            {
                // Turn it on
                g_InControlState.fRobotOn = true;
                fAdjustLegPositions = true;
                printf("Turn Robot on: %d %d : %d %d\n", 
                        ljoy.axis(JOYSTICK_AXES::LX), ljoy.axis(JOYSTICK_AXES::LY),
                        ljoy.axis(JOYSTICK_AXES::RX), ljoy.axis(JOYSTICK_AXES::RY));

            }
            else
            {
                ControllerTurnRobotOff();
            }
        }    
        
        // Few of the buttons we may process even when off...
        if (ljoy.buttonPressed(JOYSTICK_BUTTONS::L3))
            g_fDebugOutput = !g_fDebugOutput;   // toggle debug on and off
            

        // If robot is not on, lets bail from here...
        if ( !g_InControlState.fRobotOn)
            return;
        
        // In some cases we update values as to keep other places to use the values
        // So we create a local copy we use through this function.
        int axes_rx = ljoy.axis(JOYSTICK_AXES::RX);
        int axes_ry = ljoy.axis(JOYSTICK_AXES::RY);
        int axes_lx = ljoy.axis(JOYSTICK_AXES::LX);
        int axes_ly = ljoy.axis(JOYSTICK_AXES::LY);
        
        // Experimenting with trying to detect when IDLE.  maybe set a state of
        // of no button pressed and all joysticks are in the DEADBAND area...
        g_InControlState.fControllerInUse = ljoy.buttons()
            || (abs((axes_lx/256)) >= cTravelDeadZone)
            || (abs((axes_ly/256)) >= cTravelDeadZone)
            || (abs((axes_rx/256)) >= cTravelDeadZone)
            || (abs((axes_ry/256)) >= cTravelDeadZone);
        // [SWITCH MODES]

        // Cycle through modes...
        // May break this up, like PS2...
        if (ljoy.buttonPressed(JOYSTICK_BUTTONS::R1))
        {
            if (++ControlMode >= MODECNT)
            {
                ControlMode = WALKMODE;           // cycled back around...
                MSound( 2, 50, 2000, 50, 3000);
            }
            else
            {
                MSound( 1, 50, 2000);
            }
        }
        // Hack make easy to get back to standard mode...
        if (ljoy.buttonPressed(JOYSTICK_BUTTONS::PS))
        {
            ControlMode = WALKMODE;           // cycled back around...
            MSound( 2, 50, 2000, 50, 3000);
        }
        

        //[Common functions]
        //Switch Balance mode on/off
        if (ljoy.buttonPressed(JOYSTICK_BUTTONS::SQUARE))
        {
            g_InControlState.BalanceMode = !g_InControlState.BalanceMode;
            if (g_InControlState.BalanceMode)
            {
                MSound( 1, 250, 1500);
            }
            else
            {
                MSound( 2, 100, 2000, 50, 4000);
            }
        }

        //Stand up, sit down
        if (ljoy.buttonPressed(JOYSTICK_BUTTONS::TRI))
        {
            if (g_BodyYOffset>0)
                g_BodyYOffset = 0;
            else
                g_BodyYOffset = 35;
            
            fAdjustLegPositions = true;
            g_fDynamicLegXZLength = false;
        }

        // We will use L1 with the Right joystick to control both body offset as well as Speed...
        // We move each pass through this by a percentage of how far we are from center in each direction
        // We get feedback with height by seeing the robot move up and down.  For Speed, I put in sounds
        // which give an idea, but only for those whoes robot has a speaker
        if (ljoy.button(JOYSTICK_BUTTONS::L1))
        {
            // raise or lower the robot on the joystick up /down
            int delta = (axes_ry/256)/25;   
            if (delta) {
                g_BodyYOffset = max(min(g_BodyYOffset + delta, MAX_BODY_Y), 0);
                fAdjustLegPositions = true;
            }

            // Likewise for Speed control
            int dspeed = (axes_rx/256) / 16;      //
            if ((dspeed < 0) && g_InControlState.SpeedControl)
            {
                if ((word)(-dspeed) <  g_InControlState.SpeedControl)
                    g_InControlState.SpeedControl += dspeed;
                else
                    g_InControlState.SpeedControl = 0;
                MSound( 1, 50, 1000+g_InControlState.SpeedControl);
            }
            if ((dspeed > 0) && (g_InControlState.SpeedControl < 2000))
            {
                g_InControlState.SpeedControl += dspeed;
                if (g_InControlState.SpeedControl > 2000)
                    g_InControlState.SpeedControl = 2000;
                MSound( 1, 50, 1000+g_InControlState.SpeedControl);
            }

            // Likewise we use the left Joystick to control Postion of feet
            sLegInitXZAdjust = axes_lx/(256*10);        // play with this.
#ifdef ADJUSTABLE_LEG_ANGLES
            sLegInitAngleAdjust = axes_ly/(256*8);
#endif
            // Make sure other areas of code below does not use some of the joystick values
            axes_lx = 0;
            axes_ly = 0;
            axes_rx = 0;                    // don't walk when adjusting the speed here...
           
        }

        //[Walk functions]
        if (ControlMode == WALKMODE)
        {
            //Switch gates
            if ((ljoy.buttonPressed(JOYSTICK_BUTTONS::SELECT_OPT))
                                                  //No movement
                && abs(g_InControlState.TravelLength.x)<cTravelDeadZone
                && abs(g_InControlState.TravelLength.z)<cTravelDeadZone
                && abs(g_InControlState.TravelLength.y*2)<cTravelDeadZone  )
            {
                                                  // Go to the next gait...
                g_InControlState.GaitType = g_InControlState.GaitType+1;
                                                  // Make sure we did not exceed number of gaits...
                if (g_InControlState.GaitType<NUM_GAITS)
                {
#ifndef OPT_ESPEAK
                    MSound( 1, 50, 2000);
#endif
                }
                else
                {
#ifdef OPT_ESPEAK
                    MSound (2, 50, 2000, 50, 2250);
#endif
                    g_InControlState.GaitType = 0;
                }
#ifdef OPT_ESPEAK
                SpeakStr(s_asGateNames[g_InControlState.GaitType]);
#endif
                GaitSelect();
            }

            //Double leg lift height
            if (ljoy.buttonPressed(JOYSTICK_BUTTONS::R2))
            {
                MSound( 1, 50, 2000);
                                                  // wrap around mode
                HeightSpeedMode = (HeightSpeedMode + 1) & 0x3;
                DoubleTravelOn = HeightSpeedMode & 0x1;
                if ( HeightSpeedMode & 0x2)
                    g_InControlState.LegLiftHeight = 80;
                else
                    g_InControlState.LegLiftHeight = 50;
            }

            // Switch between Walk method 1 && Walk method 2
            if (ljoy.buttonPressed(JOYSTICK_BUTTONS::R3))
            {
                MSound (1, 50, 2000);
                WalkMethod = !WalkMethod;
            }

            //Walking
            if (WalkMethod)                       //(Walk Methode)
                                                  //Right Stick Up/Down
                g_InControlState.TravelLength.z = ((axes_ry/256));

            else
            {
                g_InControlState.TravelLength.x = -(axes_lx/256);
                g_InControlState.TravelLength.z = (axes_ly/256);
            }

            if (!DoubleTravelOn)                  //(Double travel length)
            {
                g_InControlState.TravelLength.x = g_InControlState.TravelLength.x/2;
                g_InControlState.TravelLength.z = g_InControlState.TravelLength.z/2;
            }

                                                  //Right Stick Left/Right
            g_InControlState.TravelLength.y = -((axes_rx/256))/4;
        }

        //[Translate functions]
        g_BodyYShift = 0;
        if (ControlMode == TRANSLATEMODE)
        {
            g_InControlState.BodyPos.x =  SmoothControl((((axes_lx/256))*2/3), g_InControlState.BodyPos.x, SmDiv);
            g_InControlState.BodyPos.z =  SmoothControl((((axes_ly/256))*2/3), g_InControlState.BodyPos.z, SmDiv);
            g_InControlState.BodyRot1.y = SmoothControl((((axes_rx/256))*2), g_InControlState.BodyRot1.y, SmDiv);
            g_BodyYShift = (-((axes_ry/256))/2);
        }

        //[Rotate functions]
        if (ControlMode == ROTATEMODE)
        {
            g_InControlState.BodyRot1.x = ((axes_ly/256));
            g_InControlState.BodyRot1.y = ((axes_rx/256))*2;
            g_InControlState.BodyRot1.z = ((axes_lx/256));
            g_BodyYShift = (-((axes_ry/256))/2);
        }

        //Calculate walking time delay
        g_InControlState.InputTimeDelay = 128 - max(max(abs((axes_lx/256)), abs((axes_ly/256))), abs((axes_rx/256)));

        //Calculate g_InControlState.BodyPos.y
        g_InControlState.BodyPos.y = max(g_BodyYOffset + g_BodyYShift,  0);
        
        // Add Dynamic leg position and body height code
        if (sLegInitXZAdjust || sLegInitAngleAdjust)
        {
            // User asked for manual leg adjustment - only do when we have finished any previous adjustment
            if (!g_InControlState.ForceGaitStepCnt) 
            {
                if (sLegInitXZAdjust)
                    g_fDynamicLegXZLength = true;

                sLegInitXZAdjust += GetLegsXZLength();  // Add on current length to our adjustment...

                // Handle maybe change angles...
#ifdef ADJUSTABLE_LEG_ANGLES                
                if (sLegInitAngleAdjust) 
                    RotateLegInitAngles(sLegInitAngleAdjust);
#endif
                // Give system time to process previous calls
                AdjustLegPositions(sLegInitXZAdjust);
            }
        }    

        if (fAdjustLegPositions && !g_fDynamicLegXZLength)
            AdjustLegPositionsToBodyHeight();    // Put main workings into main program file
    }
    else
    {
        // We did not receive a valid packet.  check for a timeout to see if we should turn robot off...
        if (g_InControlState.fRobotOn)
        {
            if (! ljoy.connected())  // See if controller is still connected?
            {
                g_InControlState.fControllerInUse = true;	// make sure bypass is not used.
                ControllerTurnRobotOff();
            }
        }
    }
}


//==============================================================================
// ControllerTurnRobotOff - code used couple of places so save a little room...
//==============================================================================
void ControllerTurnRobotOff(void)
{
    //Turn off
    printf("Turn Robot Off\n");
    g_InControlState.BodyPos.x = 0;
    g_InControlState.BodyPos.y = 0;
    g_InControlState.BodyPos.z = 0;
    g_InControlState.BodyRot1.x = 0;
    g_InControlState.BodyRot1.y = 0;
    g_InControlState.BodyRot1.z = 0;
    g_InControlState.TravelLength.x = 0;
    g_InControlState.TravelLength.z = 0;
    g_InControlState.TravelLength.y = 0;
    g_BodyYOffset = 0;
    g_BodyYShift = 0;
    g_InControlState.fRobotOn = 0;
}
