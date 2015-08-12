/*
  BioloidController.cpp - ArbotiX Library for Bioloid Pose Engine
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

#include "BioloidEX.h"
#include "dynamixel.h"
#include "ax12.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#define delay(x)  usleep((x)*1000)
#define delayMicroseconds(us) usleep((us))
extern unsigned long millis(void);
extern unsigned long micros(void);


/* initializes serial1 transmit at baud, 8-N-1 */
BioloidControllerEx::BioloidControllerEx(long baud){
    int i;
    // setup storage
    id_ = (unsigned char *) malloc(AX12_MAX_SERVOS * sizeof(unsigned char));
    pose_ = (unsigned int *) malloc(AX12_MAX_SERVOS * sizeof(unsigned int));
    nextpose_ = (unsigned int *) malloc(AX12_MAX_SERVOS * sizeof(unsigned int));
    // initialize
    for(i=0;i<AX12_MAX_SERVOS;i++){
        id_[i] = i+1;
        pose_[i] = 512;
        nextpose_[i] = 512;
    }
    frameLength = BIOLOID_FRAME_LENGTH;
    interpolating_ = 0;
    playing = 0;
    signal_new_pose_ = 0;
    
    // dxl Initialize takes baud number, not rate...
    if(dxl_initialize(0, 1) == 0) {
        //printf("Failed to open USBDynamixel\n");
    }	

    // Lets now create our worker thread. 
     _fCancel = false;
    pthread_mutex_init(&mutex_, NULL);
    pthread_cond_init(&cond_, NULL);
	pthread_barrier_init(&_barrier, 0, 2);
    pthread_create(&tidWorker, NULL, &WorkerThreadProc, this);
    
  	// sync startup
	pthread_barrier_wait(&_barrier);



}

/* new-style setup */
void BioloidControllerEx::setup(int servo_cnt){
    int i;
    // setup storage
    id_ = (unsigned char *) malloc(servo_cnt * sizeof(unsigned char));
    pose_ = (unsigned int *) malloc(servo_cnt * sizeof(unsigned int));
    nextpose_ = (unsigned int *) malloc(servo_cnt * sizeof(unsigned int));
    // initialize
    posesize_ = servo_cnt;
    for(i=0;i<posesize_;i++){
        id_[i] = i+1;
        pose_[i] = 512;
        nextpose_[i] = 512;
    }
    interpolating_ = 0;
    playing = 0;
}
void BioloidControllerEx::setId(int index, int id){
  id_[index] = id;
}
int BioloidControllerEx::getId(int index){
  return id_[index];
}

/* load a named pose from FLASH into nextpose. */
void BioloidControllerEx::loadPose( const unsigned int * addr ){
    int i;
    posesize_ = (int)*addr; // number of servos in this pose
    for(i=0; i<posesize_; i++)
        nextpose_[i] = *(addr+1+i) << BIOLOID_SHIFT;
}

int BioloidControllerEx::poseSize(void) {
    return posesize_;
}                          // how many servos are in this pose, used by Sync()

void BioloidControllerEx::poseSize(uint8_t posesize) {
    posesize_ = posesize;
}


/* read in current servo positions to the pose. */
void BioloidControllerEx::readPose(){
    for(int i=0;i<posesize_;i++){
        pose_[i] = dxl_read_word(id_[i],AX_PRESENT_POSITION_L)<<BIOLOID_SHIFT;
        delay(25);   
    }
}
/* write pose out to servos using sync write. */
#if 0
void BioloidControllerEx::writePose(){
    int temp;

    dxl_set_txpacket_id(BROADCAST_ID);

    dxl_set_txpacket_instruction(AX_SYNC_WRITE);
    dxl_set_txpacket_parameter(0, AX_GOAL_POSITION_L);
    dxl_set_txpacket_parameter(1, 2);

    for(int i=0; i<posesize_; i++ ) {
        temp = pose_[i] >> BIOLOID_SHIFT;
        dxl_set_txpacket_parameter(2+3*i, id_[i]);
        dxl_set_txpacket_parameter(2+3*i+1, dxl_get_lowbyte(temp));
        dxl_set_txpacket_parameter(2+3*i+2, dxl_get_highbyte(temp));
    }

    dxl_set_txpacket_length((2+1)*posesize_+4);
    dxl_txrx_packet();
    dxl_get_result();   // don't care for now
}
#endif
/* set up for an interpolation from pose to nextpose over TIME 
 milliseconds by setting servo speeds. */
uint8_t  BioloidControllerEx::interpolating(void) {
    return interpolating_;
}

    
void BioloidControllerEx::interpolateSetup(int time){

    // Lets signal our worker thread it has work to do...
    pthread_mutex_lock(&mutex_);
    signal_new_pose_ = 1;
    pose_time_ = time;
    pthread_cond_signal(&cond_);
    pthread_mutex_unlock(&mutex_);

}


/* interpolate our pose, this should be called at about 30Hz. */
#define WAIT_SLOP_FACTOR 10  
void BioloidControllerEx::interpolateStep(uint8_t fWait){
}

/* get a servo value in the current pose */
int BioloidControllerEx::getCurPose(int id){
    for(int i=0; i<posesize_; i++){
        if( id_[i] == id )
            return ((pose_[i]) >> BIOLOID_SHIFT);
    }
    return -1;
}
/* get a servo value in the next pose */
int BioloidControllerEx::getNextPose(int id){
    for(int i=0; i<posesize_; i++){
        if( id_[i] == id )
            return ((nextpose_[i]) >> BIOLOID_SHIFT);
    }
    return -1;
}

int BioloidControllerEx::getNextPoseByIndex(int index){
    if (index < posesize_) {
        return (nextpose_[index] >> BIOLOID_SHIFT);
    }
    return -1;
}


/* set a servo value in the next pose */
void BioloidControllerEx::setNextPose(int id, int pos){
    for(int i=0; i<posesize_; i++){
        if( id_[i] == id ){
            nextpose_[i] = (pos << BIOLOID_SHIFT);
            return;
        }
    }
}

/* Added by Kurt */
void BioloidControllerEx::setNextPoseByIndex(int index, int pos) {  // set a servo value by index for next pose
    if (index < posesize_) {
        nextpose_[index] = (pos << BIOLOID_SHIFT);
    }
}

/* play a sequence. */
void BioloidControllerEx::playSeq( const transition_t  * addr ){
    sequence = (transition_t *) addr;
    // number of transitions left to load
    transitions = *(&sequence->time);
    sequence++;    
    // load a transition
    loadPose((const unsigned int *)*(&sequence->pose));
    interpolateSetup(*(&sequence->time));
    transitions--;
    playing = 1;
}

/* keep playing our sequence */
void BioloidControllerEx::play(){
    if(playing == 0) return;
    if(interpolating_ > 0){
        interpolateStep();
    }
    else{  // move onto next pose
        sequence++;   
        if(transitions > 0){
            loadPose((const unsigned int *)(&sequence->pose));
            interpolateSetup(*(&sequence->time));
            transitions--;
        }
        else{
            playing = 0;
        }
    }
}


int ax12GetRegister(int id, int regstart, int length) {
    int iRet;
    if (length == 1) 
        iRet = dxl_read_byte(id, regstart);
    else    
        iRet = dxl_read_word(id, regstart);    
    return ((dxl_get_result() == COMM_RXSUCCESS)? iRet : -1);
}

void ax12SetRegister(int id, int regstart, int data) {
    dxl_write_byte(id, regstart, data & 0xff);
}

void ax12SetRegister2(int id, int regstart, int data) {
    dxl_write_word(id, regstart, data);
}

//==============================================================================
// Main worker thread to do interpolation and maybe some other stuff as well. 
//==============================================================================
void *BioloidControllerEx::WorkerThreadProc(void *pv)
{
    BioloidControllerEx *pbioloid = (BioloidControllerEx*)pv;
    pthread_barrier_wait(&pbioloid->_barrier);

    printf("Bioloid Thread start\n");

    int i;
    
    // fields moved out of class... 
    unsigned int * target_pose = NULL;                // This is what is used in the secondary thread as to not conflict...
    int * servo_speed = NULL;                               // speeds for interpolation 
    int pose_size = 0;                          // lets initialize our stuff here... 
    unsigned long next_frame;                   //    

    while(!pbioloid->_fCancel)
    {
        // wait for something to do...
        pthread_mutex_lock(&pbioloid->mutex_);
        while (!pbioloid->signal_new_pose_ && !pbioloid->_fCancel) {
            pthread_cond_wait( &pbioloid->cond_, &pbioloid->mutex_ );
        }
        
        /* Under protection of the lock, complete or remove the work     */
        /* from whatever worker queue we have. Here it is simply a flag  */
        // We now copy over the new target positions...
        if (pbioloid->posesize_ != pose_size ) {
            pose_size = pbioloid->posesize_ ;
            if (target_pose)
                free(target_pose);
            target_pose = (unsigned int *) malloc(pose_size * sizeof(unsigned int));

            if (servo_speed)
                free(servo_speed);
                
            servo_speed = (int *) malloc(pose_size * sizeof(int));
        }
    
        for (i = 0; i < pose_size; i++)
            target_pose[i] = pbioloid->nextpose_[i];
            
        pbioloid->signal_new_pose_ = 0;
        
        uint8_t  frame_length = pbioloid->frameLength;

        pthread_mutex_unlock(&pbioloid->mutex_);

        // We have a new interpolation to handle, so lets set everything up.
        // Pre calculate each servos fram
        int frames = (pbioloid->pose_time_ / frame_length) + 1;
        next_frame = millis() + frame_length;
        
        // set speed each servo...
        int servos_still_moving = 0;
        for(i=0; i < pose_size; i++){
            if(target_pose[i] > pbioloid->pose_[i]) {
                servo_speed[i] = (target_pose[i] - pbioloid->pose_[i])/frames + 1;
                servos_still_moving++;
            }
            else if(target_pose[i] < pbioloid->pose_[i]){
                servo_speed[i] = (pbioloid->pose_[i]-target_pose[i])/frames + 1;
                servos_still_moving++;
            }
            else
                servo_speed[i] = 0;
        }
        pbioloid->interpolating_ = 1;

//        printf("BWT %d %d\n", frames, servos_still_moving);
        
        //  Now lets do our interpolation.  
        
        while (!pbioloid->signal_new_pose_ && !pbioloid->_fCancel && servos_still_moving && frames--) {
            // We should delay a frame time now.
            long delay_time = next_frame - millis();
            if (delay_time > 0)
                delay(delay_time);

            next_frame += frame_length;

            // Setup DXL servo packet
            // update each servo
            dxl_set_txpacket_id(BROADCAST_ID);
            dxl_set_txpacket_instruction(AX_SYNC_WRITE);
            dxl_set_txpacket_parameter(0, AX_GOAL_POSITION_L);
            dxl_set_txpacket_parameter(1, 2);
            dxl_set_txpacket_length((2+1)*servos_still_moving+4);

            // Now loop through each of the servos and output those that are still moving.
            int dxl_parm_index = 2;
            
            for(i=0; i < pose_size; i++){
                if (servo_speed[i]) {
                    int diff = target_pose[i] - pbioloid->pose_[i];
                    if(diff > 0){
                        if(diff < servo_speed[i]){
                            pbioloid->pose_[i] = target_pose[i];
                            servo_speed[i] = 0;
                            servos_still_moving--;
                        }
                        else
                            pbioloid->pose_[i] += servo_speed[i];
                    } else {
                        if((-diff) < servo_speed[i]){
                            pbioloid->pose_[i] = target_pose[i];
                            servos_still_moving--;
                            servo_speed[i] = 0;
                        }
                        else
                            pbioloid->pose_[i] -= servo_speed[i];                
                    }       
                    
                    // now add this servo to DXL output.  We might be able to optimize more
                    diff = (pbioloid->pose_[i]) >> BIOLOID_SHIFT;   // reuse diff... 
                    dxl_set_txpacket_parameter(dxl_parm_index++, pbioloid->id_[i]);
                    dxl_set_txpacket_parameter(dxl_parm_index++, dxl_get_lowbyte(diff));
                    dxl_set_txpacket_parameter(dxl_parm_index++, dxl_get_highbyte(diff));
                    // if we detected that same position as before...
                }
            }
            // Now lets output the packet. 
//            printf("dxl_txrx \n");
            dxl_txrx_packet();
            dxl_get_result();   // don't care for now
        }
        pbioloid->interpolating_ = 0;     // We finished this output...
    }
    printf("Bioloid thread exit\n");
    return 0;
}

void BioloidControllerEx::end()
{

    // try to wake up the thread.
    pthread_mutex_lock(&mutex_);
    pthread_cond_signal(&cond_);
    _fCancel = true;
    pthread_mutex_unlock(&mutex_);
    
   	// pthread_join to sync
    struct timespec ts;
    int s;

   if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
                                      /* Handle error */
   }

   ts.tv_sec += 2;

   s = pthread_timedjoin_np(tidWorker, NULL, &ts);
   if (s != 0) {
       /* Handle error */
       printf("Bioloid Thread join failed: %d\n", s);
   }

	// destroy barrier
	pthread_barrier_destroy(&_barrier);
}



