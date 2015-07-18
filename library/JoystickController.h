/*
  LinuxJoyEx.h - Library for interfacing with ArbotiX LinuxJoy
  Copyright (c) 2009-2012 Michael E. Ferguson.  All right reserved.

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

// This version was modified from the original one by Kurt to run on Linux for Raspberry Pi.
// Also I renamed some members and the like to make it easier to understand...
// Also Added support to optionally use network sockets to 
   /*
    DS4 Controller Input mapping:

    axes[0] = Left Stick Horizontal
    axes[1] = Left Stick Vertical
    axes[2] = Right Stick Horizontal
    axes[3] = L2 Trigger
    axes[4] = R2 Trigger
    axes[5] = Right Stick Vertical
    axes[6] = D-Pad Left and Right
    axes[7] = D-Pad Up and Down
    buttons[0] = Square
    buttons[1] = X
    buttons[2] = Circle
    buttons[3] = Triangle
    buttons[4] = L1 Trigger
    buttons[5] = R1 Trigger
    buttons[6] = L2 Trigger
    buttons[7] = R2 Trigger
    buttons[8] = Share Button
    buttons[9] = Options Button
    buttons[10] = Left Stick Push
    buttons[11] = Right Stick Push
    buttons[12] = PS Button
    buttons[13] = Trackpad Push
    */
    
/* DS3 Controller Input Mappings...
    axes[0] = Left Stick Horizontal
    axes[1] = Left Stick Vertical
    axes[2] = Right Stick Horizontal
    axes[3] = Right Stick Vertical
    buttons[0] = select Button
    buttons[1] = Left Stick Push
    buttons[2] = Right Stick Push
    buttons[3] = Start Button
    buttons[4] = D-UP A8-
    buttons[5] = D-Rt A9
    buttons[6] = D-DN A10
    buttons[7] = D-LF 
    buttons[8] = L2 Trigger A12
    buttons[9] = R2 Trigger A13
    buttons[10] = L1 Trigger A14
    buttons[11] = R1 Trigger A15
    buttons[12] = Triangle A16
    buttons[13] = Circle A17
    buttons[14] = X - A18
    buttons[15] = Square - a19
    buttons[16] = PS Button
 */   
    
#ifndef LinuxJoy_h
#define LinuxJoy_h
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


class LinuxJoy
{
    public:
        LinuxJoy();
        bool begin(char *pszComm);
        void end();                              // release any resources.
        int ReadMsgs();                          // must be called regularly to clean out Serial buffer

        // Define some helper functions that handle some button stuff
        bool button(int ibtn);
        bool buttonPressed(int ibtn);
        bool buttonReleased(int ibtn); 
 

        // Keep information about the different Axis of this Joystick.
        int num_of_axis;                         // Number of Axis
        int *axis_values;                        // current values for the different axis
        
        // Likewise for buttons
        int num_of_buttons;                      // Number of buttons
        char *button_values;                      // Current button values
        char *previous_button_values;             // Previous button values
        
        
        // Name of joystick...
        char name_of_joystick[80];

    private:
        int *_axis;                              // Current values for the different axis
        char *_button;
        
        //Private stuff for Linux threading and file descriptor, and open file...
        pthread_mutex_t lock;                    // A lock to make sure we don't walk over ourself...
        bool _data_changed;                       // Do we have a valid packet?
        unsigned char bInBuf[7];                 // Input buffer we use in the thread to process partial messages.
        char *_pszDevice;
        bool _fCancel;                           // Cancel any input threads.
        pthread_t tidLinuxJoy;                       // Thread Id of our reader thread...
       	pthread_barrier_t _barrier;
        static void *JoystickThreadProc(void *);
};

#endif
