#include "RoboClaw.h"

//
// Constructor
//
RoboClaw::RoboClaw() : WrapperSerial::WrapperSerial()
{
}

//
// Destructor
//
RoboClaw::~RoboClaw()
{
}

// Added by Kurt
#define delay(x)  usleep((x)*1000)

extern unsigned long millis(void);
extern unsigned long micros(void);

int RoboClaw::read(uint32_t ulTimeout) {
  unsigned long ulTimeStart = micros();
  int ich;
  while ((ich = WrapperSerial::read()) == -1) {
    if ((micros()-ulTimeStart) > ulTimeout)
      break;
    usleep(100);	// give time for other things
  }
  return ich;
}



void RoboClaw::write_n(uint8_t cnt, ... )
  {
	uint8_t crc=0;
	
	//send data with crc
	va_list marker;
	va_start( marker, cnt );     /* Initialize variable arguments. */
	for(uint8_t index=0;index<cnt;index++){
		uint8_t data = (uint8_t)(va_arg(marker, int) & 0xff);  //bugbug was uin16_t
		crc+=data;
		write(data);
	}
	va_end( marker );              /* Reset variable arguments.      */
	write(crc&0x7F);
}

void RoboClaw::ForwardM1(uint8_t address, uint8_t speed){
	write_n(3,address,M1FORWARD,speed);
}

void RoboClaw::BackwardM1(uint8_t address, uint8_t speed){
	write_n(3,address,M1BACKWARD,speed);
}

void RoboClaw::SetMinVoltageMainBattery(uint8_t address, uint8_t voltage){
	write_n(3,address,SETMINMB,voltage);
}

void RoboClaw::SetMaxVoltageMainBattery(uint8_t address, uint8_t voltage){
	write_n(3,address,SETMAXMB,voltage);
}

void RoboClaw::ForwardM2(uint8_t address, uint8_t speed){
	write_n(3,address,M2FORWARD,speed);
}

void RoboClaw::BackwardM2(uint8_t address, uint8_t speed){
	write_n(3,address,M2BACKWARD,speed);
}

void RoboClaw::ForwardBackwardM1(uint8_t address, uint8_t speed){
	write_n(3,address,M17BIT,speed);
}

void RoboClaw::ForwardBackwardM2(uint8_t address, uint8_t speed){
	write_n(3,address,M27BIT,speed);
}

void RoboClaw::ForwardMixed(uint8_t address, uint8_t speed){
	write_n(3,address,MIXEDFORWARD,speed);
}

void RoboClaw::BackwardMixed(uint8_t address, uint8_t speed){
	write_n(3,address,MIXEDBACKWARD,speed);
}

void RoboClaw::TurnRightMixed(uint8_t address, uint8_t speed){
	write_n(3,address,MIXEDRIGHT,speed);
}

void RoboClaw::TurnLeftMixed(uint8_t address, uint8_t speed){
	write_n(3,address,MIXEDLEFT,speed);
}

void RoboClaw::ForwardBackwardMixed(uint8_t address, uint8_t speed){
	write_n(3,address,MIXEDFB,speed);
}

void RoboClaw::LeftRightMixed(uint8_t address, uint8_t speed){
	write_n(3,address,MIXEDLR,speed);
}

uint32_t RoboClaw::Read4_1(uint8_t address, uint8_t cmd, uint8_t *status,bool *valid){
	uint8_t crc;
	write(address);
	crc=address;
	write(cmd);
	crc+=cmd;

	uint32_t value;
	uint8_t data = read(10000);
	crc+=data;
	value=(uint32_t)data<<24;

	data = read(10000);
	crc+=data;
	value|=(uint32_t)data<<16;

	data = read(10000);
	crc+=data;
	value|=(uint32_t)data<<8;

	data = read(10000);
	crc+=data;
	value|=(uint32_t)data;
	
	data = read(10000);
	crc+=data;
	if(status)
		*status = data;
		
	data = read(10000);
	if(valid)
		*valid = ((crc&0x7F)==data);
		
	return value;
}

uint32_t RoboClaw::ReadEncM1(uint8_t address, uint8_t *status,bool *valid){
	return Read4_1(address,GETM1ENC,status,valid);
}

uint32_t RoboClaw::ReadEncM2(uint8_t address, uint8_t *status,bool *valid){
	return Read4_1(address,GETM2ENC,status,valid);
}

uint32_t RoboClaw::ReadSpeedM1(uint8_t address, uint8_t *status,bool *valid){
	return Read4_1(address,GETM1SPEED,status,valid);
}

uint32_t RoboClaw::ReadSpeedM2(uint8_t address, uint8_t *status,bool *valid){
	return Read4_1(address,GETM2SPEED,status,valid);
}

void RoboClaw::ResetEncoders(uint8_t address){
	write_n(2,address,RESETENC);
}

bool RoboClaw::ReadVersion(uint8_t address,char *version){
	uint8_t crc;
	write(address);
	crc=address;
	write(GETVERSION);
	crc+=GETVERSION;
	
	for(uint8_t i=0;i<32;i++){
		version[i]=read(10000);
		crc+=version[i];
		if(version[i]==0){
			if((crc&0x7F)==read(10000))
				return true;
			else
				return false;
		}
	}
	return false;
}

uint16_t RoboClaw::Read2(uint8_t address,uint8_t cmd,bool *valid){
	uint8_t crc;
	write(address);
	crc=address;
	write(cmd);
	crc+=cmd;
	
	uint16_t value;	
	uint8_t data = read(10000);
	crc+=data;
	value=(uint16_t)data<<8;

	data = read(10000);
	crc+=data;
	value|=(uint16_t)data;
	
	data = read(10000);
	if(valid)
		*valid = ((crc&0x7F)==data);
		
	return value;
}


uint16_t RoboClaw::ReadMainBatteryVoltage(uint8_t address,bool *valid){
	return Read2(address,GETMBATT,valid);
}

uint16_t RoboClaw::ReadLogicBattVoltage(uint8_t address,bool *valid){
	return Read2(address,GETLBATT,valid);
}

void RoboClaw::SetMinVoltageLogicBattery(uint8_t address, uint8_t voltage){
	write_n(3,address,SETMINLB,voltage);
}

void RoboClaw::SetMaxVoltageLogicBattery(uint8_t address, uint8_t voltage){
	write_n(3,address,SETMAXLB,voltage);
}

#define SetDWORDval(arg) (uint8_t)(arg>>24),(uint8_t)(arg>>16),(uint8_t)(arg>>8),(uint8_t)arg
#define SetWORDval(arg) (uint8_t)(arg>>8),(uint8_t)arg

void RoboClaw::SetM1Constants(uint8_t address, uint32_t kd, uint32_t kp, uint32_t ki, uint32_t qpps){
	write_n(18,address,SETM1PID,SetDWORDval(kd),SetDWORDval(kp),SetDWORDval(ki),SetDWORDval(qpps));
}

void RoboClaw::SetM2Constants(uint8_t address, uint32_t kd, uint32_t kp, uint32_t ki, uint32_t qpps){
	write_n(18,address,SETM2PID,SetDWORDval(kd),SetDWORDval(kp),SetDWORDval(ki),SetDWORDval(qpps));
}

uint32_t RoboClaw::ReadISpeedM1(uint8_t address,uint8_t *status,bool *valid){
	return Read4_1(address,GETM1ISPEED,status,valid);
}

uint32_t RoboClaw::ReadISpeedM2(uint8_t address,uint8_t *status,bool *valid){
	return Read4_1(address,GETM2ISPEED,status,valid);
}

void RoboClaw::DutyM1(uint8_t address, uint16_t duty){
	write_n(4,address,M1DUTY,SetWORDval(duty));
}

void RoboClaw::DutyM2(uint8_t address, uint16_t duty){
	write_n(4,address,M2DUTY,SetWORDval(duty));
}

void RoboClaw::DutyM1M2(uint8_t address, uint16_t duty1, uint16_t duty2){
	write_n(6,address,MIXEDDUTY,SetWORDval(duty1),SetWORDval(duty2));
}

void RoboClaw::SpeedM1(uint8_t address, uint32_t speed){
	write_n(6,address,M1SPEED,SetDWORDval(speed));
}

void RoboClaw::SpeedM2(uint8_t address, uint32_t speed){
	write_n(6,address,M2SPEED,SetDWORDval(speed));
}

void RoboClaw::SpeedM1M2(uint8_t address, uint32_t speed1, uint32_t speed2){
	write_n(10,address,M1SPEED,SetDWORDval(speed1),SetDWORDval(speed2));
}

void RoboClaw::SpeedAccelM1(uint8_t address, uint32_t accel, uint32_t speed){
	write_n(10,address,M1SPEEDACCEL,SetDWORDval(accel),SetDWORDval(speed));
}

void RoboClaw::SpeedAccelM2(uint8_t address, uint32_t accel, uint32_t speed){
	write_n(10,address,M2SPEEDACCEL,SetDWORDval(accel),SetDWORDval(speed));
}
void RoboClaw::SpeedAccelM1M2(uint8_t address, uint32_t accel, uint32_t speed1, uint32_t speed2){
	write_n(10,address,MIXEDSPEEDACCEL,SetDWORDval(accel),SetDWORDval(speed1),SetDWORDval(speed2));
}

void RoboClaw::SpeedDistanceM1(uint8_t address, uint32_t speed, uint32_t distance, uint8_t flag){
	write_n(11,address,M1SPEEDDIST,SetDWORDval(speed),SetDWORDval(distance),flag);
}

void RoboClaw::SpeedDistanceM2(uint8_t address, uint32_t speed, uint32_t distance, uint8_t flag){
	write_n(11,address,M2SPEEDDIST,SetDWORDval(speed),SetDWORDval(distance),flag);
}

void RoboClaw::SpeedDistanceM1M2(uint8_t address, uint32_t speed1, uint32_t distance1, uint32_t speed2, uint32_t distance2, uint8_t flag){
	write_n(19,address,M1SPEEDDIST,SetDWORDval(speed2),SetDWORDval(distance1),SetDWORDval(speed2),SetDWORDval(distance2),flag);
}

void RoboClaw::SpeedAccelDistanceM1(uint8_t address, uint32_t accel, uint32_t speed, uint32_t distance, uint8_t flag){
	write_n(15,address,M1SPEEDACCELDIST,SetDWORDval(accel),SetDWORDval(speed),SetDWORDval(distance),flag);
}

void RoboClaw::SpeedAccelDistanceM2(uint8_t address, uint32_t accel, uint32_t speed, uint32_t distance, uint8_t flag){
	write_n(15,address,M2SPEEDACCELDIST,SetDWORDval(accel),SetDWORDval(speed),SetDWORDval(distance),flag);
}

void RoboClaw::SpeedAccelDistanceM1M2(uint8_t address, uint32_t accel, uint32_t speed1, uint32_t distance1, uint32_t speed2, uint32_t distance2, uint8_t flag){
	write_n(23,address,M1SPEEDACCELDIST,SetDWORDval(accel),SetDWORDval(speed1),SetDWORDval(distance1),SetDWORDval(speed2),SetDWORDval(distance2),flag);
}

bool RoboClaw::ReadBuffers(uint8_t address, uint8_t &depth1, uint8_t &depth2){
	bool valid;
	uint16_t value = Read2(address,GETBUFFERS,&valid);
	if(valid){
		depth1 = value>>8;
		depth2 = value;
	}
	return valid;
}

bool RoboClaw::ReadCurrents(uint8_t address, uint8_t &current1, uint8_t &current2){
	bool valid;
	uint16_t value = Read2(address,GETCURRENTS,&valid);
	if(valid){
		current1 = value>>8;
		current2 = value;
	}
	return valid;
}

void RoboClaw::SpeedAccelM1M2_2(uint8_t address, uint32_t accel1, uint32_t speed1, uint32_t accel2, uint32_t speed2){
	write_n(18,address,MIXEDSPEED2ACCEL,SetDWORDval(accel1),SetDWORDval(speed1),SetDWORDval(accel2),SetDWORDval(speed2));
}

void RoboClaw::SpeedAccelDistanceM1M2_2(uint8_t address, uint32_t accel1, uint32_t speed1, uint32_t distance1, uint32_t accel2, uint32_t speed2, uint32_t distance2, uint8_t flag){
	write_n(27,address,MIXEDSPEED2ACCELDIST,SetDWORDval(accel1),SetDWORDval(speed1),SetDWORDval(distance1),SetDWORDval(accel2),SetDWORDval(speed2),SetDWORDval(distance2),flag);
}

void RoboClaw::DutyAccelM1(uint8_t address, uint16_t duty, uint16_t accel){
	write_n(6,address,M1DUTY,SetWORDval(duty),SetWORDval(accel));
}

void RoboClaw::DutyAccelM2(uint8_t address, uint16_t duty, uint16_t accel){
	write_n(6,address,M2DUTY,SetWORDval(duty),SetWORDval(accel));
}

void RoboClaw::DutyAccelM1M2(uint8_t address, uint16_t duty1, uint16_t accel1, uint16_t duty2, uint16_t accel2){
	write_n(10,address,MIXEDDUTY,SetWORDval(duty1),SetWORDval(accel1),SetWORDval(duty2),SetWORDval(accel2));
}

uint8_t RoboClaw::ReadError(uint8_t address,bool *valid){
	uint8_t crc;
	write(address);
	crc=address;
	write(GETERROR);
	crc+=GETERROR;
	
	uint8_t value = read(10000);
	crc+=value;

	if(valid)
		*valid = ((crc&0x7F)==read(10000));
	else
		read(10000);
		
	return value;
}

void RoboClaw::WriteNVM(uint8_t address){
	write_n(2,address,WRITENVM);
}
