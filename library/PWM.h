/*
 * PWM.h
 * A C++ wrapper for the EHRPWM interface
 *
 *  Created on: May 27, 2013
 *      Author: Saad Ahmad ( http://www.saadahmad.ca )
 */

#ifndef PWM_H_
#define PWM_H_
#define DEBUG_VERBOSE_OUTPUT 0
#include <string>
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <iostream>

#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>

const int MICRSECONDS_TO_NANOSECONDS = 1000;
const int MILLISECONDS_TO_MICROSECONDS = 1000;
const int MILLISECONDS_TO_NANOSECONDS = MILLISECONDS_TO_MICROSECONDS * MICRSECONDS_TO_NANOSECONDS;
const long MODULE_DELAY_TIME_US = 100 * MILLISECONDS_TO_MICROSECONDS; //  Time to wait for module to be loaded and the sysfs interface setup

namespace PWM
{
	template<class T> inline std::string ToString(const T & value)
	{
		std::stringstream ss;
		ss << value;
		return ss.str();
	}

	inline void WriteToFile(const std::string & filePath, const std::string & value)
	{
#if DEBUG_VERBOSE_OUTPUT
		std::cout << "Writing: " << value << " to: " << filePath << std::endl;
#endif
		std::ofstream out;
		out.open(filePath.c_str());
		out.exceptions(std::ios::badbit);
		out << value << std::endl;
		out.close();
	}


	// A bunch of helper functions to get us locations in the file system.
	// They are used so that we can manipulate the pwm driver through the /sys interface
	std::string GetFullNameOfFileInDirectory(const std::string & dirName, const std::string & fileNameToFind);
	std::string GetCapeManagerSlotsPath();
	std::string GetOCPPath();
	int GetCapeManagerSlot(const std::string & moduleName);
	void LoadDeviceTreeModule(const std::string & name);
	void UnloadDeviceTreeModule(const std::string name);

	class Pin
	{
	public:
		enum RunStatus
		{
			Free = -2, WaitingForSetp, Disabled, Enabled
		};
		enum Polarity
		{
			PolaritryHigh = 0, PolarityLow
		};

	private:
		std::string m_dutyFilePath;
		std::string m_periodFilePath;
		std::string m_polarityFilePath;
		std::string m_runFilePath;
		std::string m_pinName;

		long m_periodNS;
		long m_dutyNS;
		Polarity m_polarity;
		RunStatus m_runStatus;

	public:
		const std::string &GetDutyFilePath() const
		{
			return m_dutyFilePath;
		}
		const std::string &GetPeriodFilePath() const
		{
			return m_periodFilePath;
		}
		const std::string &GetPolarityFilePath() const
		{
			return m_polarityFilePath;
		}
		const std::string &GetPinName() const
		{
			return m_pinName;
		}
		const std::string &GetRunFilePath() const
		{
			return m_runFilePath;
		}

		const RunStatus &GetRunStatus() const
		{
			return m_runStatus;
		}

		const long &GetPeriodNS() const
		{
			return m_periodNS;
		}
		const Polarity &GetPolarity() const
		{
			return m_polarity;
		}
		const long &GetDutyNS() const
		{
			return m_dutyNS;
		}

	private:
		void WriteDutyNSToFile()
		{
			WriteToFile(GetDutyFilePath(), ToString(GetDutyNS()));
		}
		void WritePeriodNSToFile()
		{
			WriteToFile(GetPeriodFilePath(), ToString(GetPeriodNS()));
		}
		void WritePolarityToFile()
		{
			WriteToFile(GetPolarityFilePath(), GetPolarity() == PolaritryHigh ? std::string("0") : std::string("1"));
		}

	public:
		void SetDutyNS(const long & dutyNS)
		{
			m_dutyNS = std::min(dutyNS, GetPeriodNS());
			if (GetRunStatus() == Enabled)
				WriteDutyNSToFile();
		}
		void SetDutyUS(const int &dutyUS)
		{
			SetDutyNS((long) dutyUS * MICRSECONDS_TO_NANOSECONDS);
		}
		void SetDutyMS(const int &dutyMS)
		{
			SetDutyNS((long) dutyMS * MILLISECONDS_TO_NANOSECONDS);
		}
		void SetDutyPercent(const float &percent)
		{
			SetDutyNS(long(GetPeriodNS() * percent));
		}

		void SetPeriodNS(const long & periodNS)
		{
			if (GetRunStatus() == Enabled || GetRunStatus() == Disabled)
			{
				std::cout << "Trying to set the period but we need to release the PWM module first!" << std::endl;
				throw std::bad_exception();
				return;
			}
			m_periodNS = periodNS;
			if (GetRunStatus() == Enabled)
				WritePeriodNSToFile();
		}
		void SetPeriodUS(const int &periodUS)
		{
			SetPeriodNS((long) periodUS * MICRSECONDS_TO_NANOSECONDS);
		}
		void SetPeriodMS(const int &periodMS)
		{
			SetPeriodNS((long) periodMS * MILLISECONDS_TO_NANOSECONDS);
		}

		void SetPolarity(const Polarity & polarity)
		{
			m_polarity = polarity;
			if (GetRunStatus() == Enabled)
				WritePolarityToFile();
		}

	private:
		void SetRunStatus(const RunStatus & newRunStatus)
		{
#if DEBUG_VERBOSE_OUTPUT
			std::cout << "SetRunStatus called" << std::endl;
#endif

			if (newRunStatus != GetRunStatus())
			{
				if (newRunStatus == Disabled)
				{
					WriteToFile(GetRunFilePath(), std::string("0"));
				}
				else if (newRunStatus == Enabled)
				{
#if DEBUG_VERBOSE_OUTPUT
					std::cout << "Enabled" << std::endl;
#endif

					if (GetRunStatus() == Free)
					{
						InitPinFS();
					}
					// Force write the file values out
#if DEBUG_VERBOSE_OUTPUT
					std::cout << "Before  Write Period" << std::endl;
#endif

					WritePeriodNSToFile();
#if DEBUG_VERBOSE_OUTPUT
					std::cout << "After Write Period" << std::endl;
#endif

					WriteDutyNSToFile();
#if DEBUG_VERBOSE_OUTPUT
					std::cout << "After Write Duty" << std::endl;
#endif

					WritePolarityToFile();
#if DEBUG_VERBOSE_OUTPUT
					std::cout << "After Write Polarity" << std::endl;
#endif


					WriteToFile(GetRunFilePath(), std::string("1"));
#if DEBUG_VERBOSE_OUTPUT
					std::cout << "After Write Run" << std::endl;
#endif


				}
				else if (newRunStatus == Free)
				{
					if (GetRunStatus() != Disabled)
					{
						SetRunStatus(Disabled);
					}
					UnloadDeviceTreeModule(GetPinName());
				}
			}
			m_runStatus = newRunStatus;
		}
	public:
		void Enable()
		{
			std::cout << "Enable Called" << std::endl;
			SetRunStatus(Enabled);
		}
		void Disable()
		{
			SetRunStatus(Disabled);
		}
		void Release()
		{
			SetRunStatus(Free);
		}

	public:
		~Pin()
		{
			Release();
		}
		Pin(const std::string & pinName, const long & periodNS = 20 * MILLISECONDS_TO_NANOSECONDS, const long & dutyNS = 1 * MILLISECONDS_TO_NANOSECONDS) :
			m_pinName(pinName)
		{
			// If the pin is already in use then we need to free it!
			if (GetCapeManagerSlot(GetPinName()) != -1)
				UnloadDeviceTreeModule(GetPinName());

			m_runStatus = WaitingForSetp;

			SetPeriodNS(periodNS);
			SetDutyNS(dutyNS);
			SetPolarity(PolaritryHigh);

			InitPinFS();
		}

		void InitPinFS()
		{
#if DEBUG_VERBOSE_OUTPUT
			std::cout << "InitPinFS called" << std::endl;
#endif
			LoadDeviceTreeModule(std::string("am33xx_pwm"));
			std::string pinModule = std::string("sc_pwm_") + GetPinName();
			LoadDeviceTreeModule(pinModule);
			std::string pinInterfacePath = GetOCPPath() + GetFullNameOfFileInDirectory(GetOCPPath(), GetPinName()) + "/";
			m_dutyFilePath = pinInterfacePath + "duty";
			m_periodFilePath = pinInterfacePath + "period";
			m_polarityFilePath = pinInterfacePath + "polarity";
			m_runFilePath = pinInterfacePath + "run";


#if DEBUG_VERBOSE_OUTPUT
			std::cout << GetDutyFilePath() << std::endl;
			std::cout << GetPeriodFilePath() << std::endl;
			std::cout << GetPolarityFilePath() << std::endl;
			std::cout << GetRunFilePath() << std::endl;
#endif
		}
	};
}

#endif /* PWM_H_ */
