/*
 * BasicPWM.cpp
 *
 *	Example of basic PWM usage and fading an LED. Also shows how periods should be changed
 *
 *  Created on: May 29, 2013
 *      Author: Saad Ahmad ( http://www.saadahmad.ca )
 */

#include "PWM.h"
#include <ctime>
#include <string>
#include <unistd.h>
int main()
{
	const int delayMS = 50;
	const long periodNS = 20 * MILLISECONDS_TO_NANOSECONDS;

	PWM::Pin pinA("P8_13", periodNS); // Since both pins share the same channel, they're periods must be the same
	std::cout << "P8_13 created" << std::endl;

	PWM::Pin pinB("P8_19", periodNS);
	std::cout << "P8_19 created" << std::endl;

	int slot = PWM::GetCapeManagerSlot("P8_13");
	std::stringstream ss;
	ss << slot;
	std::cout << "Found: " << ss.str() << std::endl;


	// Enable both only after we have set the periods properly.
	// Otherwise we will have conflicts since each pin will try to set its own period and conflict with the others
	std::cout << "Before PinA enable" << std::endl;
	pinA.Enable();
	std::cout << "Before PinB enable" << std::endl;
	pinB.Enable();
	std::cout << "Pins Setup and ready to go!" << std::endl;

	// I want to do a sweep of the two servos from 500us to 2500us
	for (int pw = 500; pw < 2500; pw+=100)
	{
		pinA.SetDutyUS(pw);
		pinB.SetDutyUS(3000-pw);	// reverse this one.
		usleep(100 * 1000);		// delay for a bit maybe 5 pulses per pw
	}
	// Tell both pins to stop.
	pinA.Disable();
	pinB.Disable();
#if 0

	for (int i = 0; i < 100; i++)
	{
		std::clock_t startTime = std::clock();
		while (((clock() - startTime) * 1000.0 / CLOCKS_PER_SEC) < delayMS)
		{
			pinA.SetDutyPercent(float(i) / 100);
			pinB.SetDutyPercent(1 - float(i) / 100);
		}

	}

	// Release the pins so they are disabled and so that we can also reset their periods
	// Note if you just call Disable() then you cant change the period as both channels will be treated as in use
	pinA.Release();
	pinB.Release();

	std::cout << "Setting new periods " << std::endl;

	pinA.SetPeriodNS(periodNS * 2);
  std::cout << "New period for Pin A set" << std::endl;

	pinB.SetPeriodNS(periodNS * 2);
	std::cout << "New period for Pin B set" << std::endl;

	// Once the periods have been set we can start again
	pinA.Enable();
	std::cout << "Enabled pin A" << std::endl;

	pinB.Enable();
	std::cout << "Enabled pin B" << std::endl;

	for (int i = 0; i < 100; i++)
	{
		std::clock_t startTime = std::clock();
		while (((clock() - startTime) * 1000.0 / CLOCKS_PER_SEC) < delayMS)
		{
			pinA.SetDutyPercent(float(i) / 100);
			pinB.SetDutyPercent(1 - float(i) / 100);
		}
	}
#endif

	std::cout << "Done everything. Quiting now!" << std::endl;
}

