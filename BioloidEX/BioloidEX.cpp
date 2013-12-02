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
  speed_ = (int *) malloc(AX12_MAX_SERVOS * sizeof(int));
  // initialize
  for(i=0;i<AX12_MAX_SERVOS;i++){
    id_[i] = i+1;
    pose_[i] = 512;
    nextpose_[i] = 512;
  }
  frameLength = BIOLOID_FRAME_LENGTH;
  interpolating_ = 0;
  playing = 0;
  nextframe_ = millis();
  
  //ax12Init(baud);  
  if(dxl_initialize(0, 1000000) == 0) {
	//printf("Failed to open USBDynamixel\n");
  }	
}

/* new-style setup */
void BioloidControllerEx::setup(int servo_cnt){
  int i;
  // setup storage
  id_ = (unsigned char *) malloc(servo_cnt * sizeof(unsigned char));
  pose_ = (unsigned int *) malloc(servo_cnt * sizeof(unsigned int));
  nextpose_ = (unsigned int *) malloc(servo_cnt * sizeof(unsigned int));
  speed_ = (int *) malloc(servo_cnt * sizeof(int));
  // initialize
  poseSize = servo_cnt;
  for(i=0;i<poseSize;i++){
    id_[i] = i+1;
    pose_[i] = 512;
    nextpose_[i] = 512;
  }
  interpolating_ = 0;
  playing = 0;
  nextframe_ = millis();
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
  poseSize = (int)*addr; // number of servos in this pose
  for(i=0; i<poseSize; i++)
    nextpose_[i] = *(addr+1+i) << BIOLOID_SHIFT;
}
/* read in current servo positions to the pose. */
void BioloidControllerEx::readPose(){
  for(int i=0;i<poseSize;i++){
    pose_[i] = dxl_read_word(id_[i],AX_PRESENT_POSITION_L)<<BIOLOID_SHIFT;
    delay(25);   
  }
}
/* write pose out to servos using sync write. */
void BioloidControllerEx::writePose(){
    int temp;

    dxl_set_txpacket_id(BROADCAST_ID);

    dxl_set_txpacket_instruction(AX_SYNC_WRITE);
    dxl_set_txpacket_parameter(0, AX_GOAL_POSITION_L);
    dxl_set_txpacket_parameter(1, 2);

    for(int i=0; i<poseSize; i++ ) {
        temp = pose_[i] >> BIOLOID_SHIFT;
        dxl_set_txpacket_parameter(2+3*i, id_[i]);
        dxl_set_txpacket_parameter(2+3*i+1, dxl_get_lowbyte(temp));
        dxl_set_txpacket_parameter(2+3*i+2, dxl_get_highbyte(temp));
    }

    dxl_set_txpacket_length((2+1)*poseSize+4);
    dxl_txrx_packet();
    dxl_get_result();   // don't care for now
}

/* set up for an interpolation from pose to nextpose over TIME 
 milliseconds by setting servo speeds. */
uint8_t  BioloidControllerEx::interpolating(void) {
    return interpolating_;
}

    
void BioloidControllerEx::interpolateSetup(int time){
  int i;
  int frames = (time/frameLength) + 1;
  nextframe_ = millis() + frameLength;
  // set speed each servo...
  for(i=0;i<poseSize;i++){
    if(nextpose_[i] > pose_[i]){
      speed_[i] = (nextpose_[i] - pose_[i])/frames + 1;
    }
    else{
      speed_[i] = (pose_[i]-nextpose_[i])/frames + 1;
    }
  }
  interpolating_ = 1;
}
/* interpolate our pose, this should be called at about 30Hz. */
#define WAIT_SLOP_FACTOR 10  
void BioloidControllerEx::interpolateStep(uint8_t fWait){
  if(interpolating_ == 0) return;
  int i;
  int complete = poseSize;
  if (!fWait) {
    if (millis() < (nextframe_ - WAIT_SLOP_FACTOR)) {
      return;    // We still have some time to do something... 
    }
  }
  while(millis() < nextframe_) ;
  nextframe_ = millis() + frameLength;
  // update each servo
  for(i=0;i<poseSize;i++){
    int diff = nextpose_[i] - pose_[i];
    if(diff == 0){
      complete--;
    }
    else{
      if(diff > 0){
        if(diff < speed_[i]){
          pose_[i] = nextpose_[i];
          complete--;
        }
        else
          pose_[i] += speed_[i];
      }
      else{
        if((-diff) < speed_[i]){
          pose_[i] = nextpose_[i];
          complete--;
        }
        else
          pose_[i] -= speed_[i];                
      }       
    }
  }
  if(complete <= 0) interpolating_ = 0;
  writePose();      
}

/* get a servo value in the current pose */
int BioloidControllerEx::getCurPose(int id){
  for(int i=0; i<poseSize; i++){
    if( id_[i] == id )
      return ((pose_[i]) >> BIOLOID_SHIFT);
  }
  return -1;
}
/* get a servo value in the next pose */
int BioloidControllerEx::getNextPose(int id){
  for(int i=0; i<poseSize; i++){
    if( id_[i] == id )
      return ((nextpose_[i]) >> BIOLOID_SHIFT);
  }
  return -1;
}

int BioloidControllerEx::getNextPoseByIndex(int index){
  if (index < poseSize) {
    return (nextpose_[index] >> BIOLOID_SHIFT);
  }
  return -1;
}


/* set a servo value in the next pose */
void BioloidControllerEx::setNextPose(int id, int pos){
  for(int i=0; i<poseSize; i++){
    if( id_[i] == id ){
      nextpose_[i] = (pos << BIOLOID_SHIFT);
      return;
    }
  }
}

/* Added by Kurt */
void BioloidControllerEx::setNextPoseByIndex(int index, int pos) {  // set a servo value by index for next pose
  if (index < poseSize) {
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



