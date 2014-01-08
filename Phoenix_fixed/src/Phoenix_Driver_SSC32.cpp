#include "Hex_Cfg.h"
#include "Phoenix.h"
#include "WrapperSerial.h"
//====================================================================
//Project Lynxmotion Phoenix
//
// Servo Driver - This version is setup to use the SSC-32 to control
// the servos.
//====================================================================
#ifdef c4DOF
#define NUMSERVOSPERLEG 4
#else
#define NUMSERVOSPERLEG 3
#endif

#define NUMSERVOS (NUMSERVOSPERLEG*CNT_LEGS)

// Local data -- BUGBUG -- Could make it part of Servo object.
// Lets try only output something if it changed.
word		g_awSSC32ServoPos[NUMSERVOS];		// save away previous servo positions...
boolean		g_fSSC32ServoChanged;



//Servo Pin numbers - May be SSC-32 or actual pins on main controller, depending on configuration.
#ifdef QUADMODE
const byte cCoxaPin[] PROGMEM =
{
    cRRCoxaPin,  cRFCoxaPin,  cLRCoxaPin,  cLFCoxaPin
};
const byte cFemurPin[] PROGMEM =
{
    cRRFemurPin, cRFFemurPin, cLRFemurPin, cLFFemurPin
};
const byte cTibiaPin[] PROGMEM =
{
    cRRTibiaPin, cRFTibiaPin, cLRTibiaPin, cLFTibiaPin
};
#ifdef c4DOF
const byte cTarsPin[] PROGMEM =
{
    cRRTarsPin, cRFTarsPin, cLRTarsPin, cLFTarsPin
};
#endif
#else
const byte cCoxaPin[] PROGMEM =
{
    cRRCoxaPin,  cRMCoxaPin,  cRFCoxaPin,  cLRCoxaPin,  cLMCoxaPin,  cLFCoxaPin
};
const byte cFemurPin[] PROGMEM =
{
    cRRFemurPin, cRMFemurPin, cRFFemurPin, cLRFemurPin, cLMFemurPin, cLFFemurPin
};
const byte cTibiaPin[] PROGMEM =
{
    cRRTibiaPin, cRMTibiaPin, cRFTibiaPin, cLRTibiaPin, cLMTibiaPin, cLFTibiaPin
};
#ifdef c4DOF
const byte cTarsPin[] PROGMEM =
{
    cRRTarsPin, cRMTarsPin, cRFTarsPin, cLRTarsPin, cLMTarsPin, cLFTarsPin
};
#endif
#endif

// We are using our wrapper serial as to make more of the code shared
WrapperSerial SSCSerial;

//=============================================================================
// Global - Local to this file only...
//=============================================================================

// definition of some helper functions
extern int SSCRead (byte* pb, int cb, unsigned long ulTimeout, int EOL);

char szSSC32Device[] = "/dev/ttySSC-32";

//--------------------------------------------------------------------
//Init
//--------------------------------------------------------------------
void ServoDriver::Init(void)
{

    bool f = SSCSerial.begin(szSSC32Device, B115200);
    if (!f)
    {
#ifdef DBGSerial
        DBGSerial.println("SSCSeerial begin failed!");
#endif
        MSound (3, 40, 2500, 40, 2500, 40, 2500);
        return;
    }

    char abT[40];                                 // give a nice large buffer.
    byte cbRead;

#ifdef DBGSerial
    SSCSerial.println(F("VER"));
    SSCSerial.flush();
    cbRead = SSCRead((byte*)abT, sizeof(abT), 250000, -1);
    if (cbRead)
    {
        DBGSerial.print(F("Ver: "));
        DBGSerial.println(abT);
    }
#endif
    _fServosActive = false;                       // are the servos active?

    // Lets do the check for GP Enabled here...
#ifdef OPT_GPPLAYER
    _fGPEnabled = false;                          // starts off assuming that it is not enabled...
    _fGPActive = false;

#ifdef __AVR__
#if not defined(UBRR1H)
#if cSSC_IN != 0
    SSCSerial.listen();
#endif
#endif
#endif
    // Instead of hard checking version numbers instead ask it for
    // status of one of the players.  If we do not get a response...
    // probably does not support
    SSCSerial.println(F("QPL0"));
    cbRead = SSCRead((byte*)abT, 4, 25000, -1);

#ifdef DBGSerial
    DBGSerial.print(F("Check GP Enable: "));
    DBGSerial.println(cbRead, DEC);
#endif
    if (cbRead == 4)
        _fGPEnabled = true;                       // It Responded so must support GP...
    else
        MSound (2, 40, 2500, 40, 2500);
#endif

#ifdef cVoltagePin
    // Prime the voltage values...
    for (byte i=0; i < 8; i++)
        GetBatteryVoltage();
#endif
}

//--------------------------------------------------------------------
// Cleanup
//--------------------------------------------------------------------
void ServoDriver::Cleanup(void) {
    // Do any cleanup that the driver may need.
    printf("ServoDriver::Cleanup\n\r");
    FreeServos();
}


//--------------------------------------------------------------------
//GetBatteryVoltage - Maybe should try to minimize when this is called
// as it uses the serial port... Maybe only when we are not interpolating
// or if maybe some minimum time has elapsed...
//--------------------------------------------------------------------

#ifdef cVoltagePin
#ifndef CVADR1
#define CVADR1      30                            // VD Resistor 1 - reduced as only need ratio... 30K and 10K
#define CVADR2      10                            // VD Resistor 2
#endif

word  g_awVoltages[8]=
{
    0,0,0,0,0,0,0,0
};
word  g_wVoltageSum = 0;
byte  g_iVoltages = 0;

word ServoDriver::GetBatteryVoltage(void)
{
    g_iVoltages = (++g_iVoltages)&0x7;            // setup index to our array...
    g_wVoltageSum -= g_awVoltages[g_iVoltages];
    g_awVoltages[g_iVoltages] = analogRead(cVoltagePin);
    g_wVoltageSum += g_awVoltages[g_iVoltages];

    return ((long)((long)g_wVoltageSum*125*(CVADR1+CVADR2))/(long)(2048*(long)CVADR2));

}
#endif

//==============================================================================
// Quick and dirty helper function to read so many bytes in from the SSC with a timeout and an end of character marker...
//==============================================================================
int SSCRead (byte* pb, int cb, unsigned long ulTimeout, int EOL)
{
    int ich;
    byte* pbIn = pb;
    unsigned long ulTimeLastChar = micros();
    while (cb)
    {
        while ((ich = SSCSerial.read()) == -1)
        {
            // check for timeout
            if ((unsigned long)(micros()-ulTimeLastChar) > ulTimeout)
            {
                return (int)(pb-pbIn);
            }
            delayMicroseconds(100);	// don't just hang here...
        }
        *pb++ = (byte)ich;
        cb--;

        if (ich == EOL)
            break;                                // we matched so get out of here.
        ulTimeLastChar = micros();                // update to say we received something
    }

    return (int)(pb-pbIn);
}


//--------------------------------------------------------------------
//[GP PLAYER]
//--------------------------------------------------------------------
#ifdef OPT_GPPLAYER
uint8_t g_bGPCntSteps;
uint8_t g_bGPCurStep;
boolean g_fGPSMChanged;
//--------------------------------------------------------------------
//[FIsGPSeqDefined]
//--------------------------------------------------------------------
boolean ServoDriver::FIsGPSeqDefined(uint8_t iSeq)
{
    word wGPSeqPtr;

    // See if we can see if this sequence is defined
    SSCSerial.print(F("EER -"));
    SSCSerial.print(iSeq*2, DEC);
    SSCSerial.println(F(";2"));
    if ((SSCRead((byte*)&wGPSeqPtr, sizeof(wGPSeqPtr), 1000, -1) == sizeof(wGPSeqPtr)) && (wGPSeqPtr != 0)  && (wGPSeqPtr != 0xffff))
    {
        return true;
    }
    return false;                                 // nope return error
}


//--------------------------------------------------------------------
// Setup to start sequence number...
//--------------------------------------------------------------------
void ServoDriver::GPStartSeq(uint8_t iSeq)
{
    if (!_fGPActive && (iSeq != 0xff))
    {
        _fGPActive = true;
        _iSeq = iSeq;
        g_bGPCntSteps = 0xff;
        g_fGPSMChanged = false;
        _fServosActive = true;                    // Remember we have actually output something to the servos...
    }
    else
    {
        _iSeq = iSeq;                             // Signal for abort
    }

}


//--------------------------------------------------------------------
//[GP PLAYER]
//--------------------------------------------------------------------
void ServoDriver::GPPlayer(void)
{
    byte abStat[4];
    //byte cbRead;

    if (_fGPActive)
    {
        if (g_bGPCntSteps == 0xff)
        {
            // We have not init yet...
            g_bGPCntSteps = GPNumSteps();         // so get the number of steps.
            if (g_bGPCntSteps == 0xff)
            {
                _fGPActive = false;               // error so bail out of here...
            }
            else
            {
                g_InputController.AllowControllerInterrupts(false);

                // Since we are monitoring GP Sequence we no longer tell it to only run once...
                SSCSerial.print(F("PL0SQ"));
                SSCSerial.println(_iSeq, DEC);
                delay(20);
                while (SSCSerial.read() != -1)    // remove anything that was queued up.
                    ;
                g_InputController.AllowControllerInterrupts(true);
            }
        }
        else
        {
            // Player was started up, so lets see what the state is...
            g_InputController.AllowControllerInterrupts(false);
            if (_iSeq == 0xff)                    // User told us to abort
            {
                SSCSerial.println(F("PL0"));
                _fGPActive=false;
            }
            else
            {
                SSCSerial.print(F("QPL0\r"));
                                                  //    [GPStatSeq, GPStatFromStep, GPStatToStep, GPStatTime]
                SSCRead((byte*)abStat, sizeof(abStat), 10000, -1);

                g_bGPCurStep = abStat[1];
                if ((g_bGPCurStep == (g_bGPCntSteps-1)) && (abStat[3] == 0))
                {
                    // We are done
                    SSCSerial.println(F("PL0"));
                    _fGPActive=false;
                }
                else if (g_fGPSMChanged)
                {
                    g_fGPSMChanged = false;
                    SSCSerial.print(F("PL0SM"));
                    SSCSerial.println(_sGPSM, DEC);
                }
            }
                                                  // Ok to process hserial again...
            g_InputController.AllowControllerInterrupts(true);
        }
    }
}


//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
uint8_t ServoDriver::GPNumSteps(void)             // How many steps does the current sequence have
{
    word wSeqStart;
    uint8_t bGPCntSteps = 0xff;                   // assume an error
    byte cbRead;
                                                  // If on xbee on hserial tell hserial to not processess...
    g_InputController.AllowControllerInterrupts(false);

    // Output command to ssc
    SSCSerial.print(F("EER -"));
    SSCSerial.print(_iSeq, DEC);
    SSCSerial.println(F(";2"));
                                                  // Try to get the pointer to sequence
    cbRead = SSCRead((byte*)&wSeqStart, sizeof(wSeqStart), 10000, -1);

    if ((cbRead == sizeof(wSeqStart)) && (wSeqStart != 0) && (wSeqStart != 0xffff))
    {
        // Now try to read in the count of steps from the start of the sequence.
        // Output command to ssc
        SSCSerial.print(F("EER -"));
        SSCSerial.print(wSeqStart, DEC);
        SSCSerial.println(F(";1"));
                                                  // Try to get the pointer to sequence
        cbRead = SSCRead((byte*)&bGPCntSteps, sizeof(bGPCntSteps), 10000, -1);
    }
    return bGPCntSteps;
}


//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
uint8_t ServoDriver::GPCurStep(void)              // Return which step currently on...
{
    return _fGPActive ? g_bGPCurStep : 0xff;
}


//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void ServoDriver::GPSetSpeedMultiplyer(short sm)  // Set the Speed multiplier (100 is default)
{
    if (_sGPSM != sm)
    {
        _sGPSM = sm;
        g_fGPSMChanged = true;
    }
}
#endif                                            // OPT_GPPLAYER



//------------------------------------------------------------------------------------------
//[BeginServoUpdate] Does whatever preperation that is needed to starrt a move of our servos
//------------------------------------------------------------------------------------------
void ServoDriver::BeginServoUpdate(void)          // Start the update
{
	g_fSSC32ServoChanged = false;	// assume nothing changed.
}


//------------------------------------------------------------------------------------------
//[OutputServoInfoForLeg] Do the output to the SSC-32 for the servos associated with
//         the Leg number passed in.
//------------------------------------------------------------------------------------------
#define cPwmDiv       991                         //old 1059;
#define cPFConst      592                         //old 650 ; 900*(1000/cPwmDiv)+cPFConst must always be 1500
// A PWM/deg factor of 10,09 give cPwmDiv = 991 and cPFConst = 592
// For a modified 5645 (to 180 deg travel): cPwmDiv = 1500 and cPFConst = 900.

#ifdef c4DOF
void ServoDriver::OutputServoInfoForLeg(byte LegIndex, short sCoxaAngle1, short sFemurAngle1, short sTibiaAngle1, short sTarsAngle1)
#else
void ServoDriver::OutputServoInfoForLeg(byte LegIndex, short sCoxaAngle1, short sFemurAngle1, short sTibiaAngle1)
#endif
{
    word    wCoxaSSCV;                            // Coxa value in SSC units
    word    wFemurSSCV;                           //
    word    wTibiaSSCV;                           //
#ifdef c4DOF
    word    wTarsSSCV;                            //
#endif

	byte	bServoSaveIndex = LegIndex * NUMSERVOSPERLEG;

    // The Main code now takes care of the inversion before calling.
    wCoxaSSCV = ((long)(sCoxaAngle1 +900))*1000/cPwmDiv+cPFConst;
    wFemurSSCV = ((long)((long)(sFemurAngle1+900))*1000/cPwmDiv+cPFConst);
    wTibiaSSCV = ((long)(sTibiaAngle1+900))*1000/cPwmDiv+cPFConst;
#ifdef c4DOF
    wTarsSSCV = ((long)(sTarsAngle1+900))*1000/cPwmDiv+cPFConst;
#endif

	if (g_awSSC32ServoPos[bServoSaveIndex] != wCoxaSSCV) 
    {
#ifdef cSSC_BINARYMODE
        SSCSerial.write(pgm_read_byte(&cCoxaPin[LegIndex])  + 0x80);
        SSCSerial.write(wCoxaSSCV >> 8);
        SSCSerial.write(wCoxaSSCV & 0xff);
#else
        SSCSerial.print("#");
        SSCSerial.print(pgm_read_byte(&cCoxaPin[LegIndex]), DEC);
        SSCSerial.print("P");
        SSCSerial.print(wCoxaSSCV, DEC);
#endif
        g_awSSC32ServoPos[bServoSaveIndex] = wCoxaSSCV;
        g_fSSC32ServoChanged = true;
    }
	if (g_awSSC32ServoPos[bServoSaveIndex+1] != wFemurSSCV) 
    {
#ifdef cSSC_BINARYMODE
        SSCSerial.write(pgm_read_byte(&cFemurPin[LegIndex]) + 0x80);
        SSCSerial.write(wFemurSSCV >> 8);
        SSCSerial.write(wFemurSSCV & 0xff);
#else
        SSCSerial.print("#");
        SSCSerial.print(pgm_read_byte(&cFemurPin[LegIndex]), DEC);
        SSCSerial.print("P");
        SSCSerial.print(wFemurSSCV, DEC);
#endif
        g_awSSC32ServoPos[bServoSaveIndex+1] = wFemurSSCV;
        g_fSSC32ServoChanged = true;
    }
	if (g_awSSC32ServoPos[bServoSaveIndex+2] != wTibiaSSCV) 
    {
#ifdef cSSC_BINARYMODE
        SSCSerial.write(pgm_read_byte(&cTibiaPin[LegIndex]) + 0x80);
        SSCSerial.write(wTibiaSSCV >> 8);
        SSCSerial.write(wTibiaSSCV & 0xff);
#else
        SSCSerial.print("#");
        SSCSerial.print(pgm_read_byte(&cTibiaPin[LegIndex]), DEC);
        SSCSerial.print("P");
        SSCSerial.print(wTibiaSSCV, DEC);
#endif
        g_awSSC32ServoPos[bServoSaveIndex+2] = wTibiaSSCV;
        g_fSSC32ServoChanged = true;
    }
#ifdef c4DOF
    if ((byte)pgm_read_byte(&cTarsLength[LegIndex]))
    {
        if (g_awSSC32ServoPos[bServoSaveIndex+3] != wTarsSSCV) 
        {
#ifdef cSSC_BINARYMODE
            SSCSerial.write(pgm_read_byte(&cTarsPin[LegIndex]) + 0x80);
            SSCSerial.write(wTarsSSCV >> 8);
            SSCSerial.write(wTarsSSCV & 0xff);
#else
            SSCSerial.print("#");
            SSCSerial.print(pgm_read_byte(&cTarsPin[LegIndex]), DEC);
            SSCSerial.print("P");
            SSCSerial.print(wTarsSSCV, DEC);
#endif
            g_awSSC32ServoPos[bServoSaveIndex+3] = wTarsSSCV;
            g_fSSC32ServoChanged = true;
        }
    }
#endif
                                                  // Ok for hserial again...
    g_InputController.AllowControllerInterrupts(true);
}

//--------------------------------------------------------------------
//[CommitServoDriver Updates the positions of the servos - This outputs
//         as much of the command as we can without committing it.  This
//         allows us to once the previous update was completed to quickly
//        get the next command to start
//--------------------------------------------------------------------
boolean ServoDriver::CommitServoDriver(word wMoveTime)
{
#ifdef cSSC_BINARYMODE
    byte    abOut[3];
#endif
    if (g_fSSC32ServoChanged)       // only do the commit if we actually output something!
    {
                                                  // If on xbee on hserial tell hserial to not processess...
        g_InputController.AllowControllerInterrupts(false);

#ifdef cSSC_BINARYMODE
        abOut[0] = 0xA1;
        abOut[1] = wMoveTime >> 8;
        abOut[2] = wMoveTime & 0xff;
        SSCSerial.write(abOut, 3);
#else
        //Send <CR>
        SSCSerial.print("T");
        SSCSerial.println(wMoveTime, DEC);
#endif

        g_InputController.AllowControllerInterrupts(true);
        _fServosActive = true;                        // Remember we have actually output something to the servos...
    }
    return g_fSSC32ServoChanged;
}


//--------------------------------------------------------------------
//[FREE SERVOS] Frees all the servos
//--------------------------------------------------------------------
void ServoDriver::FreeServos(void)
{
    if (_fServosActive)
    {
        byte LegIndex;                           //If on xbee on hserial tell hserial to not processess...
        g_InputController.AllowControllerInterrupts(false);
        for (LegIndex = 0; LegIndex < 32; LegIndex++)
        {
            SSCSerial.print("#");
            SSCSerial.print(LegIndex, DEC);
            SSCSerial.print("P0");
        }
        SSCSerial.print("T200\r");
        g_InputController.AllowControllerInterrupts(true);
        _fServosActive = false;                   // remember we turned off the servos...
        
        // Also clear away all saved servo positions as they are not valid any more
        for (LegIndex = 0; LegIndex < NUMSERVOS; LegIndex++)
        g_awSSC32ServoPos[LegIndex] = 0;	  // save away previous servo positions...
    }
}

//--------------------------------------------------------------------
//Function that gets called from the main loop if the robot is not logically
//     on.  Gives us a chance to play some...
//--------------------------------------------------------------------
void ServoDriver::IdleTime(void)
{

}

#ifdef OPT_TERMINAL_MONITOR
extern void FindServoOffsets(void);
extern void SSCForwarder(void);

//==============================================================================
// ShowTerminalCommandList: Allow the Terminal monitor to call the servo driver
//      to allow it to display any additional commands it may have.
//==============================================================================
void ServoDriver::ShowTerminalCommandList(void)
{
#ifdef OPT_FIND_SERVO_OFFSETS
    DBGSerial.println(F("O - Enter Servo offset mode"));
#endif
#ifdef OPT_SSC_FORWARDER
    DBGSerial.println(F("S - SSC Forwarder"));
#endif
}


//==============================================================================
// ProcessTerminalCommand: The terminal monitor will call this to see if the
//     command the user entered was one added by the servo driver.
//==============================================================================
boolean ServoDriver::ProcessTerminalCommand(byte *psz, byte bLen)
{
#ifdef OPT_FIND_SERVO_OFFSETS
    if ((bLen == 1) && ((*psz == 'o') || (*psz == 'O')))
    {
        FindServoOffsets();
    }
#endif
#ifdef OPT_SSC_FORWARDER
    if ((bLen == 1) && ((*psz == 's') || (*psz == 'S')))
    {
        SSCForwarder();
    }
#endif
    return true;                                  // Currently not using the return value

}


//==============================================================================
// SSC Forwarder - used to allow things like Lynxterm to talk to the SSC-32
// through the Arduino...  Will see if it is fast enough...
//==============================================================================
#ifdef OPT_SSC_FORWARDER
void  SSCForwarder(void)
{
    int sChar;
    int sPrevChar;
    DBGSerial.println("SSC Forwarder mode - Enter $<cr> to exit");

    for (;;)
    {
        if ((sChar = DBGSerial.read()) != -1)
        {
            SSCSerial.write(sChar & 0xff);
            if (((sChar == '\n') || (sChar == '\r')) && (sPrevChar == '$'))
                break;                            // exit out of the loop
            sPrevChar = sChar;
        }

        if ((sChar = SSCSerial.read()) != -1)
        {
            DBGSerial.write(sChar & 0xff);
        }
    }
    DBGSerial.println("Exited SSC Forwarder mode");
}
#endif                                            // OPT_SSC_FORWARDER
//==============================================================================
//	FindServoOffsets - Find the zero points for each of our servos...
// 		Will use the new servo function to set the actual pwm rate and see
//		how well that works...
//==============================================================================
#ifdef OPT_FIND_SERVO_OFFSETS
#ifndef NUMSERVOSPERLEG
#define NUMSERVOSPERLEG 3
#endif

void FindServoOffsets()
{
    // not clean but...
    byte abSSCServoNum[NUMSERVOSPERLEG*CNT_LEGS]; // array of servos...
                                                  // we have 18 servos to find/set offsets for...
    signed short asOffsets[NUMSERVOSPERLEG*CNT_LEGS];
                                                  // array for our read in servos...
    signed char asOffsetsRead[NUMSERVOSPERLEG*CNT_LEGS];

    static char *apszLegs[] =
    {
#ifdef QUADMODE
        "RR","RF", "LR", "LF"                     // Leg Order
#else
        "RR","RM","RF", "LR", "LM", "LF"          // Leg Order
#endif
    };

    static char *apszLJoints[] =                  // which joint on the leg...
    {
        " Coxa", " Femur", " Tibia", " tArs"
    };

    byte szTemp[5];
    byte cbRead;

    int data;
    short sSN ;                                   // which servo number
    boolean fNew = true;                          // is this a new servo to work with?
    boolean fExit = false;                        // when to exit

    if (CheckVoltage())
    {
        // Voltage is low...
        DBGSerial.println("Low Voltage: fix or hit $ to abort");
        while (CheckVoltage())
        {
            if (DBGSerial.read() == '$')  return;
        }
    }

    // Fill in array of SSC-32 servo numbers
    for (sSN=0; sSN < CNT_LEGS; sSN++)            // Make sure all of our servos initialize to 0 offset from saved.
    {
        abSSCServoNum[sSN*NUMSERVOSPERLEG + 0] = pgm_read_byte(&cCoxaPin[sSN]);
        abSSCServoNum[sSN*NUMSERVOSPERLEG + 1] = pgm_read_byte(&cFemurPin[sSN]);
        abSSCServoNum[sSN*NUMSERVOSPERLEG + 2] = pgm_read_byte(&cTibiaPin[sSN]);
#ifdef c4DOF
        abSSCServoNum[sSN*NUMSERVOSPERLEG + 3] = pgm_read_byte(&cTarsPin[sSN]);
#endif
    }

    // now lets loop through and get information and set servos to 1500
    for (sSN=0; sSN < CNT_LEGS*NUMSERVOSPERLEG; sSN++ )
    {
        asOffsets[sSN] = 0;
        asOffsetsRead[sSN] = 0;

        SSCSerial.print("R");
        SSCSerial.println(32+abSSCServoNum[sSN], DEC);
        // now read in the current value...
        cbRead = SSCRead((byte*)szTemp, sizeof(szTemp), 10000, 13);
        if (cbRead > 0)
        {
            asOffsetsRead[sSN] = atoi((const char *)szTemp);
            DBGSerial.print(abSSCServoNum[sSN], HEX);
            DBGSerial.print("=");
            DBGSerial.write(szTemp, cbRead);
            DBGSerial.println(" ");
        }

        SSCSerial.print("#");
        SSCSerial.print(abSSCServoNum[sSN], DEC);
        SSCSerial.println("P1500");
    }

    // OK lets move all of the servos to their zero point.
    DBGSerial.println("Find Servo Zeros.\n$-Exit, +- changes, *-change servo");
    DBGSerial.println("    0-n Chooses a leg, C-Coxa, F-Femur, T-Tibia");

    sSN = true;
    while(!fExit)
    {
        if (fNew)
        {
            DBGSerial.print("Servo: ");
            DBGSerial.print(apszLegs[sSN/NUMSERVOSPERLEG]);
            DBGSerial.print(apszLJoints[sSN%NUMSERVOSPERLEG]);
            DBGSerial.print("(");
            DBGSerial.print(asOffsetsRead[sSN]+asOffsets[sSN], DEC);
            DBGSerial.println(")");

            // Now lets wiggle the servo
            SSCSerial.print("#");
            SSCSerial.print(abSSCServoNum[sSN], DEC);
            SSCSerial.print("P");
            SSCSerial.print(1500+asOffsets[sSN]+250, DEC);
            SSCSerial.println("T250");
            delay(250);

            SSCSerial.print("#");
            SSCSerial.print(abSSCServoNum[sSN], DEC);
            SSCSerial.print("P");
            SSCSerial.print(1500+asOffsets[sSN]-250, DEC);
            SSCSerial.println("T500");
            delay(500);

            SSCSerial.print("#");
            SSCSerial.print(abSSCServoNum[sSN], DEC);
            SSCSerial.print("P");
            SSCSerial.print(1500+asOffsets[sSN], DEC);
            SSCSerial.println("T250");
            delay(250);

            fNew = false;
        }

        //get user entered data
        data = DBGSerial.read();
        //if data received
        if (data !=-1)
        {
            if (data == '$')
                fExit = true;                     // not sure how the keypad will map so give NL, CR, LF... all implies exit

            else if ((data == '+') || (data == '-'))
            {
                if (data == '+')
                    asOffsets[sSN] += 5;          // increment by 5us
                else
                    asOffsets[sSN] -= 5;          // increment by 5us

                DBGSerial.print("    ");
                DBGSerial.println(asOffsetsRead[sSN]+asOffsets[sSN], DEC);

                SSCSerial.print("#");
                SSCSerial.print(abSSCServoNum[sSN], DEC);
                SSCSerial.print("P");
                SSCSerial.print(1500+asOffsets[sSN], DEC);
                SSCSerial.println("T100");
            }
            else if ((data >= '0') && (data <= '5'))
            {
                // direct enter of which servo to change
                fNew = true;
                sSN = (sSN % NUMSERVOSPERLEG) + (data - '0')*NUMSERVOSPERLEG;
            }
            else if ((data == 'c') || (data == 'C'))
            {
                fNew = true;
                sSN = (sSN / NUMSERVOSPERLEG) * NUMSERVOSPERLEG + 0;
            }
            else if ((data == 'f') || (data == 'F'))
            {
                fNew = true;
                sSN = (sSN / NUMSERVOSPERLEG) * NUMSERVOSPERLEG + 1;
            }
            else if ((data == 't') || (data == 'T'))
            {
                // direct enter of which servo to change
                fNew = true;
                sSN = (sSN / NUMSERVOSPERLEG) * NUMSERVOSPERLEG + 2;
            }
            else if (data == '*')
            {
                // direct enter of which servo to change
                fNew = true;
                sSN++;
                if (sSN == CNT_LEGS*NUMSERVOSPERLEG)
                    sSN = 0;
            }
        }
    }

    DBGSerial.print("Find Servo exit ");
    for (sSN=0; sSN < CNT_LEGS*NUMSERVOSPERLEG; sSN++)
    {
        DBGSerial.print("Servo: ");
        DBGSerial.print(apszLegs[sSN/NUMSERVOSPERLEG]);
        DBGSerial.print(apszLJoints[sSN%NUMSERVOSPERLEG]);
        DBGSerial.print("(");
        DBGSerial.print(asOffsetsRead[sSN]+asOffsets[sSN], DEC);
        DBGSerial.println(")");
    }

    DBGSerial.print("\nSave Changes? Y/N: ");

    //get user entered data
    while (((data = DBGSerial.read()) == -1) || ((data >= 10) && (data <= 15)))
        ;

    if ((data == 'Y') || (data == 'y'))
    {
        // Ok they asked for the data to be saved.  We will store the data with a
        // number of servos (byte)at the start, followed by a byte for a checksum...followed by our offsets array...
        // Currently we store these values starting at EEPROM address 0. May later change...
        //

        for (sSN=0; sSN < CNT_LEGS*NUMSERVOSPERLEG; sSN++ )
        {
            SSCSerial.print("R");
            SSCSerial.print(32+abSSCServoNum[sSN], DEC);
            SSCSerial.print("=");
            SSCSerial.println(asOffsetsRead[sSN]+asOffsets[sSN], DEC);
            delay(10);
        }

        // Then I need to have the SSC-32 reboot in order to use the new values.
        delay(10);                                // give it some time to write stuff out.
        SSCSerial.println("GOBOOT");
        delay(5);                                 // Give it a little time
        SSCSerial.println("g0000");               // tell it that we are done in the boot section so go run the normall SSC stuff...
        delay(500);                               // Give it some time to boot up...

    }

    else
    {
        void LoadServosConfig();
    }

    g_ServoDriver._fServosActive = true;  // let it know that servos are active...
    g_ServoDriver.FreeServos();

}
#endif                                            // OPT_FIND_SERVO_OFFSETS
#endif                                            //
