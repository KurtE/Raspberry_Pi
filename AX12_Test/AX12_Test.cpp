// Kurts Test program to try out different ways to manipulate the AX12 servos on the PhantomX
// This is a test, only a test...
//====================================================================================================
//============================================================================
// Global Include files
//=============================================================================
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdarg.h>
#include <time.h>
#include <inttypes.h>
#include <signal.h>
#include "ax12.h"
#include "BioloidEX.h"
#include "dynamixel.h"
#if defined(_WIN32) || defined(_WIN64)
#include <conio.h>
#include <Windows.h>
#using <System.dll>
#else
#include <termios.h>
#include <sys/time.h>
#include <unistd.h>
#include "ArduinoDefs.h"
#include "WrapperSerial.h"
#endif


//=============================================================================
// Options...
//=============================================================================

// Uncomment the next line if building for a Quad instead of a Hexapod.
//#define HROS1_MODE
//#define QUAD_MODE
//#define TURRET
#define SERVO1_SPECIAL

#define MAX_SERVOS  32

//=============================================================================
// Define differnt robots..
//=============================================================================

// Constants
// use for Track FSRs IDS 111 and 112
#define FSR_FIRST_ID 111
#define CNT_FSR_SERVOS 2
#define FSR_FIRST_REG  0X1A
#define CNT_FSR_VALS 10

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
            "R_ELBOW",  "L_ELBOW", "R_HIP_YAW", "L_HIP_YAW",
            "R_HIP_ROLL", "L_HIP_ROLL", "R_HIP_PITCH", "L_HIP_PITCH",
            "R_KNEE", "L_KNEE", "R_ANKLE_PITCH", "L_ANKLE_PITCH",
            "R_ANKLE_ROLL", "L_ANKLE_ROLL", "HEAD_PAN", "HEAD_TILT",
        };
int     g_hros1_count_servos = (sizeof(g_hros1_pin_table) / sizeof(g_hros1_pin_table[0]));

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

uint8_t     g_showHelp = 1;     // should we show help command?
#if defined(_WIN32) || defined(_WIN64)
uint16_t    g_comport_index = 9;    //
#else
WrapperSerial Serial = WrapperSerial();
#endif
// other globals.
uint16_t       g_wVoltage;
char           g_aszCmdLine[120];
uint8_t        g_iszCmdLine;
uint8_t        g_fTrackServos = false;
uint8_t        g_fTrackFSRs = false;
uint8_t        g_fSignaled = false;

int            g_asPositionsPrev[MAX_SERVOS];
int            g_asMins[MAX_SERVOS];
int            g_asMaxs[MAX_SERVOS];
uint8_t        g_abServos[MAX_SERVOS];             // list of servos found in last scan
uint8_t        g_cServosScanned = 0xff;            // count scanned
uint8_t        g_cServosNamed;                     // Size of our name table

// Variables used for FSR Tracking
uint8_t        g_abFSRVals[CNT_FSR_SERVOS][CNT_FSR_VALS];


// Values to use for servo position...
uint8_t        g_bServoID;
uint16_t       g_wServoGoalPos;
uint16_t       g_wServoGoalSpeed;

// forward references
extern void setup(void);
extern void loop(void);


extern void AllServosOff(void);
extern uint8_t GetNextCommandLine(void);
extern uint8_t FGetNextCmdNum(uint16_t *pw);
extern void AllServosCenter(void);
extern void HoldOrFreeServos(uint8_t fHold);
extern void SetServoPosition(void) ;
extern void SetServoID(void);
extern void WaitForMoveToComplete(uint16_t wID);
extern void GetServoPositions(void);
extern void SyncReadServoPositions(void);
extern void TrackServos(uint8_t fInit);
extern void TrackFSRs(uint8_t fInit);
extern void TrackPrintMinsMaxs(void);
extern void PrintServoValues(void);
extern void SetServosReturnDelay(void);
extern void WriteServoRegisters(void);
extern void PanServoTest(void);
extern void ScanAllServos(void);
extern void IMUTest(void);
extern void SetupDefaultServoListForRobot(char bMode);

#if defined(_WIN32) || defined(_WIN64)
// Building under windows
#define true 1
#define false 0
extern unsigned long micros(void);
#define delay(time) Sleep(time)
//====================================================================================================
// SignalHandler - Try to free up things like servos if we abort.
//====================================================================================================
BOOL CtrlHandler(DWORD fdwCtrlType)
{
    switch (fdwCtrlType)
    {
        // Handle the CTRL-C signal.
    case CTRL_C_EVENT:
        printf("Ctrl-C event\n\n");
        Beep(750, 300);
        g_fSignaled = true;

        return(TRUE);

        // CTRL-CLOSE: confirm that the user wants to exit.
    case CTRL_CLOSE_EVENT:
        Beep(600, 200);
        printf("Ctrl-Close event\n\n");
        return(TRUE);

        // Pass other signals to the next handler.
    case CTRL_BREAK_EVENT:
        Beep(900, 200);
        printf("Ctrl-Break event\n\n");
        return FALSE;

    case CTRL_LOGOFF_EVENT:
        Beep(1000, 200);
        printf("Ctrl-Logoff event\n\n");
        return FALSE;

    case CTRL_SHUTDOWN_EVENT:
        Beep(750, 500);
        printf("Ctrl-Shutdown event\n\n");
        return FALSE;

    default:
        return FALSE;
    }
}
#else
// Building under linux
//====================================================================================================
// SignalHandler - Try to free up things like servos if we abort.
//====================================================================================================
void SignalHandler(int sig){
    printf("Caught signal %d\n", sig);

    if (g_fSignaled) {
        printf("Second signal Abort\n");
        exit(1);
    }
    // Set global telling main to abort and return
    g_fSignaled = true;

}
#endif

//--------------------------------------------------------------------------
// Main: the main  function.
//--------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    // Install signal handler to allow us to do some cleanup...
#if defined(_WIN32) || defined(_WIN64)
    // Windows
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);

 #else
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = SignalHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);
#endif
    printf("Start\n");
    g_cServosNamed = g_count_servos;

#if defined(_WIN32) || defined(_WIN64)
    if (argc <= 1)
    {
        using namespace System;
        using namespace System::IO::Ports;
        using namespace System::ComponentModel;
        array<String^>^ serialPorts = nullptr;
        try
        {
            // Get a list of serial port names.
            serialPorts = SerialPort::GetPortNames();
        }
        catch (Win32Exception^ ex)
        {
            Console::WriteLine(ex->Message);
        }

        Console::WriteLine("The following serial ports were found:");

        // Display each port name to the console.
        for each(String^ port in serialPorts)
        {
            Console::WriteLine(port);
        }
        printf("Enter COM port number: ");
        GetNextCommandLine();
        g_iszCmdLine = 0;
        FGetNextCmdNum(&g_comport_index);
    }
#endif
    if (argc > 1)
    {
       for (int i=1; i < argc; i++)
        {
            printf("%d - %s\n", i, argv[i]);
        }
        // See if hex, quad, biped (only look at first char)
        char c = *(argv[1]);
        SetupDefaultServoListForRobot(c);
    }

    setup();

    while (!g_fSignaled)
    {
        //--------------------------------------------------------------------------
        // Loop: the main arduino main Loop function
        //--------------------------------------------------------------------------
        loop();
    }

    // Abort signaled...
    AllServosOff();
    printf("All Servos off\n");

    // If on 200 we may have to turn power off...
    if (g_id_controller== 200) {
        ax12SetRegister(g_id_controller, AX_TORQUE_ENABLE, 0x0);
        printf("Controller turn power off\n");
    }
    exit(1);
}


//====================================================================================================
// Setup
//====================================================================================================
void setup() {
#if defined(_WIN32) || defined(_WIN64)
    if (dxl_initialize(g_comport_index, 1) == 0)
#else
    Serial.begin();  // start off the serial port.
    //if(dxl_initialize(0, 0) == 0)
    if(dxl_initializeBaud(0, 4000000.0) == 0)
#endif
    {
        printf("Failed to open USBDynamixel\n");
    }

    // First lets try to see what controller we are using, lets first try id 253 which is default USB2AX
    delay(1000);
    uint16_t  wModel = ax12GetRegister(g_id_controller, AX_MODEL_NUMBER_L, 2);
    printf("id: %x wModel: %x\n", g_id_controller, wModel);
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

    printf("System Voltage in 10ths: %d\n", ax12GetRegister(1, AX_PRESENT_VOLTAGE, 1));
}

//====================================================================================================
// SetupDefaultServoListForRobot
//====================================================================================================
void SetupDefaultServoListForRobot(char bMode)
{
    switch (bMode)
    {

    case 'q':
    case 'Q':
        printf("Quad\n");
        g_count_servos = (sizeof(g_quad_pin_table) / sizeof(g_quad_pin_table[0]));
        g_servo_id_table = g_quad_pin_table;
        g_servo_name_table = g_quad_pin_names;
#ifdef TURRET
        if (g_aszCmdLine[0] == 'q')
        {
            printf("No turret\n");
            g_count_servos -= 2;
        }
#endif
        g_cServosNamed = g_count_servos;
        break;
    case 'x':
    case 'X':
    case 'h':
    case 'H':
        printf("Hexapod\n");
        g_count_servos = (sizeof(g_hex_pin_table) / sizeof(g_hex_pin_table[0]));
        g_servo_id_table = g_hex_pin_table;
        g_servo_name_table = g_hex_pin_names;
#ifdef TURRET
        if (g_aszCmdLine[0] == 'x')
        {
            printf("No turret\n");
            g_count_servos -= 2;
        }
#endif
        g_cServosNamed = g_count_servos;
    case 'b':
    case 'B':
        printf("Biped - HROS1\n");
        g_count_servos = (sizeof(g_hros1_pin_table) / sizeof(g_hros1_pin_table[0]));
        g_servo_id_table = g_hros1_pin_table;
        g_servo_name_table = g_hros1_pin_names;
        g_cServosNamed = g_count_servos;

        break;
    }
}

//====================================================================================================
void FlushInputFromTerminal(void)
{
    // lets toss any charcters that are in the input queue
#if defined(_WIN32) || defined(_WIN64)
    while (_kbhit())
    {
        char c = _getch();
        // act on character c in whatever way you want
    }
#else
    while(Serial.available() )
        Serial.read();
#endif
}

int GetCharFromTerminal(void)
{
    // lets toss any charcters that are in the input queue
#if defined(_WIN32) || defined(_WIN64)
    if (_kbhit())
    {
        return  _getch();
        // act on character c in whatever way you want
    }
    return -1;
#else
    return Serial.read();
#endif
}

//====================================================================================================
// Loop
//====================================================================================================
void loop() {
    // Output a prompt
    uint16_t wNewVoltage = ax12GetRegister(1, AX_PRESENT_VOLTAGE, 1);
    if (wNewVoltage != g_wVoltage) {
        g_wVoltage = wNewVoltage;
        printf("System Voltage in 10ths: %d\n", g_wVoltage);
    }

    // lets toss any charcters that are in the input queue
    FlushInputFromTerminal();

    if (g_showHelp)
    {

        printf("0 - All Servos off\n\r");
        printf("1 - All Servos center\n\r");
        printf("2 - Set Servo position [<Servo>] <Position> [<Speed>]\n\r");
        printf("3 - Set Servo Angle\n\r");
        printf("4 - Get Servo Positions\n\r");
        printf("5 - Sync Read Servo positions\n\r");
        printf("7 - Set Servos return delay\n\r");
        printf("8 - Set ID: <old> <new>\n\r");
        printf("9 - Print Servo Values\n\r");
        printf("t - Toggle track Servos\n\r");
        printf("w - write <servo> <reg> <val> (can be multiple values...\n\r");
        printf("p - Pan servo start end step\n\r");
        printf("s - Scan for all servos\n\r");
        printf("r - HROS1 FSR track\n\r");
        printf("i - Test IMU\n\r");
        printf("h - hold [<sn>]\n\r");
        printf("f - free [<sn>]\n\r");
        printf("b - Biped mode\n\r");
        printf("Q - Set Quad mode\n\r");
        printf("X - Set Hex mode\n\r");
        g_showHelp = false;
    }
    printf("\n\r: ");
#if defined(_WIN32) || defined(_WIN64)
#else
    Serial.flush();
#endif
    // Get a command
    if (GetNextCommandLine()) {
        printf("\n\r");
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

        case 'i':
        case 'I':
            IMUTest();
            break;

        case 'b':
        case 'B':
        case 'q':
        case 'Q':
        case 'x':
        case 'X':
            SetupDefaultServoListForRobot(g_aszCmdLine[0]);
            break;

        case 'w':
        case 'W':
            WriteServoRegisters();
            break;

        case 'p':
        case 'P':
            PanServoTest();
            break;

        case 's':
        case 'S':
            ScanAllServos();
            break;

        case 't':
        case 'T':
            g_fTrackServos = !g_fTrackServos;
            if (g_fTrackServos)
            {
                printf("Tracking On\n\r");
                TrackServos(true);  // call to initialize all of the positions.
            }
            else
            {
                printf("Tracking Off\n\r");
                TrackPrintMinsMaxs();
            }
            break;
        case 'r':
        case 'R':
            g_fTrackFSRs = !g_fTrackFSRs;
            if (g_fTrackFSRs)
            {
                printf("FSR Tracking On\n\r");
                TrackFSRs(true);  // call to initialize all of the positions.
            }
            else
                printf("FSR Tracking Off\n\r");
            break;

        }
    }
    else
        g_showHelp = true;

}

// Helper function to read in a command line
uint8_t GetNextCommandLine(void) {
  int ch;
  uint8_t ich = 0;
  g_iszCmdLine = 0;

  while (!g_fSignaled)
  {
    // throw away any thing less than CR character...
    while ((ch = GetCharFromTerminal()) != -1)
    {
        if ((ch >= 10) && (ch <=15))
        {
          g_aszCmdLine[ich] = 0;
          return ich;
        }
        if (ch != -1)
          g_aszCmdLine[ich++] = ch;
    }
    if (g_fTrackServos)
        TrackServos(false);
    else if (g_fTrackFSRs)
        TrackFSRs(false);
    else
       delay(100);
  }
  return 0; // We were signalled
}

//
uint8_t FGetNextCmdNum(uint16_t *pw)
{
  // Skip all leading num number characters...
    while ((g_aszCmdLine[g_iszCmdLine] < '0') || (g_aszCmdLine[g_iszCmdLine] > '9'))
    {
        if (g_aszCmdLine[g_iszCmdLine] == 0)
            return false;  // end of the line...
        g_iszCmdLine++;
    }
    *pw = 0;
    while ((g_aszCmdLine[g_iszCmdLine] >= '0') && (g_aszCmdLine[g_iszCmdLine] <= '9'))
    {
        *pw = *pw * 10 + (g_aszCmdLine[g_iszCmdLine] - '0');
        g_iszCmdLine++;
    }
    return true;
}

//=======================================================================================
void SetServosReturnDelay(void)
{
    // Allow user to pass in, default 0
    uint16_t wDelay;

    if (!FGetNextCmdNum(&wDelay ))
        wDelay = 0;
    wDelay &= 0xff; // make sure in one byte

    for (int i = 0; i < g_count_servos; i++)
    {
        ax12SetRegister(g_servo_id_table[i], AX_RETURN_DELAY_TIME, wDelay);
    }
}


//=======================================================================================
void AllServosOff(void)
{
    for (int i = 0; i < g_count_servos; i++)
    {
        ax12SetRegister(g_servo_id_table[i], AX_TORQUE_ENABLE, 0x0);
    }
}


//=======================================================================================
void AllServosCenter(void)
{
    for (int i = 0; i < g_count_servos; i++)
    {
        // See if this turns the motor off and I can turn it back on...
        ax12SetRegister(g_servo_id_table[i], AX_TORQUE_ENABLE, 0x1);
        ax12SetRegister2(g_servo_id_table[i], AX_GOAL_POSITION_L, 0x1ff);
    }
}
//=======================================================================================
void HoldOrFreeServos(uint8_t fHold)
{
    uint16_t iServo;

    if (!FGetNextCmdNum(&iServo)) {
        // All servos...
        for (int i = 0; i < g_count_servos; i++)
        {
           ax12SetRegister(g_servo_id_table[i], AX_TORQUE_ENABLE, fHold);
        }
    }
    else
    {
        ax12SetRegister(iServo, AX_TORQUE_ENABLE, fHold);
    }
}

//=======================================================================================

//=======================================================================================
void SetServoPosition(void)
{
    uint16_t w1;
    uint16_t w2;

    if (!FGetNextCmdNum(&w1))
        return;    // no parameters so bail.

    printf("Set Servo Position\n\r");
    if (FGetNextCmdNum(&w2))
    {
        // We have at least 2 parameters
        g_bServoID = (uint8_t)w1;    // So first is which servo
        g_wServoGoalPos = w2;
        if (FGetNextCmdNum(&w2))
        {
            // We have at least 3 parameters
            g_wServoGoalSpeed = w2;
            ax12SetRegister2(g_bServoID, AX_GOAL_SPEED_L, g_wServoGoalSpeed);
            printf("Goal Speed: %d\n", g_wServoGoalSpeed);
        }
    }
    else
        g_wServoGoalPos = w1;  // Only 1 paramter so assume it is the new position

    // Now lets try moving that servo there
    ax12SetRegister2(g_bServoID, AX_GOAL_POSITION_L, g_wServoGoalPos);
    printf(" ID: %d %d\n", g_bServoID, g_wServoGoalPos);
}

//=======================================================================================
void SetServoID(void)
{
    uint16_t w1;
    uint16_t w2;

    if (!FGetNextCmdNum(&w1))
        return;    // no parameters so bail.

    if (!FGetNextCmdNum(&w2))
        return;    // no parameters so bail.

    printf("Set Servo ID From: %d %d\n", w1, w2);

    // Now lets try moving that servo there
    ax12SetRegister(w1, AX_ID, w2);
}


//=======================================================================================
void WaitForMoveToComplete(uint16_t wID)
{
    do
    {
        //    delay(1);
    } while (ax12GetRegister(wID, AX_MOVING, 1));
}


//=======================================================================================
void PrintServoIDForIndex(int servo_index)
{
    if (servo_index < g_cServosNamed)
    {
        printf("%d(%s): ", g_servo_id_table[servo_index], g_servo_name_table[servo_index]);
    }
    else
    {
        printf("%d: ", g_servo_id_table[servo_index]);
    }
}
//=======================================================================================
void GetServoPositions(void)
{

    unsigned long ulBefore;
    unsigned long ulDelta;
    unsigned long ulTotalDelta = 0;
    int cntRead=0;
    int w;
    for (int i = 0; i < g_count_servos; i++)
    {
        PrintServoIDForIndex(i);
        ulBefore = micros();
        w = ax12GetRegister(g_servo_id_table[i], AX_PRESENT_POSITION_L, 2 );
        ulDelta = micros() - ulBefore;
        if (dxl_get_result() == COMM_RXSUCCESS)
        {
            cntRead++;
            ulTotalDelta += ulDelta;
        }
        printf("%d %u %d\n", w, (unsigned int)ulDelta, ax12GetRegister(g_servo_id_table[i], AX_RETURN_DELAY_TIME, 1));
        if (w == 0xffff)
        {
            printf("   Retry: %d\n", ax12GetRegister(g_servo_id_table[i], AX_PRESENT_POSITION_L, 2));
        }
    }
    if (cntRead)
    {
        printf("Cnt: %d TotalTime: %u Avg: %u\n\r", cntRead, (unsigned int)ulTotalDelta, (unsigned int)(ulTotalDelta / cntRead));
    }
}

//=======================================================================================
void ScanAllServos(void)
{

    int w;
    printf("\nStart Servo Scan\n");
    g_cServosScanned = 0;
    for (int i = 1; i < 254; i++)
    {
        dxl_ping(i);
        if (dxl_get_result() == COMM_RXSUCCESS)
        {
            w = ax12GetRegister(i, AX_PRESENT_POSITION_L, 2);
            printf("%d = %d\n\r", i, w);
            if ((i != 200) && (i != 253))
            {
                g_abServos[g_cServosScanned++] = i;
            }
        }
    }
    printf("Scan Complete\n\n");

    // For this version just setup to use these pins..
    g_count_servos = g_cServosScanned;
    g_servo_id_table = g_abServos;
    g_cServosNamed = 0;             // have no named servos

}



//=======================================================================================
#define AX_CMD_SYNC_READ      0x84
#define PARAMETER             (5)
void SyncReadServoPositions(void)
{

  unsigned long ulBefore;
  unsigned long ulDelta;
  int w;

    // Setup DXL servo packet
    // update each servo
    dxl_set_txpacket_id(g_id_controller);
    dxl_set_txpacket_instruction(INST_SYNC_READ);
    dxl_set_txpacket_parameter(0, AX_PRESENT_POSITION_L);
    dxl_set_txpacket_parameter(1, 2);

    for (int i = 0; i < g_count_servos; i++)
    {
        dxl_set_txpacket_parameter(2+i, g_servo_id_table[i]);
    }

    dxl_set_txpacket_length(g_count_servos + 4);

    ulBefore = micros();
    dxl_txrx_packet();
    int result = dxl_get_result();   // don't care for now
    ulDelta = micros() - ulBefore;
    printf("Sync Read %d time: %u\n\r", result, (unsigned int)ulDelta);

    // Now print out results
    for (int i = 0; i < g_count_servos; i++)
    {
        PrintServoIDForIndex(i);
        w = dxl_makeword(dxl_get_rxpacket_parameter(i*2), (int)dxl_get_rxpacket_parameter(i*2+1));
        printf("%d\n\r", w);
    }
}


//======================================================================================

void WriteServoRegisters(void)
{
    unsigned long ulBefore;
    unsigned long ulDelta;
    uint16_t wID;
    uint16_t wReg;
    uint16_t w;
    uint8_t cBytes = 0;
    uint8_t ab[10];   // maximum 10 registers, probably never more than 2 or 3...

    if (!FGetNextCmdNum(&wID))
        return;    // no parameters so bail.

    if (!FGetNextCmdNum(&wReg))
        return;    // no parameters so bail.

    while (FGetNextCmdNum(&w) && (cBytes < sizeof(ab)))
    {
        ab[cBytes++] = w & 0xff;
    }

    if (! cBytes)
        return;

    // Now try to set the register(s)...
    dxl_set_txpacket_id(wID);
    dxl_set_txpacket_instruction(INST_WRITE);
    dxl_set_txpacket_parameter(0, wReg);

    for (int i = 0; i < cBytes; i++)
    {
        dxl_set_txpacket_parameter(i+1, ab[i]);
    }

    dxl_set_txpacket_length(3 + cBytes);

    ulBefore = micros();
    dxl_txrx_packet();
    int result = dxl_get_result();   // don't care for now
    ulDelta = micros() - ulBefore;
    printf("Write Register %d %d %d Res=%d time=%lu\n", wID, wReg, cBytes, result, ulDelta);
}

//======================================================================================

void PanServoTest(void)
{
    uint16_t wID;
    int start_pos;
    int  end_pos;
    int Pos;
    int cur_pos;
    int cur_speed;
    int servo_increment = 1;
    int loop_count;
    uint16_t w;
    int dxl_result;

    if (!FGetNextCmdNum(&wID))
        return;    // no parameters so bail.

    if (!FGetNextCmdNum(&w))
        return;    // no parameters so bail.
    start_pos = (int)w;

    if (!FGetNextCmdNum(&w))
        return;    // no parameters so bail.
    end_pos = (int)w;

    if (FGetNextCmdNum(&w))
        servo_increment = (int)w;


    printf("Servo %d Pan test %d - %d step %d\n", wID, start_pos, end_pos, servo_increment);

    ax12SetRegister2(wID, AX_GOAL_POSITION_L, start_pos);
    delay(200); // give some time to get there!

    if (start_pos > end_pos)
        servo_increment = -servo_increment;

    for (Pos = start_pos; ; Pos += servo_increment)
    {
        if ( ((servo_increment > 0) && (Pos > end_pos))
                || ((servo_increment < 0) && (Pos < end_pos)) )
            break;
        ax12SetRegister2(wID, AX_GOAL_POSITION_L, Pos);
        dxl_result = dxl_get_result();

        // Lets wait until we get there
        for (loop_count = 0; loop_count < 50; loop_count++)
        {
            delay(10);
            cur_pos = ax12GetRegister(wID, AX_PRESENT_POSITION_L, 2 );
            if (cur_pos == Pos)
                break;
        }
        if (cur_pos != Pos) {
            cur_speed = ax12GetRegister(wID, AX_PRESENT_SPEED_L, 2 );
            printf("    Goal: %d Actual: %d Speed: %d res=%d\n", Pos, cur_pos, cur_speed, dxl_result);
        }
    }
    printf("Completed\n\n");
}

//=======================================================================================

void TrackServos(uint8_t fInit)
{
#if 1
    int w;
    uint8_t fSomethingChanged = false;
    dxl_set_txpacket_id(g_id_controller);
    dxl_set_txpacket_instruction(INST_SYNC_READ);
    dxl_set_txpacket_parameter(0, AX_PRESENT_POSITION_L);
    dxl_set_txpacket_parameter(1, 2);

    for (int i = 0; i < g_count_servos; i++)
    {
        dxl_set_txpacket_parameter(2 + i, g_servo_id_table[i]);
    }

    dxl_set_txpacket_length(g_count_servos + 4);

    dxl_txrx_packet();
    if (dxl_get_result() == COMM_RXSUCCESS)
    {
        if (fInit)
        {
            // First pass save away current values.
            for (int i = 0; i < g_count_servos; i++)
            {
                w = dxl_makeword(dxl_get_rxpacket_parameter(i * 2), (int)dxl_get_rxpacket_parameter(i * 2 + 1));
                g_asMins[i] = w;
                g_asMaxs[i] = w;
                g_asPositionsPrev[i] = w;

            }
        }
        else
        {
            // Check to see if anything moved.
            for (int i = 0; i < g_count_servos; i++)
            {
                w = dxl_makeword(dxl_get_rxpacket_parameter(i * 2), (int)dxl_get_rxpacket_parameter(i * 2 + 1));
                if (abs(w - g_asPositionsPrev[i]) > 3)
                {
                    PrintServoIDForIndex(i);
                    printf("%d(%d) ", w, (int)((((long)(w - 512)) * 375L) / 128L));
                    fSomethingChanged = true;
                    g_asPositionsPrev[i] = w;
                }
                if (g_asMins[i] > w)
                    g_asMins[i] = w;

                if (g_asMaxs[i] < w)
                    g_asMaxs[i] = w;
            }
        }
    }
    else
    {
        printf("Track: SYNC_READ failed\n\r");
    }

    if (fSomethingChanged)
        printf("\n\r");

#else
    int w;
    uint8_t fSomethingChanged = false;
    for (int i = 0; i < g_count_servos; i++)
    {
        w = ax12GetRegister(g_servo_id_table[i], AX_PRESENT_POSITION_L, 2 );
        if (fInit)
        {
            g_asMins[i] = w;
            g_asMaxs[i] = w;
        }
        if (w != g_asPositionsPrev[i])
        {
            if (!fInit)
            {
                // only print if we moved more than some delta...
                if (abs(w-g_asPositionsPrev[i]) > 3)
                {
                    PrintServoIDForIndex(i);
                    printf("%d(%d) ", w, (int)((((long)(w - 512)) * 375L) / 128L));
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
        printf("\n\r");
#endif
}

//=======================================================================================
void TrackFSRs(uint8_t fInit)
{
    //printf(".");
    for (int fsr_servo_index = 0; fsr_servo_index < CNT_FSR_SERVOS; fsr_servo_index++)
    {
        // Try to read in the FSR Values
        dxl_set_txpacket_id(FSR_FIRST_ID + fsr_servo_index);
        dxl_set_txpacket_instruction(INST_READ);
        dxl_set_txpacket_parameter(0, FSR_FIRST_REG);
        dxl_set_txpacket_parameter(1, CNT_FSR_VALS);
        dxl_set_txpacket_length(4);
        dxl_txrx_packet();
        int result = dxl_get_result();   // don't care for now

        // Now see if anything changed.
        // Now print out results
        uint8_t bChanged = 0;
        if (result == COMM_RXSUCCESS)
        {
            for (int i = 0; i < CNT_FSR_VALS; i++)
            {
                if (dxl_get_rxpacket_parameter(i) != g_abFSRVals[fsr_servo_index][i])
                {
                    g_abFSRVals[fsr_servo_index][i] = dxl_get_rxpacket_parameter(i);
                    bChanged = 1;
                }
            }
            //printf("%d %d ", FSR_FIRST_ID + fsr_servo_index, result);
            if (bChanged || fInit)
            {
                printf("%d(%d) - %d %d %d %d - %d %d\n", FSR_FIRST_ID + fsr_servo_index, result,
                    dxl_makeword(dxl_get_rxpacket_parameter(0), (int)dxl_get_rxpacket_parameter(1)),
                    dxl_makeword(dxl_get_rxpacket_parameter(2), (int)dxl_get_rxpacket_parameter(3)),
                    dxl_makeword(dxl_get_rxpacket_parameter(4), (int)dxl_get_rxpacket_parameter(5)),
                    dxl_makeword(dxl_get_rxpacket_parameter(6), (int)dxl_get_rxpacket_parameter(7)),
                    dxl_get_rxpacket_parameter(8), dxl_get_rxpacket_parameter(9));
            }
        }
        else if (fInit)
        {
            printf("fsr Servo %d status %d\n", FSR_FIRST_ID + fsr_servo_index, result);
        }
    }
}

//=======================================================================================
void TrackPrintMinsMaxs(void)
{
  for (int i = 0; i < g_count_servos; i++)
  {
        printf("%d:%d(%d) %d(%d)\n\r", (uint8_t)g_servo_id_table[i], g_asMins[i],
            (int)((((long)(g_asMins[i] - 512)) * 375L) / 128L),
            g_asMaxs[i], (int)((((long)(g_asMaxs[i] - 512)) * 375L) / 128L));
  }
}


//=======================================================================================
void PrintServoValues(void)
{
#define MAX_BYTES_TO_READ 10
    // Version that does one read ...
    uint16_t wID;
    uint16_t wEndReg;
    uint16_t w;
    if (!FGetNextCmdNum(&wID))
        return;
    if (!FGetNextCmdNum(&wEndReg))
        wEndReg = 50;

    // Lets try breaking this up into reads of up to 10 bytes each time...
    uint16_t first_byte_index = 0;
    uint16_t bytes_to_read = 10;
    int result;
    while (wEndReg)
    {
        bytes_to_read = (wEndReg > MAX_BYTES_TO_READ)? MAX_BYTES_TO_READ : wEndReg;

        dxl_set_txpacket_id(wID);
        dxl_set_txpacket_instruction(INST_READ);
        dxl_set_txpacket_parameter(0, first_byte_index);
        dxl_set_txpacket_parameter(1, bytes_to_read);
        dxl_set_txpacket_length(4);
        dxl_txrx_packet();
        result = dxl_get_result();

        if (result != COMM_RXSUCCESS)
            break;  // abort this loop...
        for (int i = 0; i < bytes_to_read; i++)
        {
            printf("%d:%x ", first_byte_index + i, dxl_get_rxpacket_parameter(i));
            if ((i%10) == 9)
                printf("\n");
        }
        first_byte_index += bytes_to_read;
        wEndReg -= bytes_to_read;
    }

    if (result != COMM_RXSUCCESS)
    {
        printf("dxl bulk Read error: %d on index: %d\n", result, first_byte_index);
        wEndReg += first_byte_index;        // lets restore the last item
        for (int i = first_byte_index; i < wEndReg; i++)
        {
            printf("%d:", i);
            w = dxl_read_byte(wID, i);
            result = dxl_get_result();
            if (result != COMM_RXSUCCESS)
            {
                w = dxl_read_byte(wID, i);
                result = dxl_get_result();
            }
            if (result == COMM_RXSUCCESS)
                printf("%x ", w);
            else
                printf("** ");
            if ((i%10) == 9)
                printf("\n\r");
        }
    }
    printf("\n");
}

//=======================================================================================
// IMUTest - Arbotix-Pro or Teensy Pro, try looping reading IMU values and see when they
// change...
//=======================================================================================
void IMUTest(void)
{
    enum
    {
        CM730_GYRO_Z_L      = 38,
        CM730_GYRO_Z_H      = 39,
        CM730_GYRO_Y_L      = 40,
        CM730_GYRO_Y_H      = 41,
        CM730_GYRO_X_L      = 42,
        CM730_GYRO_X_H      = 43,
        CM730_ACCEL_X_L     = 44,
        CM730_ACCEL_X_H     = 45,
        CM730_ACCEL_Y_L     = 46,
        CM730_ACCEL_Y_H     = 47,
        CM730_ACCEL_Z_L     = 48,
        CM730_ACCEL_Z_H     = 49
    };

    enum
    {
        GYRO_Z = ((CM730_GYRO_Z_L-CM730_GYRO_Z_L)/2),
        GYRO_Y = ((CM730_GYRO_Y_L-CM730_GYRO_Z_L)/2),
        GYRO_X = ((CM730_GYRO_X_L-CM730_GYRO_Z_L)/2),
        ACCEL_Z = ((CM730_ACCEL_Z_L-CM730_GYRO_Z_L)/2),
        ACCEL_Y = ((CM730_ACCEL_Y_L-CM730_GYRO_Z_L)/2),
        ACCEL_X = ((CM730_ACCEL_X_L - CM730_GYRO_Z_L) / 2)
    };

    uint16_t awIMUVals[6] = {0};    // array of values

    if (g_id_controller != 200) {
        printf("*** Only supports CM730 like controllers ***\n\n");
        return;
    }

    printf("\n\n**** Gyro(X, Y, Z), Accel(X, Y, Z) ***\n");
    // Flush out any input characters that are waiting...
    FlushInputFromTerminal();

    // We will stay in this mode until we enter something on keyboard to abort...
    while (GetCharFromTerminal() == -1)
    {
        dxl_set_txpacket_id(g_id_controller);
        dxl_set_txpacket_instruction(INST_READ);
        dxl_set_txpacket_parameter(0, CM730_GYRO_Z_L);
        dxl_set_txpacket_parameter(1, (CM730_ACCEL_Z_H-CM730_GYRO_Z_L) + 1);
        dxl_set_txpacket_length(4);
        dxl_txrx_packet();
        int result = dxl_get_result();

        if (result != COMM_RXSUCCESS)
        {
            delay(5);   // give a delay
            dxl_txrx_packet();  // try again?
            result = dxl_get_result();
            if (result != COMM_RXSUCCESS)
            {
                printf("Abort TXRX Error=%d\n\n", result);
                break;  // abort this loop...
            }
        }
 #define FUDGE 5
        uint8_t fChanged = 0;
        for (int i = 0; i < 6; i++)
        {
            uint16_t w = dxl_makeword(dxl_get_rxpacket_parameter(i*2), (int)dxl_get_rxpacket_parameter(i*2+1));

            if (abs((int)w - (int)awIMUVals[i]) > FUDGE)
            {
                awIMUVals[i] = w;
                fChanged = 1;
            }
        }
        if (fChanged)
        {
            printf("G(%u,%u,%u) A(%u, %u, %u)\n", awIMUVals[GYRO_X],  awIMUVals[GYRO_Y],  awIMUVals[GYRO_Z],
                                                  awIMUVals[ACCEL_X],  awIMUVals[ACCEL_Y],  awIMUVals[ACCEL_Z]);
        }
    }
    delay(250);
}

#if defined(_WIN32) || defined(_WIN64)
// BUGBUG Will move into support files.
//=======================================================================================

unsigned long micros(void)
{
    static int initialized = 0;
    static LARGE_INTEGER  freq, startCount;
    static struct timespec tv_start, tv;
    LARGE_INTEGER  curCount;
    time_t sec_part;
    long nsec_part;

    if (!initialized) {
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&startCount);
        timespec_get(&tv_start, TIME_UTC);
        initialized = 1;
    }

    QueryPerformanceCounter(&curCount);

    curCount.QuadPart -= startCount.QuadPart;
    sec_part = curCount.QuadPart / freq.QuadPart;
    nsec_part = (long)((curCount.QuadPart - (sec_part * freq.QuadPart))
        * 1000000000UL / freq.QuadPart);

    tv.tv_sec = tv_start.tv_sec + sec_part;
    tv.tv_nsec = tv_start.tv_nsec + nsec_part;
    if (tv.tv_nsec >= 1000000000UL) {
        tv.tv_sec += 1;
        tv.tv_nsec -= 1000000000UL;
    }
    return ((unsigned long)tv.tv_sec * 1000000L + (unsigned long)(tv.tv_nsec / 1000L));
}
#endif
//=======================================================================================
