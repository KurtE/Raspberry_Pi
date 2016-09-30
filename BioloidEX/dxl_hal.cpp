/**
Modification of the HAL of the Dynamixel SDK to be used with USB2AX.

Nicolas Saugnier
*/

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <linux/serial.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <errno.h>
#include "dxl_hal.h"

//#define DEBUG_WIRINGPI
#ifdef DEBUG_WIRINGPI
#include <wiringPi.h>
#define WPD_WRITE_PIN       0
#define WPD_READ_PIN        1
#define WPD_READ_DATA_PIN   2
#define WPD_FLUSH_PIN       3
#define WPD_CLEAR_PIN       4
unsigned char wpd_readDataToggle = 0;
#endif


int	gSocket_fd	= -1;
long	glStartTime	= 0;
float	gfRcvWaitTime	= 0.0f;
float	gfByteTransTime	= 0.0f;
int 	g_use_tcdrain = 0;	// assume we don't need tcdrain. 

int dxl_hal_open(int deviceIndex, float baudrate)
{
	struct termios newtio;
	struct serial_struct serinfo;
	char dev_name[20] = "/dev/ttyDXL";
	uint32_t int_baud = baudrate;

#ifdef DEBUG_WIRINGPI
	wiringPiSetup () ;
	pinMode (WPD_WRITE_PIN, OUTPUT) ;
	pinMode (WPD_READ_PIN, OUTPUT) ;
	pinMode (WPD_READ_DATA_PIN, OUTPUT) ;
	pinMode (WPD_FLUSH_PIN, OUTPUT) ;
	pinMode (WPD_CLEAR_PIN, OUTPUT) ;

#endif
	// Build in support to explit device - /dev/ttyDXL
	dxl_hal_close();    // Make sure any previous handle is closed
	
	if((gSocket_fd = open(dev_name, O_RDWR|O_NOCTTY|O_NONBLOCK)) < 0) {
		// We did not find our explicit one, lets try the standard default USB2AX file name
		sprintf(dev_name, "/dev/ttyACM%d", deviceIndex); // USB2AX is ttyACM
		
		if((gSocket_fd = open(dev_name, O_RDWR|O_NOCTTY|O_NONBLOCK)) < 0) {
			fprintf(stderr, "device open error: %s\n", dev_name);
			
			// Lets also try ttyUSBx to see if we have different board...
			sprintf(dev_name, "/dev/ttyUSB%d", deviceIndex); // USB2AX is ttyACM
			if((gSocket_fd = open(dev_name, O_RDWR|O_NOCTTY|O_NONBLOCK)) < 0) {
				fprintf(stderr, "device open error: %s\n", dev_name);
				goto DXL_HAL_OPEN_ERROR;
			}
			g_use_tcdrain = 1;	// FTDI device use tc Drain.
		}
	} else {
		// DXL found see if FTDI as to know if we should use drain.
		char szProcFD[20];
		char szPath[30];
		int ich;
		sprintf(szProcFD, "/proc/self/fd/%d", gSocket_fd);
		ich = readlink(szProcFD, szPath, sizeof(szPath));
			
		// Hack look for /dev/ttyUSB... actuall only look at USB    
		if ((ich > 0) && (szPath[8]=='U') && (szPath[9]=='S')&& (szPath[10]=='B'))    
			g_use_tcdrain = 1;		// FTDI use drain...
		else    
			g_use_tcdrain = 0;		// Others ACM S2.. Don't appear to.
	}

	if(gSocket_fd == -1)
		return 0;

	
	gfByteTransTime = (float)((1000.0f / baudrate) * 12.0f);
	
	memset(&newtio, 0, sizeof(newtio));
	// First try to set the baud rate directly.
	switch (int_baud) {
		case 4000000:
			newtio.c_cflag		= B4000000|CS8|CLOCAL|CREAD;
			break;
		case 3500000:
			newtio.c_cflag		= B3500000|CS8|CLOCAL|CREAD;
			break;
		case 3000000:
			newtio.c_cflag		= B3000000|CS8|CLOCAL|CREAD;
			break;
		case 2500000:
			newtio.c_cflag		= B2500000|CS8|CLOCAL|CREAD;
			break;
		case 2000000:
			newtio.c_cflag		= B2000000|CS8|CLOCAL|CREAD;
			break;
		default:
			newtio.c_cflag		= B1000000|CS8|CLOCAL|CREAD;
			break;
	}
	newtio.c_iflag		= IGNPAR;
	newtio.c_oflag		= 0;
	newtio.c_lflag		= 0;
	newtio.c_cc[VTIME]	= 0;	// time-out ? (TIME * 0.1?) 0 : disable
	newtio.c_cc[VMIN]	= 0;	// MIN ? read ? return ?? ?? ?? ?? ??

	tcflush(gSocket_fd, TCIFLUSH);
	
	if (tcsetattr(gSocket_fd, TCSANOW, &newtio) < 0) {
		printf("tcsetattr 1000000 failed try indirect %d\n\r", errno);

		// Try doing it indirect by setting to 38400 and
		// see if the USB driver supports setting non-standard
				// Try back at 38400 and setting attribute...
		newtio.c_cflag      = B38400|CS8|CLOCAL|CREAD;
		if (tcsetattr(gSocket_fd, TCSANOW, &newtio) < 0) {
			printf("tcsetattr failed %d\n\r", errno);
			goto DXL_HAL_OPEN_ERROR;
		}    
		
		// Get the settings...
		if (ioctl(gSocket_fd, TIOCGSERIAL, &serinfo) < 0) {
			printf("TIOCGSERIAL failed %d\n\r", errno);
			goto DXL_HAL_OPEN_ERROR;
		}
		
		serinfo.flags &= ~ASYNC_SPD_MASK;
		serinfo.flags |= ASYNC_SPD_CUST;
		serinfo.custom_divisor = serinfo.baud_base / baudrate;
			
		if(ioctl(gSocket_fd, TIOCSSERIAL, &serinfo) < 0) {
			printf("TIOCSSERIAL failed %d\n\r", errno);
			goto DXL_HAL_OPEN_ERROR;
		}    
	}
	
	return 1;

DXL_HAL_OPEN_ERROR:
	dxl_hal_close();
	return 0;
}

void dxl_hal_close()
{
	if(gSocket_fd != -1)
		close(gSocket_fd);
	gSocket_fd = -1;
}

int dxl_hal_set_baud( float baudrate )
{
//	struct serial_struct serinfo;
	
	if(gSocket_fd == -1)
		return 0;
	
	//USB2AX uses the CDC ACM driver for which these settings do not exist.
	/*
	if(ioctl(gSocket_fd, TIOCGSERIAL, &serinfo) < 0) {
		fprintf(stderr, "Cannot get serial info\n");
		return 0;
	}
	
	serinfo.flags &= ~ASYNC_SPD_MASK;
	serinfo.flags |= ASYNC_SPD_CUST;
	serinfo.custom_divisor = serinfo.baud_base / baudrate;
	
	if(ioctl(gSocket_fd, TIOCSSERIAL, &serinfo) < 0) {
		fprintf(stderr, "Cannot set serial info\n");
		return 0;
	}
	*/
	//dxl_hal_close();
	//dxl_hal_open(gDeviceName, baudrate);
	
	gfByteTransTime = (float)((1000.0f / baudrate) * 12.0f);
	return 1;
}

	
#ifdef DEBUG_WIRINGPI
void dxl_hal_clear(void)
{
	digitalWrite(WPD_CLEAR_PIN, HIGH);
	if (g_use_tcdrain)
		tcflush(gSocket_fd, TCIFLUSH);
	digitalWrite(WPD_CLEAR_PIN, LOW);
}

void dxl_hal_flush(void)
{
	digitalWrite(WPD_FLUSH_PIN, HIGH);
#if 0
	tcdrain(gSocket_fd);
#endif
	digitalWrite(WPD_FLUSH_PIN, LOW);
}
int dxl_hal_tx( unsigned char *pPacket, int numPacket )
{
	int iRet;
	digitalWrite(WPD_WRITE_PIN, HIGH);
	iRet = write(gSocket_fd, pPacket, numPacket);
	digitalWrite(WPD_WRITE_PIN, LOW);
	return iRet;
}

int dxl_hal_rx( unsigned char *pPacket, int numPacket )
{
	memset(pPacket, 0, numPacket);
	int iRet;
	digitalWrite(WPD_READ_PIN, HIGH);
	iRet = read(gSocket_fd, pPacket, numPacket);
	digitalWrite(WPD_READ_PIN, LOW);
	if (iRet > 0) {
		wpd_readDataToggle ^= 1;
		digitalWrite(WPD_READ_DATA_PIN, wpd_readDataToggle);
	}
	return iRet;
}

#else
void dxl_hal_clear(void)
{
	tcflush(gSocket_fd, TCIFLUSH);
}

void dxl_hal_flush(void)
{
	// Some controllers like FTDI are helped others hurt by call to drain.
	if (g_use_tcdrain)
		tcdrain(gSocket_fd);
}
int dxl_hal_tx( unsigned char *pPacket, int numPacket )
{
	return write(gSocket_fd, pPacket, numPacket);
}

int dxl_hal_rx( unsigned char *pPacket, int numPacket )
{
	memset(pPacket, 0, numPacket);
	return read(gSocket_fd, pPacket, numPacket);
}
#endif

static inline long myclock()
{
	struct timeval tv;
	gettimeofday (&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

void dxl_hal_set_timeout( int NumRcvByte )
{
	glStartTime = myclock();
	gfRcvWaitTime = (float)(gfByteTransTime*(float)NumRcvByte + 5.0f);
}

int dxl_hal_timeout(void)
{
	long time;
	
	time = myclock() - glStartTime;
	
	if(time > gfRcvWaitTime)
		return 1;
	else if(time < 0)
		glStartTime = myclock();
		
	return 0;
}
