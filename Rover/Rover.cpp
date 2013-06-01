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
#ifdef PAN_SERVO
uint16_t        g_wPan;              // What PWM value should we send to the Pan servo
uint16_t        g_wTilt;             // Likewise for Tilt... 
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
boolean g_fShowDebugPrompt;
boolean g_fDebugOutput;


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

// And define our servos
ServoEx     g_aServos[MNUMSERVOS];    // Define servos object for each servo

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


//--------------------------------------------------------------------------
// SETUP: the main arduino setup function.
//--------------------------------------------------------------------------
int main(int argc, char *argv[])
{
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


    g_MotorsDriver.Init();

    Serial.println("Botboardduino Rover Program Startup");
 
    g_fDebugOutput = false;			// start with it off!
    g_fShowDebugPrompt = true;
    g_fRoverActive = false;
    g_fRoverActivePrev = false;
    g_fServosInit = false;
    g_bGear = 3;                                // We init in 3rd gear.
    g_bSteeringMode = ONE_STICK_MODE;
#ifdef PAN_SERVO
    g_wPan = 90;
    g_wTilt = 90;    // In degrees
#endif
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
                sRDrivePWM = RStickY;
                sLDrivePWM = LStickY;
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
                Serial.print(RStickY, DEC);
                Serial.print(",");
                Serial.print(RStickX, DEC);
                Serial.print(" - ");
                Serial.print(sRDrivePWM, DEC);
                Serial.print(",");
                Serial.println(sLDrivePWM, DEC);
            }
    #endif
        // Call our motors driver code which may change depending on how we talk to the motors...
            g_MotorsDriver.RDrive(sRDrivePWM);
            g_MotorsDriver.LDrive(sLDrivePWM);

            // Also if we have a pan/tilt lets update that as well..
    #ifdef PAN_SERVO
            if (g_bSteeringMode != TANK_MODE) {
                if (LStickX && ((ServoGroupMove.moving() & ((1 << PANS_I))) == 0)) {
                    w = max(min(g_wPan + LStickX/20, PAN_MAX), PAN_MIN);
                    if (w != g_wPan) {
                        g_aServos[PANS_I].move(w, 100);
                        g_wPan = w;
                    }
                }
        
                if (LStickY && ((ServoGroupMove.moving() & ((1 << TILTS_I))) == 0)) {
                    w = max(min(g_wTilt - LStickY/20, TILT_MAX), TILT_MIN);
                    if (w != g_wTilt) {
                        g_aServos[TILTS_I].move(w, 100);
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
            || (abs(command.rightH) >= cTravelDeadZone)
            || (abs(command.rightV) >= cTravelDeadZone)
            || (abs(command.leftH) >= cTravelDeadZone)
            || (abs(command.leftV) >= cTravelDeadZone);

#ifdef PAN_SERVO
        // If we have a pan and tilt servos use LT to reset them to zero. 
        if ((command.buttons & BUT_LT) && !(g_bButtonsPrev & BUT_LT))
        {
            g_wPan = 90;
            g_wTilt = 90;
            
            // Could do as one move but good enough
            g_aServos[PANS_I].move(90, 300);
            g_aServos[TILTS_I].move(90, 300);
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
        LStickY = NormalizeJoystickValue(command.leftV);
        LStickX = NormalizeJoystickValue(command.leftH);
        RStickY = NormalizeJoystickValue(command.rightV);
        RStickX = NormalizeJoystickValue(command.rightH);
       
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
#if MNUMSERVOS > 0
    SERVO_SAVE_DATA ssd;
    uint8_t bChksum;
    uint8_t i;
    uint8_t *pb;
    boolean fValid;

    if (g_fServosInit)
        return;

    g_fServosInit = true;    // say we did it...
    bChksum = 0;
    fValid = true;        // start off assuming true


    // Now try to read the block out starting at 0...
    pb = (uint8_t*)&ssd;
    for (i=0; i< sizeof(ssd); i++) {
       *pb++ = EEPROM.read(i);
    }

    // Make sure the count of servos looks about right...
    if ((ssd.cServos >=2) && (ssd.cServos <= MNUMSERVOS)) {
        // lets first loop through and calculate the checksum to make
        // sure the data looks valid.
        for (i=0; i< ssd.cServos; i++) 	{
            bChksum += (ssd.asd[i].sPWOffset & 0xff);
            bChksum += ssd.asd[i].sPWOffset >> 8;
            // do a little pre-validation...    

            if ((ssd.asd[i].sPWOffset < -200) || (ssd.asd[i].sPWOffset > 200))
                fValid = false;
        }	

        if (bChksum != ssd.bChksum) 	
            fValid = false;
    }

    // Lets dump the servo offsets to see what we have...
#ifdef DEBUG
    if (g_fDebugOutput) {
        Serial.print("Load Servo offsets :");
        Serial.print(fValid, HEX);
        // we have valid data, update our servos...
        for (i=0; i<MNUMSERVOS; i++) {
            Serial.print(" ");
            Serial.print(ssd.asd[i].sPWOffset, DEC);
        }
        Serial.println("");
    }
#endif        


    // OK Lets attach all of our servos
    for (i=0; i<MNUMSERVOS; i++) {
        if (fValid && (i < ssd.cServos))
            g_aServos[i].attach(pgm_read_uint8_t(&g_abPinTable[i]), OUR_SERVOS_MIN+ssd.asd[i].sPWOffset, OUR_SERVOS_MAX+ssd.asd[i].sPWOffset);
        else
            g_aServos[i].attach(pgm_read_uint8_t(&g_abPinTable[i]), OUR_SERVOS_MIN, OUR_SERVOS_MAX);

    }
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
        Serial.println("Arduino Rover Monitor");
        Serial.println("D - Toggle debug on or off");
#if MNUMSERVOS > 0
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
#if MNUMSERVOS > 0
        } else if ((ich == 1) && ((szCmdLine[0] == 'o') || (szCmdLine[0] == 'O'))) {
            FindServoZeroPoints();
        } else if (((szCmdLine[0] == 's') || (szCmdLine[0] == 'S'))) {
            // ok lets grab a servo number
            int iServo = 0;
            short sSign = 1;
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
                if (szCmdLine[i] == '-') {
                    sSign = -1;
                    i++;
                }
                while ((szCmdLine[i] >= '0') && (szCmdLine[i] <= '9')) {
                    iAngle = iAngle*10 + szCmdLine[i++] - '0';
                }
                // Ok lets try moving the servo there...
                InitializeServos();        // Make sure the servos have been initialized...
                g_aServos[iServo].move(sSign * iAngle, 500);
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
#if MNUMSERVOS > 0
void FindServoZeroPoints()
{
    // not clean but...
    int data;
    short sSN; 			// which servo number
    boolean fNew = true;	// is this a new servo to work with?
    boolean fExit = false;	// when to exit
    int ich;
    short sOffset;

    // OK lets move all of the servos to their zero point.
    InitializeServos();
    Serial.println("Find Servo Zeros.\n$-Exit, +- changes, *-change servo");

    // Lets move all of the servos to their default location...
    for (sSN=0; sSN < MNUMSERVOS; sSN++)
        g_aServos[sSN].move(90, 500);

    sSN = 0;     // start at our first servo.

    while(!fExit) {
        if (fNew) {
            sOffset = g_aServos[sSN].readMicroseconds() - 1500;    // assume we are at zero degrees which should map to 1500
            Serial << "Servo: " <<  g_apszServos[sSN] << "(" << _DEC(sOffset) << ")" << endl; 

        // Now lets wiggle the servo
            ServoGroupMove.wait(0x3f);
            g_aServos[sSN].move(90+25, 500);

            ServoGroupMove.wait(0x3f);
            g_aServos[sSN].move(90-25, 500);

            ServoGroupMove.wait(0x3f);
            g_aServos[sSN].move(90, 500);
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
            sOffset += 5;		// increment by 5us
        else
            sOffset -= 5;		// increment by 5us

        Serial <<"    " << _DEC(sOffset) << endl; 
                // Lets try to use attach to change the offsets...
                g_aServos[sSN].attach(pgm_read_uint8_t(&g_abPinTable[sSN]), OUR_SERVOS_MIN+sOffset, OUR_SERVOS_MAX+sOffset);
                g_aServos[sSN].move(90, 500);
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
        Serial.print(g_aServos[sSN].readMicroseconds(), DEC);
    }

    Serial.print("\nSave Changes? Y/N: ");

    //get user entered data
    while (((data = Serial.read()) == -1) || ((data >= 10) && (data <= 15)))
    ; 

    if ((data == 'Y') || (data == 'y')) {
        // Ok they asked for the data to be saved.  We will store the data with a 
        // number of servos (uint8_t)at the start, followed by a uint8_t for a checksum...followed by uint16_t for each servo center and for the
    // heck of it a uint16_t for the range, although we don't muck with it yet, we might...
    // 
    SERVO_SAVE_DATA ssd;
    ssd.cServos = MNUMSERVOS;
    ssd.bChksum = 0;
    for (sSN=0; sSN < MNUMSERVOS; sSN++) {
        ssd.asd[sSN].sPWOffset =  g_aServos[sSN].readMicroseconds()-1500;
        ssd.bChksum += (ssd.asd[sSN].sPWOffset & 0xff);
        ssd.bChksum += ssd.asd[sSN].sPWOffset >> 8;
    }

    // Will write the block out starting at 0... may later change to use some of the other stuff.
        uint8_t *pb = (uint8_t*)&ssd;
        
        for (sSN=0; sSN < sizeof(ssd); sSN++) 
            EEPROM.write(sSN, *pb++);
    } else {
        g_fServosInit = false;    // Lets go back and reinit...
         InitializeServos();
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
