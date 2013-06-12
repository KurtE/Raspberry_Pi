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


// Packet Serial Mode
#define PACKETS_ADDRESS 128    //  Address of the device

#define BBB_SERVO_SUPPORT

#define OPT_PCMSOUND

#define DEADBAND    4         // How much dead band on stick values


#define OUR_SERVOS_MIN 563
#define OUR_SERVOS_MAX 2437


#define VOLTAGE_MIN1	100	// 10 volts is the min that we will allow...


#define PAN_MIN      750    // Min Pan 
#define PAN_MAX      2250   // Max Pan
#define TILT_MIN     750    // Max tilt value
#define TILT_MAX     2250   // Min ... 
#define ROTATE_MIN   750
#define ROTATE_MAX   2250

#ifdef BBB_SERVO_SUPPORT
#define NUM_PAN_TILT_SERVOS 3
#define MNUMSERVOS    (NUM_PAN_TILT_SERVOS)
enum {PANS_I=0, TILTS_I};
static char *g_apszServos[] = {"Pan", "Tilt", "Rotate"};
#endif

