#ifndef RoboClaw_h
#define RoboClaw_h

#include <stdarg.h>

#include "WrapperSerial.h"

/******************************************************************************
* Definitions
******************************************************************************/

#define _RC_VERSION 10 // software version of this library
#ifndef GCC_VERSION
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

class RoboClaw : public WrapperSerial
{
	enum {M1FORWARD = 0,
			M1BACKWARD = 1,
			SETMINMB = 2,
			SETMAXMB = 3,
			M2FORWARD = 4,
			M2BACKWARD = 5,
			M17BIT = 6,
			M27BIT = 7,
			MIXEDFORWARD = 8,
			MIXEDBACKWARD = 9,
			MIXEDRIGHT = 10,
			MIXEDLEFT = 11,
			MIXEDFB = 12,
			MIXEDLR = 13,
			GETM1ENC = 16,
			GETM2ENC = 17,
			GETM1SPEED = 18,
			GETM2SPEED = 19,
			RESETENC = 20,
			GETVERSION = 21,
			GETMBATT = 24,
			GETLBATT = 25,
			SETMINLB = 26,
			SETMAXLB = 27,
			SETM1PID = 28,
			SETM2PID = 29,
			GETM1ISPEED = 30,
			GETM2ISPEED = 31,
			M1DUTY = 32,
			M2DUTY = 33,
			MIXEDDUTY = 34,
			M1SPEED = 35,
			M2SPEED = 36,
			MIXEDSPEED = 37,
			M1SPEEDACCEL = 38,
			M2SPEEDACCEL = 39,
			MIXEDSPEEDACCEL = 40,
			M1SPEEDDIST = 41,
			M2SPEEDDIST = 42,
			MIXEDSPEEDDIST = 43,
			M1SPEEDACCELDIST = 44,
			M2SPEEDACCELDIST = 45,
			MIXEDSPEEDACCELDIST = 46,
			GETBUFFERS = 47,
			GETCURRENTS = 49,
			MIXEDSPEED2ACCEL = 50,
			MIXEDSPEED2ACCELDIST = 51,
			M1DUTYACCEL = 52,
			M2DUTYACCEL = 53,
			MIXEDDUTYACCEL = 54,
			GETERROR = 90,
			WRITENVM = 94};
public:
	// public methods
	RoboClaw();
	~RoboClaw();

	void ForwardM1(uint8_t address, uint8_t speed);
	void BackwardM1(uint8_t address, uint8_t speed);
	void SetMinVoltageMainBattery(uint8_t address, uint8_t voltage);
	void SetMaxVoltageMainBattery(uint8_t address, uint8_t voltage);
	void ForwardM2(uint8_t address, uint8_t speed);
	void BackwardM2(uint8_t address, uint8_t speed);
	void ForwardBackwardM1(uint8_t address, uint8_t speed);
	void ForwardBackwardM2(uint8_t address, uint8_t speed);
	void ForwardMixed(uint8_t address, uint8_t speed);
	void BackwardMixed(uint8_t address, uint8_t speed);
	void TurnRightMixed(uint8_t address, uint8_t speed);
	void TurnLeftMixed(uint8_t address, uint8_t speed);
	void ForwardBackwardMixed(uint8_t address, uint8_t speed);
	void LeftRightMixed(uint8_t address, uint8_t speed);
	uint32_t ReadEncM1(uint8_t address, uint8_t *status=NULL,bool *valid=NULL);
	uint32_t ReadEncM2(uint8_t address, uint8_t *status=NULL,bool *valid=NULL);
	uint32_t ReadSpeedM1(uint8_t address, uint8_t *status=NULL,bool *valid=NULL);
	uint32_t ReadSpeedM2(uint8_t address, uint8_t *status=NULL,bool *valid=NULL);
	void ResetEncoders(uint8_t address);
	bool ReadVersion(uint8_t address,char *version);
	uint16_t ReadMainBatteryVoltage(uint8_t address,bool *valid=NULL);
	uint16_t ReadLogicBattVoltage(uint8_t address,bool *valid=NULL);
	void SetMinVoltageLogicBattery(uint8_t address, uint8_t voltage);
	void SetMaxVoltageLogicBattery(uint8_t address, uint8_t voltage);
	void SetM1Constants(uint8_t address, uint32_t Kd, uint32_t Kp, uint32_t Ki, uint32_t qpps);
	void SetM2Constants(uint8_t address, uint32_t Kd, uint32_t Kp, uint32_t Ki, uint32_t qpps);
	uint32_t ReadISpeedM1(uint8_t address,uint8_t *status=NULL,bool *valid=NULL);
	uint32_t ReadISpeedM2(uint8_t address,uint8_t *status=NULL,bool *valid=NULL);
	void DutyM1(uint8_t address, uint16_t duty);
	void DutyM2(uint8_t address, uint16_t duty);
	void DutyM1M2(uint8_t address, uint16_t duty1, uint16_t duty2);
	void SpeedM1(uint8_t address, uint32_t speed);
	void SpeedM2(uint8_t address, uint32_t speed);
	void SpeedM1M2(uint8_t address, uint32_t speed1, uint32_t speed2);
	void SpeedAccelM1(uint8_t address, uint32_t accel, uint32_t speed);
	void SpeedAccelM2(uint8_t address, uint32_t accel, uint32_t speed);
	void SpeedAccelM1M2(uint8_t address, uint32_t accel, uint32_t speed1, uint32_t speed2);
	void SpeedDistanceM1(uint8_t address, uint32_t speed, uint32_t distance, uint8_t flag=0);
	void SpeedDistanceM2(uint8_t address, uint32_t speed, uint32_t distance, uint8_t flag=0);
	void SpeedDistanceM1M2(uint8_t address, uint32_t speed1, uint32_t distance1, uint32_t speed2, uint32_t distance2, uint8_t flag=0);
	void SpeedAccelDistanceM1(uint8_t address, uint32_t accel, uint32_t speed, uint32_t distance, uint8_t flag=0);
	void SpeedAccelDistanceM2(uint8_t address, uint32_t accel, uint32_t speed, uint32_t distance, uint8_t flag=0);
	void SpeedAccelDistanceM1M2(uint8_t address, uint32_t accel, uint32_t speed1, uint32_t distance1, uint32_t speed2, uint32_t distance2, uint8_t flag=0);
	bool ReadBuffers(uint8_t address, uint8_t &depth1, uint8_t &depth2);
	bool ReadCurrents(uint8_t address, uint8_t &current1, uint8_t &current2);
	void SpeedAccelM1M2_2(uint8_t address, uint32_t accel1, uint32_t speed1, uint32_t accel2, uint32_t speed2);
	void SpeedAccelDistanceM1M2_2(uint8_t address, uint32_t accel1, uint32_t speed1, uint32_t distance1, uint32_t accel2, uint32_t speed2, uint32_t distance2, uint8_t flag=0);
	void DutyAccelM1(uint8_t address, uint16_t duty, uint16_t accel);
	void DutyAccelM2(uint8_t address, uint16_t duty, uint16_t accel);
	void DutyAccelM1M2(uint8_t address, uint16_t duty1, uint16_t accel1, uint16_t duty2, uint16_t accel2);
	uint8_t ReadError(uint8_t address,bool *valid=NULL);
	void WriteNVM(uint8_t address);
	
private:
	void write_n(uint8_t byte, ...);
	int	 read(uint32_t ulTimeout);
	uint32_t Read4_1(uint8_t address, uint8_t cmd, uint8_t *status,bool *valid);
	uint16_t Read2(uint8_t address,uint8_t cmd,bool *valid);
	
};

#endif