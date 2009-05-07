/*! \file ADF4360.h
 *  \author David M.S. Johnson 
 *  \brief Include-file for the class FPGA_daughter_board::STF_AD_FAST
 *  \section license License
 *
 *  Copyright (C) 2008 David M.S. Johnson <david.m.johnson@stanford.edu>\n
 *  This file is part of the Stanford Timing Interface (STI).
 *	
 *	This structure shamlessly derived from source code originally by Jason
 *	Hogan <hogan@stanford.edu>
 *
 *  The STI is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  The STI is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the STI.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef STF_DDS_DEVICE_H
#define STF_DDS_DEVICE_H

#include "FPGA_Device.h"
#include <math.h>

class STF_DDS_Device : public FPGA_Device
{
public:

	STF_DDS_Device(ORBManager* orb_manager, 
		std::string DeviceName, 
		std::string IPAddress,
		unsigned short ModuleNumber);
	~STF_DDS_Device();

//STI_Device functions

	// Device setup
	bool deviceMain(int argc, char **argv);

	// Device Attributes
	void defineAttributes();
	void refreshAttributes();
	bool updateAttribute(std::string key, std::string value);

	// Device Channels
	void defineChannels();
	bool readChannel(ParsedMeasurement &Measurement);

	// Device Command line interface setup
	std::string execute(int argc, char **argv);
	void definePartnerDevices(); // requires none

	// Device-specific event parsing
	void parseDeviceEvents(const RawEventMap &eventsIn, 
		boost::ptr_vector<SynchronousEvent>  &eventsOut) throw(std::exception);
	
	// Event Playback control
	virtual void stopEventPlayback() {};

	void writeData(uInt32 data);

private:
	
	short wordsPerEvent();

	uInt64 generateDDScommand(uInt32 addr, uInt32 p_registers);
	uInt32 generateDDSphase(double doublePhase);
	uInt32 generateDDSamplitude(double doubleAmplitude);
	uInt32 generateDDSfrequency(double doubleFrequency);

	bool updateDDS; //allows multiple attributes to be changed before running a timing sequence to update
	bool notInitialized; //determines if DDS has been setup with correct VCO freq, etc.. If it has, don't need to re-run every time

	bool ExternalClock;
	double extClkFreq;
	double crystalFreq;
	uInt32 PLLmultiplier; // valid values are 4-20. Multiplier for the input clock. 10*25 MHz crystal = 250 MHz -> 0x80000000 = 250 MHz
	uInt32 ChargePumpControl; // higher values increase the charge pump current
	uInt32 ProfilePinConfig; // Determines how the profile pins are configured
	uInt32 RuRd; // Ramp Up / Ramp Down control
	uInt32 ModulationLevel; // set to 0 for now

	uInt32 ActiveChannel;
	bool VCOGainControl;
	uInt32 AFPSelect;
	bool LSnoDwell;
	bool LSenable;
	bool LoadSRR;
	bool AutoclearSweep;
	bool ClearSweep;
	bool AutoclearPhase;
	bool ClearPhase;
	bool SinCos;
	//uInt8 DACCurrentControl;
	uInt32 Phase;
	double PhaseInDegrees;
	uInt32 Frequency;
	double FrequencyInMHz;
	bool AmplitudeEnable;
	uInt32 Amplitude;
	double AmplitudeInPercent;



	
	class DDS_Event : public BitLineEvent<64>
	{
	public:
		DDS_Event(double time, uInt32 command, uInt32 value, FPGA_Device* device);
		DDS_Event(const DDS_Event &copy) : BitLineEvent<64>(copy) { }

		void setupEvent();
		void loadEvent();
		void playEvent(){}; //no implementation for DDS
		void collectMeasurementData(){}; //no implementation for DDS

	private:
		uInt32 timeAddress;
		uInt32 valueAddress;
		uInt32 commandAddress;
		uInt32 time32;

		FPGA_Device* device_f;
	};
	


};


#endif
