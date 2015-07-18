/*
   This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

// Note: This one is hacked up to try to compile under linux...

#include "JoystickController.h"
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
#include <pthread.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>


#define JOY_DEV "/dev/js0"


/* Constructor */
LinuxJoy::LinuxJoy()
{
    _data_changed = false;
}


static ssize_t _read(int fd, void *buf, size_t count)
{
	extern ssize_t read(int fd, void *buf, size_t count);
	return read(fd, buf, count);
}

void *LinuxJoy::JoystickThreadProc(void *pv)
{
    LinuxJoy *pljoy = (LinuxJoy*)pv;
    fd_set fdset;                                // file descriptor set to wait on.
    timeval tv;                                   // how long to wait.
    int joy_fd = -1;                              // File descriptor 
    bool error_reported = false;    

    pthread_barrier_wait(&pljoy->_barrier);

    printf("Thread start(%s)\n", pljoy->_pszDevice);

    int ret;
    struct js_event js;
    int iIndex = 0; // index into which byte we should start reading into of the joystick stuff...

    while(!pljoy->_fCancel)
    {
        // First lets see if we have a joystick object yet, if not try to open.
        //
        if (joy_fd == -1)
        {
            if ((joy_fd = open(pljoy->_pszDevice,O_RDONLY)) != -1 )
            {
                ioctl(joy_fd, JSIOCGAXES , &(pljoy->num_of_axis));
                ioctl(joy_fd, JSIOCGBUTTONS , &(pljoy->num_of_buttons));
                ioctl(joy_fd, JSIOCGNAME(80), &(pljoy->name_of_joystick));

                // Keep our realtime cache of Axis and buttons
                pljoy->_axis = (int *) calloc( pljoy->num_of_axis , sizeof(int));
                pljoy->_button = (char *) calloc(  pljoy->num_of_buttons , sizeof (char));
                
                // Plus our ReadMessage which caches values
                pljoy->axis_values = (int *) calloc(pljoy->num_of_axis , sizeof(int)); 
                pljoy->button_values = (char *) calloc( pljoy->num_of_buttons , sizeof (char));
                pljoy->previous_button_values = (char *) calloc( pljoy->num_of_buttons , sizeof (char));
                
                if(! pljoy->_axis || ! pljoy->_button || ! pljoy->axis_values || ! pljoy->button_values || ! pljoy->previous_button_values)
                    return (void*)-1;      // ran out of memory???
                
                memset(pljoy->_axis, 0, pljoy->num_of_axis*sizeof(int));
                memset(pljoy->_button, 0, pljoy->num_of_buttons*sizeof(char));
                memset(pljoy->button_values, 0, pljoy->num_of_buttons*sizeof(char));  // Others will be set on first query, but need to init as this gets moved to previous...

                // Debug  will comment out later
                printf( " Joy stick detected : %s \n \t %d axis \n\t %d buttons \n\n" , pljoy->name_of_joystick ,  pljoy->num_of_axis ,  pljoy->num_of_buttons);

                fcntl( joy_fd, F_SETFL , O_NONBLOCK ); // use non - blocking methods
                error_reported = false;                 // if we lose the device... 
            }
            else
            {
                if (!error_reported) {
                    printf(" Error opening joystick \n " );
                    error_reported = true;
                }
                usleep(250000);  // try again in 1/4 second...
            }    
        }
        
        if (joy_fd != -1)
        {    
            // Lets try using select to block our thread until we have some input available...
            FD_ZERO(&fdset);
            FD_SET(joy_fd, &fdset);                   // Make sure we are set to wait for our descriptor
            tv.tv_sec = 0;
            tv.tv_usec = 250000;                          // 1/4 of a second...
                                                          // wait until some input is available...
            select(joy_fd + 1, &fdset, NULL, NULL, &tv);
            
        
            if(FD_ISSET(joy_fd, &fdset)) 
            { 
                // try to read in rest of structure... 
                while((!pljoy->_fCancel) && (ret = _read(joy_fd, ((char*)&js)+iIndex, sizeof(js)-iIndex)) > 0)
                {
                    iIndex += ret; // see how many bytes we have of the js event...
                    if (iIndex == sizeof(js)) 
                    {
                        pthread_mutex_lock(&pljoy->lock);  // make sure we keep things consistent...
                        switch(js.type & ~ JS_EVENT_INIT)
                        {
                            case JS_EVENT_AXIS :
                                pljoy->_axis [ js.number ] = js.value;
                                //printf("A %d : %d\n", js.number, js.value);
                                break;
                            case JS_EVENT_BUTTON :
                                printf("B %d : %d\n", js.number, js.value);
                                pljoy->_button [js.number ] = js.value;
                        }
                        pljoy->_data_changed = true;
                        pthread_mutex_unlock(&pljoy->lock);
                        iIndex = 0;         // start over reading from start of data...
                    }
                }
            }
            // If we get to here try sleeping for a little time
            //usleep(1000);                                 // Note: we could maybe simply block the thread until input available!
        }
    }
    close (joy_fd);
    printf("LinuxJoy - LinuxJoy thread exit\n");
    return 0;
}



bool LinuxJoy::begin(char *pszDevice)
{
    int err;
    // Create our lock to make sure we can do stuff safely
    if (pthread_mutex_init(&lock, NULL) != 0)
        return false;

    _fCancel = false;	// Flag to let our thread(s) know to abort.
    

    // remember the device name
    _pszDevice = (char*)malloc(strlen(pszDevice) + 1);
    if (!_pszDevice)
    {
        printf("malloc of device name failed\n");
        return -1;
    }    

    strcpy(_pszDevice, pszDevice);
    // Now we need to create our thread for receiving messages from the joystick...
	pthread_barrier_init(&_barrier, 0, 2);
    err = pthread_create(&tidLinuxJoy, NULL, &JoystickThreadProc, this);
    if (err != 0)
        return false;

  	// sync startup
	pthread_barrier_wait(&_barrier);

    return true;
}

void LinuxJoy::end()
{
    _fCancel = true;
    
   	// pthread_join to sync
	pthread_join(tidLinuxJoy, NULL);

	// destroy barrier
	pthread_barrier_destroy(&_barrier);
}

/* process messages coming from LinuxJoy
 *  format = 0xFF RIGHT_H RIGHT_V LEFT_H LEFT_V BUTTONS EXT CHECKSUM */
int LinuxJoy::ReadMsgs()
{
    // Probably should probably rename some of this...
    if (_data_changed)
    {
        pthread_mutex_lock(&lock);
        memcpy(axis_values, _axis, sizeof(_axis[0])*num_of_axis);

        memcpy(previous_button_values, button_values, sizeof(_button[0])*num_of_buttons);
        memcpy(button_values, _button, sizeof(_button[0])*num_of_buttons);

        _data_changed = false;                     // clear out so we know if something new comes in
        pthread_mutex_unlock(&lock);
        return 1;
    }
    return 0;
}

// Decided to not inline these as may have special case code for psuedo buttons
bool LinuxJoy::button(int ibtn) 
{
    return button_values[ibtn] != 0;
}


bool LinuxJoy::buttonPressed(int ibtn) 
{
    return ((button_values[ibtn] != 0) && (previous_button_values[ibtn] == 0));
}

bool LinuxJoy::buttonReleased(int ibtn) 
{
    return ((button_values[ibtn] == 0) && (previous_button_values[ibtn] != 0));
}

