/*
 * PWM.cpp
 *
 *  Created on: May 30, 2013
 *      Author: Saad Ahmad ( http://www.saadahmad.ca )
 */
#include "PWM.h"
#include <unistd.h>
//#define DEBUG_VERBOSE_OUTPUT 1

namespace PWM
{
	// A bunch of helper functions to get us locations in the file system.
	// They are used so that we can manipulate the pwm driver through the /sys interface
	std::string GetFullNameOfFileInDirectory(const std::string & dirName, const std::string & fileNameToFind)
	{
		DIR *pDir;

		dirent *pFile;
		if ((pDir = opendir(dirName.c_str())) == NULL)
		{
			std::cout << "Directory name: " << dirName << " doesnt exist!" << std::endl;
			throw std::bad_exception();
		}
		while ((pFile = readdir(pDir)) != NULL)
		{
			std::string currentFileName = (pFile->d_name);
			if (currentFileName.find(fileNameToFind) != std::string::npos)
			{
				return currentFileName;
			}
		}
		return std::string("");
	}

	std::string GetCapeManagerSlotsPath()
	{
		static std::string g_capeManagerSlotsPath;
		if (g_capeManagerSlotsPath.length() <= 0)
		{
#if DEBUG_VERBOSE_OUTPUT
			std::cout << "Setting up cape manager path" << std::endl;
#endif
			std::string capeBasePath("/sys/devices/");
			std::string fileName = GetFullNameOfFileInDirectory(capeBasePath, std::string("bone_capemgr."));
			g_capeManagerSlotsPath = capeBasePath + fileName + "/slots";
		}
		return g_capeManagerSlotsPath;
	}

	std::string GetOCPPath()
	{
		static std::string g_ocpPath;
		if (g_ocpPath.length() == 0)
		{
			std::string ocpBasePath("/sys/devices/");
			std::string ocpName = GetFullNameOfFileInDirectory(ocpBasePath, std::string("ocp."));
			g_ocpPath = ocpBasePath + ocpName + '/';
		}
		return g_ocpPath;
	}

	int GetCapeManagerSlot(const std::string & moduleName)
	{
#if DEBUG_VERBOSE_OUTPUT
		std::cout << "Trying to find slot for module: " << moduleName << std::endl;
#endif
		std::ifstream in(GetCapeManagerSlotsPath().c_str());
		in.exceptions(std::ios::badbit);
		int slot = -1;
		while (in >> slot)
		{
			std::string restOfLine;
			std::getline(in, restOfLine);
			if (restOfLine.find(moduleName) != std::string::npos)
			{
#if DEBUG_VERBOSE_OUTPUT
				std::cout << "Found Module: " << moduleName << " at slot: " << slot << std::endl;
#endif
				return slot;
			}
		}
#if DEBUG_VERBOSE_OUTPUT
		std::cout << "Module: " << moduleName << " not found in cape manager!" << std::endl;
#endif
		return -1;
	}
	void LoadDeviceTreeModule(const std::string & name)
	{
		int slot = GetCapeManagerSlot(name);
		if (slot == -1)
		{
#if DEBUG_VERBOSE_OUTPUT
			std::cout << "Adding Module: " << name << std::endl;
			std::cout << "Its going in: " << GetCapeManagerSlotsPath() << std::endl;
#endif
			WriteToFile(GetCapeManagerSlotsPath(), name);

			usleep(MODULE_DELAY_TIME_US);
		}
		else
		{
#if DEBUG_VERBOSE_OUTPUT
			std::cout << "Module " << name << " is already in here!" << std::endl;
#endif
		}
	}
	void UnloadDeviceTreeModule(const std::string name)
	{
		int currentSlot = GetCapeManagerSlot(name);
		if (currentSlot == -1)
		{
			std::cout << "Why is the module " << name << " being unloaded when its not in use?" << std::endl;
			throw std::bad_exception();
		}
#if DEBUG_VERBOSE_OUTPUT
		std::cout << "Unloading module: " << name << std::endl;
#endif
		WriteToFile(GetCapeManagerSlotsPath(), std::string("-") + ToString(currentSlot));

		usleep(MODULE_DELAY_TIME_US);
	}
}
