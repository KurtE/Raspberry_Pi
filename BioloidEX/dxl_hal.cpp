/**
Modification of the HAL of the Dynamixel SDK to be used with USB2AX.

Nicolas Saugnier
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <linux/serial.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include "dxl_hal.h"

int	gSocket_fd	= -1;
long	glStartTime	= 0;
float	gfRcvWaitTime	= 0.0f;
float	gfByteTransTime	= 0.0f;

int dxl_hal_open(int deviceIndex, float baudrate)
{
	struct termios newtio;
	struct serial_struct serinfo;
	char dev_name[20] = "/dev/ttyDXL";

    // Build in support to explit device - /dev/ttyDXL
	dxl_hal_close();    // Make sure any previous handle is closed
    
	if((gSocket_fd = open(dev_name, O_RDWR|O_NOCTTY|O_NONBLOCK)) < 0) {
        // We did not find our explicit one, lets try the standard default USB2AX file name
        sprintf(dev_name, "/dev/ttyACM%d", deviceIndex); // USB2AX is ttyACM
        
        if((gSocket_fd = open(dev_name, O_RDWR|O_NOCTTY|O_NONBLOCK)) < 0) {
            fprintf(stderr, "device open error: %s\n", dev_name);
            goto DXL_HAL_OPEN_ERROR;
        }
    }

	memset(&newtio, 0, sizeof(newtio));
	newtio.c_cflag		= B1000000|CS8|CLOCAL|CREAD;
	newtio.c_iflag		= IGNPAR;
	newtio.c_oflag		= 0;
	newtio.c_lflag		= 0;
	newtio.c_cc[VTIME]	= 0;	// time-out ? (TIME * 0.1?) 0 : disable
	newtio.c_cc[VMIN]	= 0;	// MIN ? read ? return ?? ?? ?? ?? ??

	tcflush(gSocket_fd, TCIFLUSH);
	tcsetattr(gSocket_fd, TCSANOW, &newtio);
	
	if(gSocket_fd == -1)
		return 0;
        
	//USB2AX uses the CDC ACM driver for which these settings do not exist.
    // However this code may also be used with Arbotix Pro... 
	if(ioctl(gSocket_fd, TIOCGSERIAL, &serinfo) >= 0) {
        fprintf(stderr, "Read serial info\n");
        serinfo.flags &= ~ASYNC_SPD_MASK;
        serinfo.flags |= ASYNC_SPD_CUST;
        serinfo.custom_divisor = serinfo.baud_base / baudrate;
        
        if(ioctl(gSocket_fd, TIOCSSERIAL, &serinfo) < 0) {
            fprintf(stderr, "Cannot set serial info\n");
            return 0;
        }
	}
    
	dxl_hal_close();
	
	gfByteTransTime = (float)((1000.0f / baudrate) * 12.0f);
	
	memset(&newtio, 0, sizeof(newtio));
	dxl_hal_close();
	
	if((gSocket_fd = open(dev_name, O_RDWR|O_NOCTTY|O_NONBLOCK)) < 0) {
		fprintf(stderr, "device open error: %s\n", dev_name);
		goto DXL_HAL_OPEN_ERROR;
	}

	newtio.c_cflag		= B1000000|CS8|CLOCAL|CREAD;
	newtio.c_iflag		= IGNPAR;
	newtio.c_oflag		= 0;
	newtio.c_lflag		= 0;
	newtio.c_cc[VTIME]	= 0;	// time-out ? (TIME * 0.1?) 0 : disable
	newtio.c_cc[VMIN]	= 0;	// MIN ? read ? return ?? ?? ?? ?? ??

	tcflush(gSocket_fd, TCIFLUSH);
	tcsetattr(gSocket_fd, TCSANOW, &newtio);
	
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

void dxl_hal_clear(void)
{
	tcflush(gSocket_fd, TCIFLUSH);
}

void dxl_hal_flush(void)
{
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
