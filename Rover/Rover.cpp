/****************************************************************************
* Kurt's Rover - Running Linux
*     Can configure for Sabertooth or Roboclaw
*     using either PWM or Packet serial.
*     This version is using an XBee based Commander
*     This supports an optional Pan/Tilt option. 
*
*PS2 CONTROLS:
*	- Start				Turn on/off Rover / Arm
*
*	ROVER CONTROL
*	Tank mode:
*	- Left Stick U/D	Left Wheels forward / reverse
*	- Right Stick U/D	Right Wheels forward / reverse
*
*	One Stick mode:
*	- Right Stick U/D	forward / reverse
*	- Right Stick L/R	steering
*       - Left Stick            Pan/Tilt - delta from current position. 
*
*	universal:
*	- D-Pad up		Shift up gear (1 to 4)
*	- D-Pad down		Shift down gear (1 to 4)
*	- R3			Change steering mode (Tank / One stick)
*       - L3                    Move Pan/Tilt back to zero position. 
*
* On Roboclaw:
*    Switch settings for Packet mode: 0010100000
*                           PWM Mode: 1000e00000
*    The Motor Controller should be configured with Mixed Mode off as this program
*    allows you to control the rover in both Tank mode and in a logical mixed mode.
*
* Configured for use with BotboardDuino
*    Pin        Usage
*    0-1        USB
*    
*    5          Sound Pin
*    6-9        PS2
*    10-11      Motor Controller (Channel 1/2)  
*
****************************************************************************/

#define DEBUG
//--------------------------------------------------------------------------
// Included libraries
//--------------------------------------------------------------------------
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
#include "RoboClaw.h"
#include "CommanderEx.h"


//--------------------------------------------------------------------------
// Global Defines
//--------------------------------------------------------------------------

// Move most of the configuration gunk into header file.
#include "Rover_Config.h"

#ifdef BBB_SERVO_SUPPORT
#include "PWM.h"


// BUGBUG: should work on not hard codding...
PWM::Pin pinPan("P8_13", 20000000); // Since both pins share the same channel, they're periods must be the same
PWM::Pin pinTilt("P8_19", 20000000);
PWM::Pin pinRot("P9_16", 20000000);

PWM::Pin *g_aServos[] = {&pinPan, &pinTilt, &pinRot};

#endif

enum {ONE_STICK_MODE, TANK_MODE};    // define different steering modes

#define cTravelDeadZone 4                         //The deadzone for the analog input from the remote

#define ARBOTIX_TO  1000                          // if we don't get a valid message in this number of mills turn off

//--------------------------------------------------------------------------
// Global classes and variables
//--------------------------------------------------------------------------

Commander       command = Commander();
RoboClaw        RClaw;
WrapperSerial   Serial = WrapperSerial();


// Define a Motor Driver class, should probably extract this to another file
class MotorsDriver {
  public:  
    void Init(void);
    void RDrive(short sVal);
    void LDrive(short sVal);
  private:
    short     RDrivePrev;
    short     LDrivePrev;

};



MotorsDriver g_MotorsDriver;

// Rover State Variables...
boolean     g_fRoverActive;        // Is the rover active?
boolean     g_fRoverActivePrev;    // Previous state...
boolean     g_fServosInit;
uint8_t     g_bGear;               // What gear our we in?
uint8_t     g_bSteeringMode;       //

// Some definitions for Pan and Tilt
#ifdef BBB_SERVO_SUPPORT
uint16_t        g_wPan;              // What PWM value should we send to the Pan servo
uint16_t        g_wTilt;             // Likewise for Tilt... 
uint16_t        g_wRot;              // And the rotation
#endif

// Normalized data coming back from PS2 function
short            LStickX;	        //Stick position to calculate one stick steering
short            LStickY;		    // Stick position to calculate one stick steering
short            RStickX;		    // Stick position to calculate one stick steering
short            RStickY;		    // Stick position to calculate one stick steering

short           sRDrivePWM;
short           sLDrivePWM;

// Scratch variables
uint16_t        w;

// Some input save information.
unsigned long   g_ulLastMsgTime;
static byte     g_bButtonsPrev;
boolean         g_fControllerInUse;

// Debug stuff
boolean         g_fShowDebugPrompt;
boolean         g_fDebugOutput;

//  Variables to use to keep from printing too often...
#ifdef DEBUG
short           RStickYPrev;
short           RStickXPrev;
short           LStickYPrev;
short           LStickXPrev;
short           sRDrivePWMPrev;
short           sLDrivePWMPrev;
#endif


#if MNUMSERVOS > 0
typedef struct _servo_save_data
{
    uint8_t	cServos;		// count of servos in our save data.
    uint8_t	bChksum;		// Checksum.  simple check to see if the data is valid
    struct _asd				// Servo Data
    {
        short	sPWOffset;	        // The PW Offset in microseconds.
    } asd[MNUMSERVOS];

} SERVO_SAVE_DATA;


#endif

char szRClawDevice[] = "/dev/ttyRCLAW"; 		// We created symbolic link on BBBk"/dev/ttyO2";
char szCommanderDevice[] = "/dev/ttyXBEE";        // "/dev/ttyO1";

//--------------------------------------------------------------------------
// External and forward reference function definitions
//--------------------------------------------------------------------------
extern void InitializeServos(void);
extern boolean TerminalMonitor(void);
extern void CheckVoltages(void) ;
extern void ControlInput(void);
extern void FindServoZeroPoints();

#define RCD_VERSION 01
class RoverConfigData 
{
    public: 
        void Load(void);
        void Save(void);
        
        // For now have real simple data.
        uint8_t     bVersion;   // What version is stored out.
        uint8_t     bChecksum;  // checksum
        
        // Real data - start off with servos.
        enum {PAN=0, TILT, ROTATE, cServos};
        struct {
            uint16_t    wCenter;    // Pulse Width for Center of Pan
            uint16_t    wMin;        // Minimum Value;
            uint16_t    wMax;        // Max Pan value
        } aServos[RoverConfigData::cServos];
};

RoverConfigData rcd;    // create an instance of the configuration data.

//--------------------------------------------------------------------------
// SignalHandler - Try to free up things like servos if we abort.
//--------------------------------------------------------------------------
void SignalHandler(int sig){
    printf("Caught signal %d\n", sig);

    // Stop motors if they are active
    if (g_fRoverActive) {    
        g_MotorsDriver.RDrive(0);
        g_MotorsDriver.LDrive(0);
    }

#ifdef BBB_SERVO_SUPPORT
    printf("Free Servos\n");
    for (int i=0; i < sizeof(g_aServos)/sizeof(g_aServos[0]); i++) {
        g_aServos[i]->Disable();
    }
#endif
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


    if (!RClaw.begin(pszDevice = (argc > 1? argv[1] : szRClawDevice), B38400))
    {
        printf("RClaw (%s) Begin failed\n", pszDevice);
        return 0;
    }    

    if (!command.begin(pszDevice = (argc > 2? argv[2] : szCommanderDevice), B38400))
    {
        printf("Commander (%s) Begin failed\n", pszDevice);
        return 0;
    }    

    int error;
    
    delay(250);
    Serial.begin(/*57600*/);

    // Try to load the Rover Configuration Data
    rcd.Load();

    g_MotorsDriver.Init();

    Serial.println("Kurt's Rover Program Startup\n");
 
    g_fDebugOutput = false;			// start with it off!
    g_fShowDebugPrompt = true;
    g_fRoverActive = false;
    g_fRoverActivePrev = false;
    g_fServosInit = false;
    g_bGear = 3;                                // We init in 3rd gear.
    g_bSteeringMode = ONE_STICK_MODE;
    // Initialize our pan and tilt servos
    InitializeServos();                                // Make sure the servos are active
    
    for(;;) 
    {
    //--------------------------------------------------------------------------
    // Loop: the main arduino main Loop function
    //--------------------------------------------------------------------------
        // We also have a simple debug monitor that allows us to 
        // check things. call it here..
        if (TerminalMonitor())
            continue;           
            
        CheckVoltages();    // check voltages - if we get too low shut down the servos...    
        
        // Lets get the PS2 data...
        ControlInput();
        
        // Drive the rover
        if (g_fRoverActive) {
            if (g_bSteeringMode == TANK_MODE) {
                sRDrivePWM = LStickY; //RStickY; // BUGBUG - appears like wrong ones doing each...
                sLDrivePWM = RStickY; // LStickY;
            } else {    // One stick driving
                if ((RStickY >=0) && (RStickX >= 0)) {    // Quadrant 1
                    sRDrivePWM = RStickY - RStickX;
                    sLDrivePWM = max(RStickX, RStickY);
                } else if ((RStickY<0) && (RStickX>=0))   { //Quadrant 2
                sRDrivePWM = (RStickY + RStickX); 
                sLDrivePWM = min (-RStickX, RStickY);    
                    
            } else if ((RStickY<0)  && (RStickX<0)) {    //Quadrant 3
                sRDrivePWM = min (RStickX, RStickY);
                sLDrivePWM = (RStickY - RStickX);
                
            } else if ((RStickY>=0) && (RStickX<0)) {    // Quadrant 4
                sRDrivePWM = max(-RStickX, RStickY);
                sLDrivePWM = (RStickY + RStickX);
            } else {	
                    sRDrivePWM = 0;
                sLDrivePWM = 0;
                }
            }
            
            // Lets output the appropriate stuff to the motor controller
            // ok lets figure out our speeds to output to the two motors.  two different commands
            // depending on going forward or backward.
            // Scale the two values for the motors.
            sRDrivePWM = max(min((sRDrivePWM * g_bGear) / 4, 127), -127);    // This should keep up in the -127 to +127 range and scale it depending on what gear we are in.
            sLDrivePWM = max(min((sLDrivePWM * g_bGear) / 4, 127), -127);

#ifdef DEBUG
            if (g_fDebugOutput) {
                if ((RStickY != RStickYPrev) || (RStickX != RStickXPrev) ||
                        (LStickY != LStickYPrev) || (LStickX != LStickXPrev) ||
                        (sRDrivePWM != sRDrivePWMPrev) || (sLDrivePWM != sLDrivePWMPrev)) {
                    Serial.print(LStickY, DEC);
                    Serial.print(",");
                    Serial.print(LStickX, DEC);
                    Serial.print(" ");
                    Serial.print(RStickY, DEC);
                    Serial.print(",");
                    Serial.print(RStickX, DEC);
                    Serial.print(" - ");
                    Serial.print(sLDrivePWM, DEC);
                    Serial.print(",");
                    Serial.println(sRDrivePWM, DEC);
                    LStickYPrev = LStickY;
                    LStickXPrev = LStickX;
                    RStickYPrev = RStickY;
                    RStickXPrev = RStickX;
                    sRDrivePWMPrev = sRDrivePWM;
                    sLDrivePWMPrev = sLDrivePWM;
                }
            }
#endif
        // Call our motors driver code which may change depending on how we talk to the motors...
            g_MotorsDriver.RDrive(sRDrivePWM);
            g_MotorsDriver.LDrive(sLDrivePWM);

            // Also if we have a pan/tilt lets update that as well..
    #ifdef BBB_SERVO_SUPPORT
            if (g_bSteeringMode != TANK_MODE) {
                if (LStickX ) {
                    if (command.buttons & BUT_L6) {     //modify which thing we are controlling depending on if L6 is down or not.
                        w = max(min(g_wRot + LStickX/8, rcd.aServos[RoverConfigData::ROTATE].wMax), rcd.aServos[RoverConfigData::ROTATE].wMin);
                        if (w != g_wRot) {
                            pinRot.SetDutyUS(w);
                            g_wRot = w;
                        }
                    } else {
                        w = max(min(g_wPan + LStickX/8, rcd.aServos[RoverConfigData::PAN].wMax), rcd.aServos[RoverConfigData::PAN].wMin);
                        if (w != g_wPan) {
                            pinPan.SetDutyUS(w);
                            g_wPan = w;
                        }
                    }    
                }
        
                if (LStickY) {
                    w = max(min(g_wTilt + LStickY/8, rcd.aServos[RoverConfigData::TILT].wMax), rcd.aServos[RoverConfigData::TILT].wMin);
                    if (w != g_wTilt) {
                        pinTilt.SetDutyUS(w);
                        g_wTilt = w;
                    }
                }
            }
    #endif
            
            delay (10);
        } else {
            if (g_fRoverActivePrev) {    
                MSound( 3, 100, 2500, 80, 2250, 60, 2000);
                g_MotorsDriver.RDrive(0);
                g_MotorsDriver.LDrive(0);
            }
        delay (10);
        }

        g_fRoverActivePrev = g_fRoverActive; 	
    }
}

//==============================================================================
// Simple inline function 
//==============================================================================

inline short NormalizeJoystickValue(short s)
{
    if ((s > DEADBAND) || (s < -DEADBAND ))
        return s;
    return 0;
}

//==============================================================================
// ControlInput -This is code the checks for and processes input from
//        the Commander controller.
//==============================================================================
void ControlInput(void)
{
    boolean fPacketChanged;
    short	JoyStickValueIn;
    
    if(command.ReadMsgs() > 0)
    {
 
        if (!g_fRoverActive) 
        {
            //Turn on
            MSound( 3, 60, 2000, 80, 2250, 100, 2500);
            g_fRoverActive = true;
        }
    
  
        // Experimenting with trying to detect when IDLE.  maybe set a state of
        // of no button pressed and all joysticks are in the DEADBAND area...
        g_fControllerInUse = command.buttons 
            || (abs(command.leftH) >= cTravelDeadZone)
            || (abs(command.leftV) >= cTravelDeadZone)
            || (abs(command.rightH) >= cTravelDeadZone)
            || (abs(command.rightV) >= cTravelDeadZone);

#ifdef BBB_SERVO_SUPPORT
        // If we have a pan and tilt servos use LT to reset them to zero. 
        if ((command.buttons & BUT_LT) && !(g_bButtonsPrev & BUT_LT))
        {
            g_wPan = rcd.aServos[RoverConfigData::PAN].wCenter;
            g_wTilt = rcd.aServos[RoverConfigData::TILT].wCenter;
            g_wRot = rcd.aServos[RoverConfigData::ROTATE].wCenter;
            
            // Could do as one move but good enough
            pinPan.SetDutyUS(g_wPan);
            pinTilt.SetDutyUS(g_wTilt);
            pinRot.SetDutyUS(g_wRot);
        }
#endif
    

        // Check some buttons for to see if we should be changing state...
        if ((command.buttons & BUT_L4) && !(g_bButtonsPrev & BUT_L4))
        {
            if (g_bGear < 4) {
                g_bGear++;
                MSound( 1, 50, 2000);
            } else {
                MSound( 2, 50, 2000, 50, 2500);
            }
        }

        if ((command.buttons & BUT_L5) && !(g_bButtonsPrev & BUT_L5))
        {
            if (g_bGear > 1) {
                g_bGear--;
                MSound( 1, 50, 2000);
            } else {
                MSound( 2, 50, 2500, 50, 2000);
            }
        }

        if ((command.buttons & BUT_RT) && !(g_bButtonsPrev & BUT_RT))
        {
            MSound( 1, 50, 2000);
            
            if (g_bSteeringMode == ONE_STICK_MODE)
                g_bSteeringMode = TANK_MODE;
            else
                g_bSteeringMode = ONE_STICK_MODE;
        }
        
        // Ok lets grab the current Stick values.
        LStickY = NormalizeJoystickValue(command.rightV);
        LStickX = NormalizeJoystickValue(command.rightH);
        RStickY = NormalizeJoystickValue(command.leftV);
        RStickX = NormalizeJoystickValue(command.leftH);
       
        // Save away the buttons state as to not process the same press twice.
        g_bButtonsPrev = command.buttons;
        g_ulLastMsgTime = millis();

    }
    else
    {
        // We did not receive a valid packet.  check for a timeout to see if we should turn robot off...
        if (g_fRoverActive)
        {
            if ((millis() - g_ulLastMsgTime) > ARBOTIX_TO)
            {
                g_fRoverActive = false;
                // Turn off...
                MSound( 3, 60, 2500, 80, 2250, 100, 2000);
            }
        }
    }

}

//==============================================================================
// InitializeServos - We will delay when we call the servo init stuff until a
//        command such as start is called on.  This should make it that if your
//        robot starts off running on USB power it does not overpower it...
//==============================================================================
void InitializeServos(void)
{
#ifdef BBB_SERVO_SUPPORT

    // Enable both only after we have set the periods properly.
    // Otherwise we will have conflicts since each pin will try to set its own period and conflict with the others
    g_wPan = rcd.aServos[RoverConfigData::PAN].wCenter;
    g_wTilt = rcd.aServos[RoverConfigData::TILT].wCenter; 
    g_wRot =  rcd.aServos[RoverConfigData::ROTATE].wCenter;  

    pinPan.SetDutyUS(g_wPan);
    pinTilt.SetDutyUS(g_wTilt);
    pinRot.SetDutyUS(g_wRot);
    pinPan.Enable();
    pinTilt.Enable();
    pinRot.Enable();



#endif
}


//==============================================================================
// TerminalMonitor - Simple background task checks to see if the user is asking
//    us to do anything, like update debug levels ore the like.
//==============================================================================
boolean TerminalMonitor(void)
{
    uint8_t szCmdLine[20];
    int ich;
    int ch;
    // See if we need to output a prompt.
    if (g_fShowDebugPrompt) {
        Serial.println("Rover Debug Monitor");
        Serial.println("D - Toggle debug on or off");
#ifdef BBB_SERVO_SUPPORT
        Serial.println("O - Enter Servo offset mode");
        Serial.println("S <SN> <ANGLE> - Move Servo");
#endif        
        g_fShowDebugPrompt = false;
    }
       
    // First check to see if there is any characters to process.
    if (ich = Serial.available()) {
        ich = 0;
        // For now assume we receive a packet of data from serial monitor, as the user has
        // to click the send button...
        for (ich=0; ich < sizeof(szCmdLine); ich++) {
            ch = Serial.read();        // get the next character
            if ((ch == -1) || ((ch >= 10) && (ch <= 15)))
                break;
             szCmdLine[ich] = ch;
        }
        szCmdLine[ich] = '\0';    // go ahead and null terminate it...
        Serial.print("Serial Cmd Line:");        
        Serial.write(szCmdLine, ich);
        Serial.println("!!!");
        
        // So see what are command is.
        if (ich == 0) {
            g_fShowDebugPrompt = true;
        } else if ((ich == 1) && ((szCmdLine[0] == 'd') || (szCmdLine[0] == 'D'))) {
            g_fDebugOutput = !g_fDebugOutput;
            if (g_fDebugOutput) 
                Serial.println("Debug is on");
            else
                Serial.println("Debug is off");
#ifdef BBB_SERVO_SUPPORT
        } else if ((ich == 1) && ((szCmdLine[0] == 'o') || (szCmdLine[0] == 'O'))) {
            FindServoZeroPoints();
        } else if (((szCmdLine[0] == 's') || (szCmdLine[0] == 'S'))) {
            // ok lets grab a servo number
            int iServo = 0;
            int iAngle = 0;    
            int i;        
            for (i=1; i< ich; i++) {
                if (szCmdLine[i] != ' ')
                    break;
            }
            
            if ((szCmdLine[i] >= '0') && (szCmdLine[i] <= '9')) {
                iServo = szCmdLine[i++] - '0';
                
                // now angle
                while (szCmdLine[i] == ' ')
                    i++;
                while ((szCmdLine[i] >= '0') && (szCmdLine[i] <= '9')) {
                    iAngle = iAngle*10 + szCmdLine[i++] - '0';
                }
                // Ok lets try moving the servo there...
                g_aServos[iServo]->SetDutyUS(iAngle);
            }
#endif
        }
        
        return true;
    }
    return false;
} 
//==============================================================================
// CheckVoltages: Try to catch when the voltages drop too low and turn off the
// servos.  This can be because the battery level got too low or if we turn it off
//     and the Arduino is still being powered by USB
//==============================================================================
uint16_t g_wVSPrev;
#if defined(PACKET_MODE)
unsigned long g_ulCVTimeLast;
#endif

void CheckVoltages(void) 
{
#ifdef AVS_PIN
    uint16_t wVS = analogRead(AVS_PIN);
#ifdef DEBUG
    if (g_fDebugOutput && (wVS != g_wVSPrev)) {
        Serial.print("VS: ");
        Serial.println(wVS, DEC);
        g_wVSPrev = wVS;
    }
#endif
    if ((wVS < (uint16_t)AVAL_MIN) && g_fServosInit) {
        // detach any servos we may have...
        MSound( 3, 100, 2500, 100, 2500, 100, 2500);
    }
#elif defined(PACKET_MODE)
    // Using Roboclaw in packet mode, so we can ask it for the
    // motor voltages... Note: This requires us to send and receive
    // stuff serially, so probably only do this every so often.
    if ((millis()-g_ulCVTimeLast) > 1000)
    {
        // more than a second since we last tested
        uint16_t wVS = RClaw.ReadMainBatteryVoltage(PACKETS_ADDRESS);

#ifdef DEBUG
        if (g_fDebugOutput && (wVS != g_wVSPrev)) {
            Serial.print("VS: ");
            Serial.println(wVS, DEC);
            g_wVSPrev = wVS;
        }
#endif
        if ((wVS < (uint16_t)VOLTAGE_MIN1) && g_fServosInit) {
            // detach any servos we may have...
            MSound( 3, 100, 2500, 100, 2500, 100, 2500);
        }
        g_ulCVTimeLast = millis();  // remember last we did it.
    }    
#endif
}




//==============================================================================
//	FindServoZeroPoints - Find the zero points for each of our servos... 
// 		Will use the new servo function to set the actual pwm rate and see
//		how well that works...
//==============================================================================
#ifdef BBB_SERVO_SUPPORT
void MoveServo(PWM::Pin *apin, uint16_t wNew, uint16_t wTime)
{
    long lPWNS = apin->GetDutyNS();	// Pulse Width in NS
    Serial.print((long)(wNew*1000), DEC);
    Serial.print(" ");
    Serial.print(lPWNS, DEC);

    uint16_t cCycles = (wTime/20);
    Serial.print(" ");
    Serial.print(cCycles, DEC);
    if (cCycles) {
        long lDeltaNSCycle = (((long)wNew*1000) - lPWNS) / cCycles;
        Serial.print(" ");
        Serial.println(lDeltaNSCycle, DEC);
        while (cCycles--) {
            lPWNS += lDeltaNSCycle;
            apin->SetDutyNS(lPWNS);
            delay(20); // delay a bit
        }
    }
    apin->SetDutyUS(wNew);
} 

void FindServoZeroPoints()
{
    // not clean but...
    int data;
    short sSN; 			// which servo number
    boolean fNew = true;	// is this a new servo to work with?
    boolean fExit = false;	// when to exit
    int ich;
    word wCenter;

    // OK lets move all of the servos to their zero point.
    InitializeServos();
    Serial.println("Find Servo Zeros.\n$-Exit, +- changes, *-change servo");

    // Lets move all of the servos to their default location...
    for (sSN=0; sSN < MNUMSERVOS; sSN++)
        g_aServos[sSN]->SetDutyUS(rcd.aServos[sSN].wCenter);

    sSN = 0;     // start at our first servo.

    while(!fExit) {
        if (fNew) {
            wCenter = rcd.aServos[sSN].wCenter;
            Serial.print("Servo: ");
            Serial.print(g_apszServos[sSN]);
            Serial.print("(");
            Serial.print(wCenter-1500, DEC);
            Serial.println(")");

            // Now lets wiggle the servo
            MoveServo(g_aServos[sSN], wCenter+250, 500);
            MoveServo(g_aServos[sSN], wCenter-250, 500);
            MoveServo(g_aServos[sSN], wCenter, 500);
            fNew = false;
        }
    
    //get user entered data
    data = Serial.read();
    //if data received
    if (data !=-1) 	{
            if (data == '$')
        fExit = true;	// not sure how the keypad will map so give NL, CR, LF... all implies exit

        else if ((data == '+') || (data == '-')) {
            if (data == '+')
                rcd.aServos[sSN].wCenter += 5;		// increment by 5us
            else
                rcd.aServos[sSN].wCenter -= 5;		// increment by 5us

            Serial.print("    ");
            Serial.println(rcd.aServos[sSN].wCenter, DEC);
            // Lets try to use attach to change the offsets...
            MoveServo(g_aServos[sSN], rcd.aServos[sSN].wCenter, 100);
        } else if ((data >= '0') && (data < ('0'+ MNUMSERVOS))) {
        // direct enter of which servo to change
        fNew = true;
        sSN = data - '0';
        } else if (data == '*') {
            // direct enter of which servo to change
        fNew = true;
        sSN++;
        if (sSN == MNUMSERVOS) 
            sSN = 0;	
        }
    }
    }
    Serial.print("Find Servo exit ");
    for (sSN=0; sSN < MNUMSERVOS; sSN++){
        Serial.print(" ");
        Serial.print(g_aServos[sSN]->GetDutyNS()/1000, DEC);
    }

    Serial.print("\nSave Changes? Y/N: ");

    //get user entered data
    while (((data = Serial.read()) == -1) || ((data >= 10) && (data <= 15)))
    ; 

    if ((data == 'Y') || (data == 'y')) {
        // call off to save away the updated data.
        rcd.Save();

    } else {
        g_fServosInit = false;    // Lets go back and reinit...
        rcd.Load();
    }

}

#endif

#ifndef OPT_PCMSOUND
void MSound(byte cNotes, ...)
{
}
#endif

//##############################################################################
//##############################################################################
//##############################################################################
//==============================================================================
// Packet Serial Mode
//==============================================================================
#ifdef PACKET_MODE



//==============================================================================
//  MotorsDriver::Init
//==============================================================================
void MotorsDriver::Init(void) {
    RDrivePrev = 0;
    LDrivePrev = 0;

    // BUGBUG:: have some of this inline currently...
}

//==============================================================================
//  MotorsDriver::RDrive
//==============================================================================
void MotorsDriver::RDrive(short sVal) {
    if (sVal != RDrivePrev) { 
        RDrivePrev = sVal;

        if (sVal >= 0) 
            RClaw.ForwardM1(PACKETS_ADDRESS, sVal);
        else
            RClaw.BackwardM1(PACKETS_ADDRESS, -sVal);
    }
}

//==============================================================================
//  MotorsDriver::RDrive
//==============================================================================
void MotorsDriver::LDrive(short sVal) {
    if (sVal != LDrivePrev) { 
        LDrivePrev = sVal;

        if (sVal >= 0) 
            RClaw.ForwardM2(PACKETS_ADDRESS, sVal);
        else
            RClaw.BackwardM2(PACKETS_ADDRESS, -sVal);
    }
}


#endif

//##############################################################################
//##############################################################################
//##############################################################################
//==============================================================================
// PWM Mode
//==============================================================================
#ifdef PWM_MODE
//==============================================================================
//  MotorsDriver::Init
//==============================================================================
void MotorsDriver::Init(void) {
    // The actual init of servo system will be done on first usage...
    RDrivePrev = 0;
    LDrivePrev = 0;
}

//==============================================================================
//  MotorsDriver::RDrive
//==============================================================================
void MotorsDriver::RDrive(short sVal) {
    InitializeServos();    // make sure the PWM has been initialized.
    if (sVal != RDrivePrev) { 
      RDrivePrev = sVal;
        g_aServos[MOTOR1_I].write(90+(sVal*2)/3);
    }
}

//==============================================================================
//  MotorsDriver::RDrive
//==============================================================================
void MotorsDriver::LDrive(short sVal) {
    InitializeServos();    // make sure the PWM has been initialized.
    if (sVal != LDrivePrev) { 
      LDrivePrev = sVal;
        g_aServos[MOTOR2_I].write(90+(sVal*2)/3);
    }
}
#endif


//==============================================================================
//  RoverConfigData Class...
//==============================================================================

void RoverConfigData::Load(void) {
    // Initialize to default data...
    // Lets see if we can read in the file information...
    boolean fValid = false;

    FILE * pf = fopen("/usr/share/rover/roverrc", "r");
    if (pf) {
        int cbRead = fread((uint8_t *)this, 1, sizeof(RoverConfigData), pf);
        fclose(pf);
        if ((cbRead == sizeof(RoverConfigData)) && (bVersion == RCD_VERSION)) {
            uint8_t *pb = (uint8_t *)this;
            uint8_t bCalcChecksum = 0;
    
            for(pb+=2; pb < ((uint8_t*)this + sizeof(RoverConfigData)); pb++)
                bCalcChecksum += *pb;
            if (bCalcChecksum == bChecksum) {
                printf("Rover Config Data Valid\n");
                fValid = true;
            }
        }
    }

    if (!fValid) {
        printf("Rover Config Data - Using defaults\n");
        aServos[RoverConfigData::PAN].wCenter = 1500;
        aServos[RoverConfigData::PAN].wMin = PAN_MIN;
        aServos[RoverConfigData::PAN].wMax = PAN_MAX;
        
        aServos[RoverConfigData::TILT].wCenter = 1500;
        aServos[RoverConfigData::TILT].wMin = TILT_MIN;   
        aServos[RoverConfigData::TILT].wMax = TILT_MAX;

        aServos[RoverConfigData::ROTATE].wCenter = 1500;
        aServos[RoverConfigData::ROTATE].wMin = ROTATE_MIN;   
        aServos[RoverConfigData::ROTATE].wMax = ROTATE_MAX;
    }
}

void RoverConfigData::Save(void) {
    bVersion = RCD_VERSION;   // What version is stored out.
    uint8_t *pb = (uint8_t *)this;
    bChecksum = 0;
    
    for(pb+=2; pb < ((uint8_t*)this + sizeof(RoverConfigData)); pb++)
        bChecksum += *pb;
    
    
    FILE * pf = fopen("/usr/share/rover/roverrc", "w");
    if (!pf) {
        int status = mkdir("/usr/share/rover", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        if (status == 0)
            pf = fopen("/usr/share/rover/roverrc", "w");
    }

    if (pf) {
        fwrite((uint8_t *)this, sizeof(RoverConfigData), 1, pf);
        fclose(pf);
    }
    else {
        printf("Failed to open otuput file\n");
    }
}
