//--------------------------------------------------------------------------
// Kurts Rover configuration file.
// This configuration file is used to hide a bunch of the messy stuff where
// we have a few different configurations defined and several tables depend
// on which ones we have defined.  This is especially true about what Servos
// are defined, which we can use PWM type signals to control the motors or we
// can use serial packet mode.  Likewise we can define to have Pan and Tilt servos
// or not...
//--------------------------------------------------------------------------

// Define which way we will control motors, only uncomment one of these.
#define PACKET_MODE
//#define PWM_MODE



// Packet Serial Mode
#define PACKETS_ADDRESS 128    //  Address of the device

//#define PAN_SERVO                2      // Pan and tilt... (Optional) Undefine if not needed
//#define TILT_SERVO               3      //

#define OPT_PCMSOUND

#define SOUND_PIN                5        // Botboarduino JR pin number
#define PS2_DAT                  6        // PS2 Data line
#define PS2_CMD                  7        // PS2 Command line
#define PS2_SELECT               8        // PS2 Select line
#define PS2_CLOCK                9        // PS2 Clock Line

#define MOTOR_CONTROLLER_PIN_1  10        // output pin serial mode - Also supports PWM... 
#define MOTOR_CONTROLLER_PIN_2  11        // input pin Serial mode ..

#define DEADBAND    4         // How much dead band on stick values


#define OUR_SERVOS_MIN 563
#define OUR_SERVOS_MAX 2437


#define VOLTAGE_MIN1	100	// 10 volts is the min that we will allow...


#define PAN_MIN       10     // Min Pan 
#define PAN_MAX       170    // Max Pan
#define TILT_MIN     10      // Max tilt value
#define TILT_MAX     170     // Min ... 

#ifdef PWM_MODE
#define NUM_MOTOR_SERVOS 2
#endif

#ifdef PAN_SERVO
#define NUM_PAN_TILT_SERVOS 2
#endif
#define MNUMSERVOS    (NUM_MOTOR_SERVOS+NUM_PAN_TILT_SERVOS)

#if MNUMSERVOS
#if NUM_MOTOR_SERVOS
#if NUM_PAN_TILT_SERVOS
static const uint8_t g_abPinTable[] PROGMEM = {MOTOR_CONTROLLER_PIN_1, MOTOR_CONTROLLER_PIN_2, PAN_SERVO, TILT_SERVO};
enum {MOTOR1_I=0, MOTOR2_I, PANS_I, TILTS_I};

// Note: maybe move to program mem
static char *g_apszServos[] = {"Motor1","Motor2","Pan", "Tilt"};

#else
static const byte g_abPinTable[] PROGMEM = {MOTOR_CONTROLLER_PIN_1, MOTOR_CONTROLLER_PIN_2};
enum {MOTOR1_I=0, MOTOR2_I};
static char *g_apszServos[] = {"Motor1","Motor2"};
#endif

#else
static const byte g_abPinTable[] PROGMEM = {PAN_SERVO, TILT_SERVO};
enum {PANS_I=0, TILTS_I};
static char *g_apszServos[] = {"Pan", "Tilt"};
#endif

#endif

