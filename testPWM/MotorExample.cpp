/*
 * MotorExample.cpp
 *
 *	Example of PWM used to control motors via an esc
 *
 *  Created on: May 29, 2013
 *      Author: Saad Ahmad ( http://www.saadahmad.ca )
 */

#include <string>
#include "PWM.h"
#include "Motor.h"
#include <ctime>
#include <cstdlib>

const float MIN_SPEED = 0;
const float MAX_SPEED = 100;
const int MIN_MOTOR_PULSE_TIME = 1000;
const int MAX_MOTOR_PULSE_TIME = 2000;
const std::string PIN_MOTOR_YAW("P8_13");
const std::string PIN_MOTOR_RIGHT("P8_19");
const std::string PIN_MOTOR_LEFT("P9_14");

MotorControl motorControls[] = { MotorControl(PIN_MOTOR_LEFT, MIN_SPEED, MIN_SPEED, MAX_SPEED), MotorControl(PIN_MOTOR_RIGHT, MIN_SPEED, MIN_SPEED, MAX_SPEED), MotorControl(PIN_MOTOR_YAW, MIN_SPEED, MIN_SPEED, MAX_SPEED), };


#include <signal.h>

// Stop all the motors when we interrupt the program so they don't keep going
void sig_handler(int signum)
{
	for (unsigned int iMotor = 0; iMotor < 3; iMotor++)
	{
		MotorControl & motor = motorControls[iMotor];
		motor.SetOutputValue(0);
		motor.UpdatePWMSignal();
	}

	exit(signum);
}

int main()
{
	signal(SIGINT, sig_handler);
	signal(SIGSEGV, sig_handler);
	signal(SIGQUIT, sig_handler);
	signal(SIGHUP, sig_handler);
	signal(SIGABRT, sig_handler);

	for (int iMotor = 0; iMotor < 3; iMotor++)
	{
		motorControls[iMotor].Enable();
	}

	while (true)
	{
		// Increase by 10% each time and view the output PPM signal
		for (int i = 0; i < 100; i += 10)
		{
			std::clock_t startTime = std::clock();
			while ( ((clock() - startTime) * 1000.0 / CLOCKS_PER_SEC) < 500  )
			{
				for (int iMotor = 0; iMotor < 3; iMotor++)
				{
					MotorControl & motor = motorControls[iMotor];
					motor.SetOutputValue((i + iMotor * 20) % 100); // Offset the motors a bit
					motor.UpdatePWMSignal();
				}
			}
		}
	}
}

