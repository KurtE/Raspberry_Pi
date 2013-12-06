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
//  id_ = (unsigned char *) malloc(AX12_MAX_SERVOS * sizeof(unsigned char));
//  pose_ = (unsigned int *) malloc(AX12_MAX_SERVOS * sizeof(unsigned int));
  nextpose_ = (unsigned int *) malloc(AX12_MAX_SERVOS * sizeof(unsigned int));
//  speed_ = (int *) malloc(AX12_MAX_SERVOS * sizeof(int));
  // initialize
  for(i=0;i<AX12_MAX_SERVOS;i++){
//    id_[i] = i+1;
//    pose_[i] = 512;
    nextpose_[i] = 512;
  }
//  frameLength = BIOLOID_FRAME_LENGTH;
//  interpolating = 0;
//  playing = 0;
//  nextframe_ = millis();
  
  //ax12Init(baud);  
  // Warning not baud that gets passed but the baud index...
  if(dxl_initialize(0, 1) == 0) {
	//printf("Failed to open USBDynamixel\n");
  }	
}

void BioloidControllerEx::setId(int index, int id){
    dxl_write_byte(AX_ID_DEVICE, AX_REG_POSE_ID_FIRST+index, id);
    dxl_get_result();   // don't care for now
}

int BioloidControllerEx::getId(int index){
    return dxl_read_byte(AX_ID_DEVICE, AX_REG_POSE_ID_FIRST+index);
}

int BioloidControllerEx::poseSize(void) {
    return dxl_read_byte(AX_ID_DEVICE,  AX_REG_POSE_SIZE);
}                          // how many servos are in this pose, used by Sync()

void BioloidControllerEx::poseSize(uint8_t posesize) {
    posesize_ = posesize;
    dxl_write_byte(AX_ID_DEVICE,  AX_REG_POSE_SIZE, posesize);
    dxl_get_result();   // don't care for now
}


/* read in current servo positions to the pose. */
void BioloidControllerEx::readPose(){
    dxl_set_txpacket_id(AX_ID_DEVICE);
    dxl_set_txpacket_instruction(AX_CMD_READ_POSE);
    dxl_set_txpacket_length(2);
    dxl_txrx_packet();
    dxl_get_result();   // don't care for now
}

/* set up for an interpolation from pose to nextpose over TIME 
 milliseconds by setting servo speeds. */
 extern "C" {
    extern unsigned char gbInstructionPacket[];
}

int BioloidControllerEx::interpolateSetup(int time){
    int temp;

    dxl_set_txpacket_id(AX_ID_DEVICE);

    dxl_set_txpacket_instruction(AX_CMD_POSE_MASK);
    
    // output the time value.
    dxl_set_txpacket_parameter(0, dxl_get_lowbyte(time));
    dxl_set_txpacket_parameter(1, dxl_get_highbyte(time));
    
    // Next 5 bytes will be the mask of which servos we are
    // outputting.  We will build as we output the data.
    uint32_t ulServoBitMask = 0x1;
    uint32_t ulServosOutputMask = 0;
    

    for(int i=0; i<posesize_; i++ ) {
        temp = nextpose_[i] >> BIOLOID_SHIFT;
        dxl_set_txpacket_parameter(7+2*i,   dxl_get_lowbyte(temp));
        dxl_set_txpacket_parameter(7+2*i+1, dxl_get_highbyte(temp));
        ulServosOutputMask |= ulServoBitMask;
        ulServoBitMask <<= 1;   // setup for next bit
    }
    
    for(int i=2; i < 2+5; i++) {
        dxl_set_txpacket_parameter(i, ulServosOutputMask & 0x7f);   // output 7 bits per byte
        ulServosOutputMask >>= 7;        
    }
    // And output the packet. 
    dxl_set_txpacket_length(2*posesize_+9);
// Debug print out stuff
#ifdef DEBUG_VERBOSE    
    uint8_t bPacketLen = gbInstructionPacket[3]+3;
    for (int i=0; i < bPacketLen; i++) {
        printf("%2x ", gbInstructionPacket[i]);
        if ((i % 10) == 9)
            printf("\n\r");
    }
    printf("\n\r");
#endif    
    dxl_txrx_packet();
    return dxl_get_result();   // don't care for now
}

/* interpolate our pose, this should be called at about 30Hz. */
#define WAIT_SLOP_FACTOR 10  

uint8_t BioloidControllerEx::interpolating(void) {
  return dxl_read_byte(AX_ID_DEVICE, AX_REG_POSE_INTERPOLATING);
}


/* Added by Kurt */
int BioloidControllerEx::getCurPoseByIndex(int index) {
  return dxl_read_word(AX_ID_DEVICE, AX_REG_SLOT_CUR_POSE_FIRST+(index*2));
}

int BioloidControllerEx::getNextPoseByIndex(int index) {  // set a servo value by index for next pose
  if (index < posesize_) {
    return (nextpose_[index] >> BIOLOID_SHIFT);
  }
  return -1;
}

void BioloidControllerEx::setNextPoseByIndex(int index, int pos) {  // set a servo value by index for next pose
  if (index < posesize_) {
    nextpose_[index] = (pos << BIOLOID_SHIFT);
  }
}
