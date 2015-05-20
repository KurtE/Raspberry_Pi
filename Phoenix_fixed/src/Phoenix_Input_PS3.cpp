#define DEBUG
//====================================================================
//Project Lynxmotion Phoenix
//Hardware setup: PS3 version
//
// This is a merge of the PS2 code for Arduino with PS3 support from
// Trossen Robotics HR-OS1 framework...
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
//PS2 CONTROLS:
//[Common Controls]
//- StartTurn on/off the bot
//- L1Toggle Shift mode
//- L2Toggle Rotate mode
//- CircleToggle Single leg mode
//   - Square        Toggle Balance mode
//- TriangleMove body to 35 mm from the ground (walk pos) 
//and back to the ground
//- D-Pad upBody up 10 mm
//- D-Pad downBody down 10 mm
//- D-Pad leftdecrease speed with 50mS
//- D-Pad rightincrease speed with 50mS
//
// Optional: L3 button down, Left stick can adjust leg positions...
// or if OPT_SINGLELEG is not defined may try using Circle

//
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
//[Single leg Controls]
//- selectSwitch legs
//- Left StickMove Leg X/Z (relative)
//- Right StickMove Leg Y (absolute)
//- R2Hold/release leg position
//
//[GP Player Controls]
//- selectSwitch Sequences
//- R2Start Sequence
//
//====================================================================
// [Include files]
#include "Hex_Cfg.h"
#include "Phoenix.h"
#include "PS3Controller.h"

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


#define cTravelDeadZone 4      //The deadzone for the analog input from the remote
#define  MAXPS2ERRORCNT  5     // How many times through the loop will we go before shutting off robot?

#ifndef MAX_BODY_Y
#define MAX_BODY_Y 100
#endif

//=============================================================================
// Global - Local to this file only...
//=============================================================================
unsigned long g_ulLastMsgTime;
short  g_sGPSMController;                         // What GPSM value have we calculated. 0xff - Not used yet

#ifdef USEMULTI
//==============================================================================
//
// Lets define our Sub-class of the InputControllerClass
//
//==============================================================================
class PS3InputController :
public InputController
{
    public:
        PS3InputController();               // A reall simple constructor...

        virtual void     Init(void);
        virtual void     ControlInput(void);
        virtual void     AllowControllerInterrupts(boolean fAllow);

};

PS3InputController g_CommanderController;

//==============================================================================
// Constructor. See if there is a simple way to have one or more Input
//     controllers. Maybe register at construction time
//==============================================================================
PS3InputController::PS3InputController()
{
    RegisterInputController(this);
}


#else
#define PS3InputController InputController
// Define an instance of the Input Controller...
InputController  g_InputController;               // Our Input controller
#endif

static short   g_BodyYOffset;
static short   g_BodyYShift;
static byte    ControlMode;
static byte    HeightSpeedMode;
//static bool  DoubleHeightOn;
static bool    DoubleTravelOn;
static bool    WalkMethod;
byte           GPSeq;                             //Number of the sequence


 _PS3           PS3_PREV;     //keep a copy of status from last call...
 
// some external or forward function references.
extern void PS3TurnRobotOff(void);

//==============================================================================
// This is The function that is called by the Main program to initialize
//the input controller, which in this case is the PS2 controller
//process any commands.
//==============================================================================
char szDevice[] = "/dev/ttyXBEE";

// If both PS2 and XBee are defined then we will become secondary to the xbee
void PS3InputController::Init(void)
{
    //  DBGSerial.println("Init Commander Start");
    g_BodyYOffset = 0;
    g_BodyYShift = 0;

  if (PS3Controller_Start() == 0) {
#ifdef DBGSerial
        DBGSerial.println("PS3Controller Start failed");
#endif
  }
    GPSeq = 0;                                    // init to something...

    ControlMode = WALKMODE;
    HeightSpeedMode = NORM_NORM;
    //    DoubleHeightOn = false;
    DoubleTravelOn = false;
    WalkMethod = false;
    //  DBGSerial.println("Init Commander End");

    SpeakStr("Init");

}


//==============================================================================
// This function is called by the main code to tell us when it is about to
// do a lot of bit-bang outputs and it would like us to minimize any interrupts
// that we do while it is active...
//==============================================================================
void PS3InputController::AllowControllerInterrupts(boolean fAllow)
{
    // We don't need to do anything...
}


//==============================================================================
// This is The main code to input function to read inputs from the Commander and then
//process any commands.
//==============================================================================
void PS3InputController::ControlInput(void)
{
    // See if we have a new command available...
    // Overkill, but first pass... 
    if(memcmp((const void *)&PS3, &PS3_PREV, sizeof(PS3)))
    {
        int LJoyX = PS3.key.LJoyX-128;
        int LJoyY = PS3.key.LJoyY-128;
        int RJoyX = PS3.key.RJoyX-128;
        int RJoyY = PS3.key.RJoyY-128;
#ifdef DEBUG
#ifdef DBGSerial
            // setup to output back to our USB port
            if (g_fDebugOutput)
            {
                DBGSerial.print( (PS3.keys[3]<< 16) | (PS3.keys[4] << 8) | PS3.keys[5], HEX);
                DBGSerial.print(" - ");
                DBGSerial.print(LJoyX, DEC);
                DBGSerial.print(" ");
                DBGSerial.print(LJoyY, DEC);
                DBGSerial.print(" ");
                DBGSerial.print(RJoyX, DEC);
                DBGSerial.print(" ");
                DBGSerial.print(RJoyY, DEC);
            }
#endif
#endif
        
        // See if start button was pressed
        if ((PS3.key.Start ) && !(PS3_PREV.key.Start )) 
        {
            if (g_InControlState.fRobotOn)
            {
                g_InControlState.fControllerInUse = true;	// make sure bypass is not used.
                PS3TurnRobotOff();
            }
            else
                g_InControlState.fRobotOn = true;           // turn it on... 
        }
        // Experimenting with trying to detect when IDLE.  maybe set a state of
        // of no button pressed and all joysticks are in the DEADBAND area...
        g_InControlState.fControllerInUse = PS3.keys[3] || PS3.keys[4] || PS3.keys[5] 
            || (abs(LJoyY) >= cTravelDeadZone)
            || (abs(RJoyX) >= cTravelDeadZone)
            || (abs(RJoyY) >= cTravelDeadZone);

        // [SWITCH MODES]

        // Cycle through modes...
        if ((PS3.key.Select ) && !(PS3_PREV.key.Select ))
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

        //[Common functions]
        //Switch Balance mode on/off
        if ((PS3.key.Square) && !(PS3_PREV.key.Square))
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
        if ((PS3.key.Triangle) && !(PS3_PREV.key.Triangle))
        {
            if (g_BodyYOffset>0)
                g_BodyYOffset = 0;
            else
                g_BodyYOffset = 35;
        }

        // We will use L6 with the Right joystick to control both body offset as well as Speed...
        // We move each pass through this by a percentage of how far we are from center in each direction
        // We get feedback with height by seeing the robot move up and down.  For Speed, I put in sounds
        // which give an idea, but only for those whoes robot has a speaker
        if (PS3.key.R1 )
        {
            // raise or lower the robot on the joystick up /down
            // Maybe should have Min/Max
            g_BodyYOffset += RJoyY/25;

            // Likewise for Speed control
            int dspeed = RJoyX / 16;      //
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

            RJoyX = 0;                    // don't walk when adjusting the speed here...
        }

        //[Walk functions]
        if (ControlMode == WALKMODE)
        {
            //Switch gates
            if (((PS3.key.Select) && !(PS3_PREV.key.Select))
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
            if ((PS3.key.R1) && !(PS3_PREV.key.R1))
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
            if ((PS3.key.RightHat) && !(PS3_PREV.key.RightHat))
            {
                MSound (1, 50, 2000);
                WalkMethod = !WalkMethod;
            }

            //Walking
            if (WalkMethod)                       //(Walk Methode)
                                                  //Right Stick Up/Down
                g_InControlState.TravelLength.z = (RJoyY);

            else
            {
                g_InControlState.TravelLength.x = -LJoyX;
                g_InControlState.TravelLength.z = LJoyY;
            }

            if (!DoubleTravelOn)                  //(Double travel length)
            {
                g_InControlState.TravelLength.x = g_InControlState.TravelLength.x/2;
                g_InControlState.TravelLength.z = g_InControlState.TravelLength.z/2;
            }

                                                  //Right Stick Left/Right
            g_InControlState.TravelLength.y = -(RJoyX)/4;
        }

        //[Translate functions]
        g_BodyYShift = 0;
        if (ControlMode == TRANSLATEMODE)
        {
            g_InControlState.BodyPos.x =  SmoothControl(((LJoyX)*2/3), g_InControlState.BodyPos.x, SmDiv);
            g_InControlState.BodyPos.z =  SmoothControl(((LJoyY)*2/3), g_InControlState.BodyPos.z, SmDiv);
            g_InControlState.BodyRot1.y = SmoothControl(((RJoyX)*2), g_InControlState.BodyRot1.y, SmDiv);

            //      g_InControlState.BodyPos.x = (LJoyX)/2;
            //      g_InControlState.BodyPos.z = -(LJoyY)/3;
            //      g_InControlState.BodyRot1.y = (RJoyX)*2;
            g_BodyYShift = (-(RJoyY)/2);
        }

        //[Rotate functions]
        if (ControlMode == ROTATEMODE)
        {
            g_InControlState.BodyRot1.x = (LJoyY);
            g_InControlState.BodyRot1.y = (RJoyX)*2;
            g_InControlState.BodyRot1.z = (LJoyX);
            g_BodyYShift = (-(RJoyY)/2);
        }

        //Calculate walking time delay
        g_InControlState.InputTimeDelay = 128 - max(max(abs(LJoyX), abs(LJoyY)), abs(RJoyX));

        //Calculate g_InControlState.BodyPos.y
        g_InControlState.BodyPos.y = max(g_BodyYOffset + g_BodyYShift,  0);

        // Save away the buttons state as to not process the same press twice.
        memcpy(&PS3_PREV, (const void *)&PS3, sizeof(PS3));
        g_ulLastMsgTime = millis();
    }
}


//==============================================================================
// PS3TurnRobotOff - code used couple of places so save a little room...
//==============================================================================
void PS3TurnRobotOff(void)
{
    //Turn off
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

