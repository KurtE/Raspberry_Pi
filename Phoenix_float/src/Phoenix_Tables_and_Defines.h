//=============================================================================
//Project Phoenix - This file is an extract of lots of the data and tables
//  from the main Phoenix_Code file.  It was move here as to make the other
//  file a little easier to read.
//
//=============================================================================
//Build tables for Leg configuration like I/O and MIN/ Max values to easy access values using a FOR loop
//Constants are still defined as single values in the cfg file to make it easy to read/configure

// BUGBUG: Need a cleaner way to define...
// Lets allow for which legs servos to be inverted to be defined by the robot
// This is used by the Lynxmotion Symetrical Quad.
#ifndef cRRCoxaInv
#define cRRCoxaInv 1 
#endif
#ifndef cRMCoxaInv 
#define cRMCoxaInv 1 
#endif
#ifndef cRFCoxaInv 
#define cRFCoxaInv 1 
#endif

#ifndef cLRCoxaInv 
#define cLRCoxaInv 0 
#endif
#ifndef cLMCoxaInv 
#define cLMCoxaInv 0 
#endif
#ifndef cLFCoxaInv 
#define cLFCoxaInv 0 
#endif

#ifndef cRRFemurInv 
#define cRRFemurInv 1 
#endif
#ifndef cRMFemurInv 
#define cRMFemurInv 1 
#endif
#ifndef cRFFemurInv 
#define cRFFemurInv 1 
#endif

#ifndef cLRFemurInv 
#define cLRFemurInv 0 
#endif
#ifndef cLMFemurInv 
#define cLMFemurInv 0 
#endif
#ifndef cLFFemurInv 
#define cLFFemurInv 0 
#endif

#ifndef cRRTibiaInv 
#define cRRTibiaInv 1 
#endif
#ifndef cRMTibiaInv 
#define cRMTibiaInv 1 
#endif
#ifndef cRFTibiaInv 
#define cRFTibiaInv 1 
#endif

#ifndef cLRTibiaInv 
#define cLRTibiaInv 0 
#endif
#ifndef cLMTibiaInv 
#define cLMTibiaInv 0 
#endif
#ifndef cLFTibiaInv 
#define cLFTibiaInv 0 
#endif

#ifndef cRRTarsInv
#define cRRTarsInv 1 
#endif
#ifndef cRMTarsInv 
#define cRMTarsInv 1 
#endif
#ifndef cRFTarsInv 
#define cRFTarsInv 1 
#endif

#ifndef cLRTarsInv 
#define cLRTarsInv 0 
#endif
#ifndef cLMTarsInv 
#define cLMTarsInv 0 
#endif
#ifndef cLFTarsInv 
#define cLFTarsInv 0 
#endif

// Also define default BalanceDelay
#ifndef BALANCE_DELAY
#define BALANCE_DELAY 100
#endif


#ifndef QUADMODE
// Standard Hexapod...
// Servo Horn offsets
#ifdef cRRFemurHornOffset1                        // per leg configuration
static const short cFemurHornOffset1[] = {
  cRRFemurHornOffset1, cRMFemurHornOffset1, cRFFemurHornOffset1, cLRFemurHornOffset1, cLMFemurHornOffset1, cLFFemurHornOffset1};
#define CFEMURHORNOFFSET1(LEGI) ((short)pgm_read_word(&cFemurHornOffset1[LEGI]))
#else                                             // Fixed per leg, if not defined 0
#ifndef cFemurHornOffset1
#define cFemurHornOffset1  0
#endif
#define CFEMURHORNOFFSET1(LEGI)  (cFemurHornOffset1)
#endif

#ifdef cRRTibiaHornOffset1   // per leg configuration
static const short cTibiaHornOffset1[] = {
  cRRTibiaHornOffset1, cRMTibiaHornOffset1, cRFTibiaHornOffset1, cLRTibiaHornOffset1, cLMTibiaHornOffset1, cLFTibiaHornOffset1};
#define CTIBIAHORNOFFSET1(LEGI) ((short)pgm_read_word(&cTibiaHornOffset1[LEGI]))
#else   // Fixed per leg, if not defined 0
#ifndef cTibiaHornOffset1
#define cTibiaHornOffset1  0
#endif
#define CTIBIAHORNOFFSET1(LEGI)  (cTibiaHornOffset1)
#endif

#ifdef c4DOF
#ifdef cRRTarsHornOffset1                         // per leg configuration
static const short cTarsHornOffset1[] = {
  cRRTarsHornOffset1,  cRMTarsHornOffset1,  cRFTarsHornOffset1,  cLRTarsHornOffset1,  cLMTarsHornOffset1,  cLFTarsHornOffset1};
#define CTARSHORNOFFSET1(LEGI) ((short)pgm_read_word(&cTarsHornOffset1[LEGI]))
#else                                             // Fixed per leg, if not defined 0
#ifndef cTarsHornOffset1
#define cTarsHornOffset1  0
#endif
#define CTARSHORNOFFSET1(LEGI)  cTarsHornOffset1
#endif
#endif

//Min / Max values
#ifndef SERVOS_DO_MINMAX
const short cCoxaMin[] = {
  cRRCoxaMin,  cRMCoxaMin,  cRFCoxaMin,  cLRCoxaMin,  cLMCoxaMin,  cLFCoxaMin};
const short cCoxaMax[] = {
  cRRCoxaMax,  cRMCoxaMax,  cRFCoxaMax,  cLRCoxaMax,  cLMCoxaMax,  cLFCoxaMax};
const short cFemurMin[] ={
  cRRFemurMin, cRMFemurMin, cRFFemurMin, cLRFemurMin, cLMFemurMin, cLFFemurMin};
const short cFemurMax[] ={
  cRRFemurMax, cRMFemurMax, cRFFemurMax, cLRFemurMax, cLMFemurMax, cLFFemurMax};
const short cTibiaMin[] ={
  cRRTibiaMin, cRMTibiaMin, cRFTibiaMin, cLRTibiaMin, cLMTibiaMin, cLFTibiaMin};
const short cTibiaMax[] = {
  cRRTibiaMax, cRMTibiaMax, cRFTibiaMax, cLRTibiaMax, cLMTibiaMax, cLFTibiaMax};

#ifdef c4DOF
const short cTarsMin[] = {
  cRRTarsMin, cRMTarsMin, cRFTarsMin, cLRTarsMin, cLMTarsMin, cLFTarsMin};
const short cTarsMax[] = {
  cRRTarsMax, cRMTarsMax, cRFTarsMax, cLRTarsMax, cLMTarsMax, cLFTarsMax};
#endif
#endif

// Servo inverse direction
const bool cCoxaInv[] = {cRRCoxaInv, cRMCoxaInv, cRFCoxaInv, cLRCoxaInv, cLMCoxaInv, cLFCoxaInv};
bool cFemurInv[] = {cRRFemurInv, cRMFemurInv, cRFFemurInv, cLRFemurInv, cLMFemurInv, cLFFemurInv};
const bool cTibiaInv[] = {cRRTibiaInv, cRMTibiaInv, cRFTibiaInv, cLRTibiaInv, cLMTibiaInv, cLFTibiaInv};

#ifdef c4DOF
const bool cTarsInv[] = {cRRTarsInv, cRMTarsInv, cRFTarsInv, cLRTarsInv, cLMTarsInv, cLFTarsInv};
#endif

//Leg Lengths
const byte cCoxaLength[] = {
  cRRCoxaLength,  cRMCoxaLength,  cRFCoxaLength,  cLRCoxaLength,  cLMCoxaLength,  cLFCoxaLength};
const byte cFemurLength[] = {
  cRRFemurLength, cRMFemurLength, cRFFemurLength, cLRFemurLength, cLMFemurLength, cLFFemurLength};
const byte cTibiaLength[] = {
  cRRTibiaLength, cRMTibiaLength, cRFTibiaLength, cLRTibiaLength, cLMTibiaLength, cLFTibiaLength};
#ifdef c4DOF
const byte cTarsLength[] = {
  cRRTarsLength, cRMTarsLength, cRFTarsLength, cLRTarsLength, cLMTarsLength, cLFTarsLength};
#endif

//Body Offsets [distance between the center of the body and the center of the coxa]
const short cOffsetX[] = {
  cRROffsetX, cRMOffsetX, cRFOffsetX, cLROffsetX, cLMOffsetX, cLFOffsetX};
const short cOffsetZ[] = {
  cRROffsetZ, cRMOffsetZ, cRFOffsetZ, cLROffsetZ, cLMOffsetZ, cLFOffsetZ};

//Default leg angle
const short cCoxaAngle[] = {
  cRRCoxaAngle, cRMCoxaAngle, cRFCoxaAngle, cLRCoxaAngle, cLMCoxaAngle, cLFCoxaAngle};

#ifdef cRRInitCoxaAngle    // We can set different angles for the legs than just where they servo horns are set...
const short cCoxaInitAngle[] = {
  cRRInitCoxaAngle, cRMInitCoxaAngle, cRFInitCoxaAngle, cLRInitCoxaAngle, cLMInitCoxaAngle, cLFInitCoxaAngle};
#endif

//Start positions for the leg
const short cInitPosX[] = {
  cRRInitPosX, cRMInitPosX, cRFInitPosX, cLRInitPosX, cLMInitPosX, cLFInitPosX};
const short cInitPosY[] = {
  cRRInitPosY, cRMInitPosY, cRFInitPosY, cLRInitPosY, cLMInitPosY, cLFInitPosY};
const short cInitPosZ[] = {
  cRRInitPosZ, cRMInitPosZ, cRFInitPosZ, cLRInitPosZ, cLMInitPosZ, cLFInitPosZ};

//=============================================================================
#else
// Quads...
// Servo Horn offsets
#ifdef cRRFemurHornOffset1                        // per leg configuration
static const short cFemurHornOffset1[] = {
  cRRFemurHornOffset1, cRFFemurHornOffset1, cLRFemurHornOffset1, cLFFemurHornOffset1};
#define CFEMURHORNOFFSET1(LEGI) ((short)pgm_read_word(&cFemurHornOffset1[LEGI]))
#else                                             // Fixed per leg, if not defined 0
#ifndef cFemurHornOffset1
#define cFemurHornOffset1  0
#endif
#define CFEMURHORNOFFSET1(LEGI)  (cFemurHornOffset1)
#endif

#ifdef cRRTibiaHornOffset1   // per leg configuration
static const short cTibiaHornOffset1[] = {
  cRRTibiaHornOffset1, cRFTibiaHornOffset1, cLRTibiaHornOffset1, cLFTibiaHornOffset1};
#define CTIBIAHORNOFFSET1(LEGI) ((short)pgm_read_word(&cTibiaHornOffset1[LEGI]))
#else   // Fixed per leg, if not defined 0
#ifndef cTibiaHornOffset1
#define cTibiaHornOffset1  0
#endif
#define CTIBIAHORNOFFSET1(LEGI)  (cTibiaHornOffset1)
#endif



#ifdef c4DOF
#ifdef cRRTarsHornOffset1                         // per leg configuration
static const short cTarsHornOffset1[] = {
  cRRTarsHornOffset1, cRFTarsHornOffset1,  cLRTarsHornOffset1, cLFTarsHornOffset1};
#define CTARSHORNOFFSET1(LEGI) ((short)pgm_read_word(&cTarsHornOffset1[LEGI]))
#else                                             // Fixed per leg, if not defined 0
#ifndef cTarsHornOffset1
#define cTarsHornOffset1  0
#endif
#define CTARSHORNOFFSET1(LEGI)  cTarsHornOffset1
#endif
#endif

//Min / Max values
#ifndef SERVOS_DO_MINMAX
const short cCoxaMin[] = {
  cRRCoxaMin,  cRFCoxaMin,  cLRCoxaMin,  cLFCoxaMin};
const short cCoxaMax[] = {
  cRRCoxaMax,  cRFCoxaMax,  cLRCoxaMax,  cLFCoxaMax};
const short cFemurMin[] ={
  cRRFemurMin, cRFFemurMin, cLRFemurMin, cLFFemurMin};
const short cFemurMax[] ={
  cRRFemurMax, cRFFemurMax, cLRFemurMax, cLFFemurMax};
const short cTibiaMin[] ={
  cRRTibiaMin, cRFTibiaMin, cLRTibiaMin, cLFTibiaMin};
const short cTibiaMax[] = {
  cRRTibiaMax, cRFTibiaMax, cLRTibiaMax, cLFTibiaMax};

#ifdef c4DOF
const short cTarsMin[] = {
  cRRTarsMin, cRFTarsMin, cLRTarsMin, cLFTarsMin};
const short cTarsMax[] = {
  cRRTarsMax, cRFTarsMax, cLRTarsMax, cLFTarsMax};
#endif
#endif

// Servo inverse direction
const bool cCoxaInv[] = {cRRCoxaInv, cRFCoxaInv, cLRCoxaInv, cLFCoxaInv};
bool cFemurInv[] = {cRRFemurInv, cRFFemurInv, cLRFemurInv, cLFFemurInv};
const bool cTibiaInv[] = {cRRTibiaInv, cRFTibiaInv, cLRTibiaInv, cLFTibiaInv};

#ifdef c4DOF
const bool cTarsInv[] = {
    cRRTarsInv, cRFTarsInv, cLRTarsInv, cLFTarsInv};
#endif

//Leg Lengths
const byte cCoxaLength[] = {
  cRRCoxaLength,  cRFCoxaLength,  cLRCoxaLength,  cLFCoxaLength};
const byte cFemurLength[] = {
  cRRFemurLength, cRFFemurLength, cLRFemurLength, cLFFemurLength};
const byte cTibiaLength[] = {
  cRRTibiaLength, cRFTibiaLength, cLRTibiaLength, cLFTibiaLength};
#ifdef c4DOF
const byte cTarsLength[] = {
  cRRTarsLength, cRFTarsLength, cLRTarsLength, cLFTarsLength};
#endif

//Body Offsets [distance between the center of the body and the center of the coxa]
const short cOffsetX[] = {
  cRROffsetX, cRFOffsetX, cLROffsetX, cLFOffsetX};
const short cOffsetZ[] = {
  cRROffsetZ, cRFOffsetZ, cLROffsetZ, cLFOffsetZ};

//Default leg angle
const short cCoxaAngle[] = {
  cRRCoxaAngle, cRFCoxaAngle, cLRCoxaAngle, cLFCoxaAngle};

#ifdef cRRInitCoxaAngle    // We can set different angles for the legs than just where they servo horns are set...
const short cCoxaInitAngle[] = {
  cRRInitCoxaAngle, cRFInitCoxaAngle, cLRInitCoxaAngle, cLFInitCoxaAngle};
#endif


//Start positions for the leg
const short cInitPosX[] = {
  cRRInitPosX, cRFInitPosX, cLRInitPosX, cLFInitPosX};
const short cInitPosY[] = {
  cRRInitPosY, cRFInitPosY, cLRInitPosY, cLFInitPosY};
const short cInitPosZ[] = {
  cRRInitPosZ, cRFInitPosZ, cLRInitPosZ, cLFInitPosZ};

#endif
