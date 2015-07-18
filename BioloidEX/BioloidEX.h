/*
  BioloidController.h - ArbotiX Library for Bioloid Pose Engine
  Copyright (c) 2008-2012 Michael E. Ferguson.  All right reserved.

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

#ifndef BioloidEx_h
#define BioloidEx_h

/* poses:
 *  PROGMEM prog_uint16_t name[ ] = {4,512,512,482,542}; // first number is # of servos
 * sequences:
 *  PROGMEM transition_t name[] = {{NULL,count},{pose_name,1000},...} 
 */

#include <stdint.h>
#include <pthread.h>

// Added defines to make code easier to read.
#define AX_ID_DEVICE        0xFD
#define AX_ID_BROADCAST     0xFE

#define AX_CMD_READ_DATA    0x02
#define AX_CMD_WRITE_DATA	0x03
#define AX_CMD_ACTION		0x05
#define AX_CMD_RESET        0x06
#define AX_CMD_BOOTLOAD     0x08 
#define AX_CMD_SYNC_WRITE	0x83
#define AX_CMD_SYNC_READ    0x84


/* pose engine runs at 30Hz (33ms between frames) 
   recommended values for interpolateSetup are of the form X*BIOLOID_FRAME_LENGTH - 1 */
#define BIOLOID_FRAME_LENGTH      20
/* we need some extra resolution, use 13 bits, rather than 10, during interpolation */
#define BIOLOID_SHIFT             3

/** a structure to hold transitions **/
typedef struct{
    unsigned int * pose;    // addr of pose to transition to 
    int time;               // time for transition
} transition_t; 

/** Bioloid Controller Class for mega324p/644p clients. **/
class BioloidControllerEx
{
  public:
    /* For compatibility with legacy code */
    BioloidControllerEx(long baud);               // baud usually 1000000
    
    /* New-style constructor/setup */ 
    BioloidControllerEx() {};
    void setup(int servo_cnt);
    void end();     

    /* Pose Manipulation */
    void loadPose( const unsigned int * addr ); // load a named pose from FLASH  
    void readPose();                            // read a pose in from the servos  
    void writePose();                           // write a pose out to the servos
    int getCurPose(int id);                     // get a servo value in the current pose
    int getNextPose(int id);                    // get a servo value in the next pose
    int getNextPoseByIndex(int index);           // get a servo value in the next pose
    void setNextPose(int id, int pos);          // set a servo value in the next pose
    void setNextPoseByIndex(int index, int pos);  // set a servo value by index for next pose
    void setId(int index, int id);              // set the id of a particular storage index
    int getId(int index);                       // get the id of a particular storage index
    
    /* Pose Engine */
    void interpolateSetup(int time);            // calculate speeds for smooth transition
    void interpolateStep(uint8_t fWait=true);                     // move forward one step in current interpolation  
    uint8_t interpolating(void);                // are we in an interpolation? 0=No, 1=Yes
    unsigned char runningSeq;                   // are we running a sequence? 0=No, 1=Yes 
    int poseSize(void);                         // how many servos are in this pose, used by Sync()
    void poseSize(uint8_t posesize);            // set the pose size
    
    bool _fCancel;                              // Cancel worker thread.
    pthread_t tidWorker;                        // Thread Id of our worker thread...
   	pthread_barrier_t _barrier;
    pthread_cond_t  cond_; 
    pthread_mutex_t mutex_;
    bool signal_new_pose_;                      // The main thread is signaling 
    int pose_time_;                             // How long is the next pose supposed to take
    static void *WorkerThreadProc(void *);

    // Kurt's Hacks
    uint8_t frameLength;                        // Allow variable frame lengths, to test...

    /* to interpolate:
     *  bioloid.loadPose(myPose);
     *  bioloid.interpolateSetup(67);
     *  while(bioloid.interpolating > 0){
     *      bioloid.interpolateStep();
     *      delay(1);
     *  }
     */

    /* Sequence Engine */
    void playSeq( const transition_t * addr );  // load a sequence and play it from FLASH
    void play();                                // keep moving forward in time
    unsigned char playing;                      // are we playing a sequence? 0=No, 1=Yes

    /* to run the sequence engine:
     *  bioloid.playSeq(walk);
     *  while(bioloid.playing){
     *      bioloid.play();
     *  }
     */
    
  private:  
    uint8_t posesize_;                          // cache the actual pose size.
    unsigned int * pose_;                       // the current pose, updated by Step(), set out by Sync()
    unsigned int * nextpose_;                   // the destination pose, where we put on load
    unsigned char * id_;                        // servo id for this index

    transition_t * sequence;                    // sequence we are running
    int transitions;                            // how many transitions we have left to load
    uint8_t interpolating_ ;                    // Actual state.
   
};

// BUGBUG:: added some AX12 helper functions here. Could/Should create ax12.cpp/h...
// Some helper functions to map to DXL functionality
extern int ax12GetRegister(int id, int regstart, int length);
extern void ax12SetRegister(int id, int regstart, int data);
extern void ax12SetRegister2(int id, int regstart, int data);


#endif

