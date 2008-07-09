

#include "ENET_GPIB_device.h"
#include "USB1408FS.h" //definition of the USB1408FS class
#include "AGILENT8648A.h"
#include "Matlab.h"
#include "AutoLocker.h"
#include "RbScanner.h"
#include "WhichLock.h"
#include "GetLock.h"
#include "Vortex6000.h"

#include <windows.h>

#include <iostream> //cin & cout commands


#include <string> //needed for string manipulations
#include <sstream> //needed for conversion of int to string
#include <vector> //needed to be able to use vectors for data storage


int main(int argc, char* argv[])

{
	//define initial state
	std::vector <double> DAQ_vector;
	std::vector <double> voltage_vector;
	std::vector <double> FREQ_vector;
	std::vector <double> FITDAQ_vector;
	std::vector <double> FITFREQ_vector;
	double offsetGHz = .7;
	double freqDiffGHz;
	double tempVoltage;
	double GHzToMHz = 1000;

	GETLOCK getLock;
	AUTOLOCKER autolocker;
	AGILENT8648A agilent8648a;
	RBSCANNER rbscanner;
	WHICHLOCK whichlock;
	MATLABPLOTTER matlabplotter;
		USB1408FS usb1408fs;

	bool notLocked = true;
	bool rightLock = false;
	bool noDiff = true;
	bool enable_lock = false;

	int i;
	int new_transition;
   

	

	
	// Scan without sidebands to find the global minimum
   	getLock.lock(&offsetGHz, matlabplotter, usb1408fs, agilent8648a);

	std::cerr << "Do you want to enable the lock?" << std::endl;
	std::cin >> enable_lock;

	if(enable_lock) {
		autolocker.enable_lock();
	}
	else {
		return (0);
	}

	//runs RbScanner and determines what is the lock transition
	notLocked = whichlock.LockedTo(offsetGHz, matlabplotter, usb1408fs);
	if (notLocked) {
		std::cout << "Eeeek! Error!" << std::endl;
	}

	// All that follows resets the lock transition if so desired
	std::cout << "Is this the right locking transition (1/0)? ";
    std::cin >> rightLock;

	while (!rightLock) 
	{
		tempVoltage = getLock.lockVoltage; // store a working lock voltage in case of errors

		std::cout << "What transition do you want to change to? " << std::endl;
		for (i = 0; i < LABELLENGTH; i++)
		{
			std::cout << "[" << i << "] " << whichlock.labels[i];
			if ((int) fmod((float) i, (float) 2) == 1) {
				std::cout << std::endl;
			}
			else
			{
				std::cout << "\t";
			}
		}
		std::cout << std::endl;
		std::cin >> new_transition;

		noDiff = whichlock.freqDiff(new_transition, &freqDiffGHz);
		if (noDiff) {
			std::cerr << "That is not a valid transition." << std::endl;
			continue;
		}

		autolocker.disable_lock();
		getLock.setLockVoltage(tempVoltage + freqDiffGHz * getLock.GHzToV, usb1408fs);

		enable_lock = false;
		std::cerr << "Do you want to enable the lock?" << std::endl;
		std::cin >> enable_lock;
		if(enable_lock) {
			autolocker.enable_lock();
		}
		else {
			return (0);
		}

		// Runs RbScanner and determines which transition we're locked to
		notLocked = whichlock.LockedTo(offsetGHz, matlabplotter, usb1408fs);
		if (notLocked) {
			std::cout << "Eeeek! Error!" << std::endl;
			std::cout << "Resetting lock..." << std::endl;

			autolocker.disable_lock();
			getLock.setLockVoltage(tempVoltage, usb1408fs);

			enable_lock = false;
			std::cerr << "Do you want to enable the lock?" << std::endl;
			std::cin >> enable_lock;
			if(enable_lock) {
				autolocker.enable_lock();
			}
			else {
				return (0);
			}

			// Runs RbScanner and determines which transition we're locked to
			notLocked = whichlock.LockedTo(offsetGHz, matlabplotter, usb1408fs);
			if (notLocked) {
				std::cout << "Lock lost. Rerun autolocker." << std::endl;
				return (0);
			}
		}

		std::cout << "Is this the right locking transition (1/0)? ";
		std::cin >> rightLock;
		
	}

	autolocker.enable_vortex_loop(notLocked, rightLock, usb1408fs);

	return 0;
};
