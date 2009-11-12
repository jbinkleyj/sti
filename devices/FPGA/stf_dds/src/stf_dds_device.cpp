/*! \file ADF4360.h
 *  \author David M.S. Johnson 
 *  \brief source for the class STF_DDS_Device
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

#include "stf_dds_device.h"


STF_DDS_Device::STF_DDS_Device(ORBManager* orb_manager, std::string configFilename) :
FPGA_Device(orb_manager, "DDS", configFilename)
{

	dds_parameters.push_back(DDS_Parameters());
	dds_parameters.push_back(DDS_Parameters());
	dds_parameters.push_back(DDS_Parameters());
	dds_parameters.push_back(DDS_Parameters());

	updateDDS = false;
	IOUpdate = true;
	notInitialized = true;
	
	ExternalClock = false;
	extClkFreq = 25.0; // in MHz

	crystalFreq = 25.0; // in MHz
	sampleFreq = 500.0; //in MSPS
	SYNC_CLK = sampleFreq / 4;	//In MHz, even though sampleFreq is in MSPS
	PLLmultiplier = static_cast<uInt32>(floor(sampleFreq / crystalFreq)); // valid values are 4-20. Multiplier for the input clock. 20*25 MHz crystal = 500 MHz -> 0x80000000 = 250 MHz
	
	ActiveChannel = 0; //corresponds to channel 0
	VCOEnable = true;

	ModulationLevel = 0; // set to 0 for now

}

STF_DDS_Device::~STF_DDS_Device()
{
}

bool STF_DDS_Device::deviceMain(int argc, char **argv)
{
	return false;
}
	
void STF_DDS_Device::defineAttributes()
{

	//Attributes not set in serial commands
	addAttribute("External Clock", "false", "true, false"); //Use external clock?
	addAttribute("External Clock Frequency", extClkFreq); //External Clock Frequency in MHz
	
	addAttribute("Active Channel", "0", "0, 1, 2, 3"); // can set channel 0,1,2,3 or any combination i.e. 0xF = all channels
	addAttribute("DAC Current", "High", "Off, Low, Medium, High"); //DAC full scale current control, 2 bits, "11" default, "10" best noise
	
	addAttribute("Mode", "None", "None, Function Generator, Sweep, 2 Level Modulation"); //main selector switch
	
	//stuff needed for sweeps
	// Linear Sweep Rate
	addAttribute("Rising Sweep Ramp Rate(%)", 0); //8 bits
	addAttribute("Falling Sweep Ramp Rate(%)", 0); //8 bits
	// Rising Delta Word
	addAttribute("Rising Delta Word", dds_parameters.at(ActiveChannel).risingDeltaWordInMHz); //32 bits
	// Falling Delta Word
	addAttribute("Falling Delta Word", dds_parameters.at(ActiveChannel).fallingDeltaWordInMHz); //32 bits
	// Sweep End Point
	addAttribute("Sweep End Point", dds_parameters.at(ActiveChannel).sweepEndPointInMHz); //32 bits
	// sweep go button
	addAttribute("Start Sweep", "down", "up, down");
	
	//Phase
	addAttribute("Phase", dds_parameters.at(ActiveChannel).PhaseInDegrees); //14 bits
	//Frequency
	addAttribute("Frequency", dds_parameters.at(ActiveChannel).FrequencyInMHz); //32 bits
	//Amplitude
	addAttribute("Amplitude", dds_parameters.at(ActiveChannel).AmplitudeInPercent); //10 bits allowed range 0-1023
}

void STF_DDS_Device::refreshAttributes()
{
	
	// All attributes are stored in c++, none are on the fpga
	//Attributes not set in serial commands
	setAttribute("External Clock", (ExternalClock ? "true" : "false")); //Use external clock?
	setAttribute("External Clock Frequency", extClkFreq); //External Clock Frequency in MHz
	
	setAttribute("Active Channel", ActiveChannel); // can set channel 0,1,2,3 or any combination i.e. 0xF = all channels
	setAttribute("DAC Current", dds_parameters.at(ActiveChannel).DACCurrentControl); //DAC full scale current control, 2 bits, "11" default, "10" best noise
	
	if(dds_parameters.at(ActiveChannel).mode == 0)
		setAttribute("Mode", "None"); //main selector switch
	else if(dds_parameters.at(ActiveChannel).mode == 1)
		setAttribute("Mode", "Function Generator"); //main selector switch
	else if(dds_parameters.at(ActiveChannel).mode == 2)
		setAttribute("Mode", "Sweep"); //main selector switch
	else if(dds_parameters.at(ActiveChannel).mode == 3)
		setAttribute("Mode", "2 Level Modulation"); //main selector switch
	else
		setAttribute("Mode", "None"); //main selector switch
	
	//stuff needed for sweeps
	// Linear Sweep Rate
//	setAttribute("Rising Sweep Ramp Rate(%)", dds_parameters.at(ActiveChannel).risingSweepRampRateInPercent); //8 bits
//	setAttribute("Falling Sweep Ramp Rate(%)", dds_parameters.at(ActiveChannel).fallingSweepRampRateInPercent); //8 bits
	// Rising Delta Word
//	setAttribute("Rising Delta Word", dds_parameters.at(ActiveChannel).risingDeltaWordInMHz); //32 bits
	// Falling Delta Word
//	setAttribute("Falling Delta Word", dds_parameters.at(ActiveChannel).fallingDeltaWordInMHz); //32 bits
	// Sweep End Point
//	setAttribute("Sweep End Point", dds_parameters.at(ActiveChannel).sweepEndPointInMHz); //32 bits
	// sweep go button
//	setAttribute("Start Sweep", (dds_parameters.at(ActiveChannel).startSweep ? "up" : "down"));
	
	//Phase
	setAttribute("Phase", dds_parameters.at(ActiveChannel).PhaseInDegrees); //14 bits
	//Frequency
	setAttribute("Frequency", dds_parameters.at(ActiveChannel).FrequencyInMHz); //32 bits
	//Amplitude
	setAttribute("Amplitude", dds_parameters.at(ActiveChannel).AmplitudeInPercent); //10 bits allowed range 0-1023
	

}

bool STF_DDS_Device::updateAttribute(std::string key, std::string value)
{

	double tempDouble;

	bool successDouble = stringToValue(value, tempDouble);

	//uInt32 tempUInt32;
	//bool successUInt32 = stringToValue(value, tempUInt32);

	bool success = false;

	bool stateChange = false; // we use this to determine if data should be written to the DDS. don't update the DDS if it's state hasn't changed

	STI::Types::TDDS ddsValue;
	ddsValue.ampl.number( dds_parameters.at(ActiveChannel).AmplitudeInPercent );
	ddsValue.freq.number( dds_parameters.at(ActiveChannel).FrequencyInMHz );
	ddsValue.phase.number( dds_parameters.at(ActiveChannel).PhaseInDegrees );

	RawEvent rawEvent(50000, 0, 0);
	rawEvent.setChannel(ActiveChannel); //set the channel to the current active channel

	if(key.compare("Mode") == 0)
	{
		success = true;
		dds_parameters.at(ActiveChannel).startSweep = false; //just make sure we don't have a profile pin set high
		dds_parameters.at(ActiveChannel).ClearSweep = true; //always want to keep the sweep counter cleared, unless we're actively sweeping

		if(value.compare("None") == 0 && dds_parameters.at(ActiveChannel).mode != 0)
		{
			updateDDS = false;
			dds_parameters.at(ActiveChannel).AFPSelect = 0;
			dds_parameters.at(ActiveChannel).LinearSweepEnable = false;
			dds_parameters.at(ActiveChannel).AmplitudeEnable = false;
			std::cerr << "My mode is None" << std::endl;
			dds_parameters.at(ActiveChannel).mode = 0;
			stateChange = true; //state has changed, so we need to update the DDS
		}
		else if(value.compare("Function Generator") == 0 && dds_parameters.at(ActiveChannel).mode != 1)
		{
			updateDDS = true;
			dds_parameters.at(ActiveChannel).AFPSelect = 0;
			dds_parameters.at(ActiveChannel).LinearSweepEnable = false;
			dds_parameters.at(ActiveChannel).AmplitudeEnable = true;
			std::cerr << "My mode is Function Generator" << std::endl;
			dds_parameters.at(ActiveChannel).mode = 1;
			stateChange = true; //state has changed, so we need to update the DDS
		}
		else if(value.compare("Sweep") == 0 && dds_parameters.at(ActiveChannel).mode != 2)
		{
			updateDDS = true;
			dds_parameters.at(ActiveChannel).AFPSelect = 2;
			dds_parameters.at(ActiveChannel).LinearSweepEnable = true;
			dds_parameters.at(ActiveChannel).AmplitudeEnable = false;
			std::cerr << "My mode is Sweep" << std::endl;
			dds_parameters.at(ActiveChannel).mode = 2;
			stateChange = true; //state has changed, so we need to update the DDS
		}
		else if(value.compare("2 Level Modulation") == 0  && dds_parameters.at(ActiveChannel).mode != 3)
		{
			updateDDS = true;
			dds_parameters.at(ActiveChannel).AFPSelect = 2;
			dds_parameters.at(ActiveChannel).LinearSweepEnable = false;
			dds_parameters.at(ActiveChannel).AmplitudeEnable = false;
			std::cerr << "My mode is 2 Level Modulation" << std::endl;
			dds_parameters.at(ActiveChannel).mode = 3;
			stateChange = true; //state has changed, so we need to update the DDS
		}
		else
			success = false;

		rawEvent.setValue( "Switch Mode" ); //this will write all set up commands
	}

	else if(key.compare("Start Sweep") == 0)
	{
		success = true;
		dds_parameters.at(ActiveChannel).ClearSweep = false;


		if(value.compare("down") == 0 && dds_parameters.at(ActiveChannel).startSweep) //only sets parameter if it is different
		{
			dds_parameters.at(ActiveChannel).startSweep = false;
			stateChange = true;
		}
		else if(value.compare("up") == 0 && !dds_parameters.at(ActiveChannel).startSweep) //only sets parameter if it is different
		{
			dds_parameters.at(ActiveChannel).startSweep = true;
			stateChange = true;
		}
		else
			success = false;

		rawEvent.setValue( "0x03" ); // 
		//rawEvent.setValue( "Non Register Command" ); //this will create a dummy event that doesn't write to DDS registers
	}
	
	else if(key.compare("External Clock") == 0)
	{
		success = true;
		if(value.compare("false") == 0 && ExternalClock)
		{
			ExternalClock = false;
			stateChange = true;
		}
		else if(value.compare("true") == 0 && !ExternalClock)
		{
			ExternalClock = true;
			stateChange = true;
		}
		else
			success = false;

		if(ExternalClock)
			PLLmultiplier = static_cast<uInt32>(floor(sampleFreq / extClkFreq)); 
		else
			PLLmultiplier = static_cast<uInt32>(floor(sampleFreq / crystalFreq)); 

		rawEvent.setValue( "Switch Mode" ); //	addr = 0x01 to change PLL Multiplier, so just run switch mode
	}		
	else if(key.compare("External Clock Frequency") == 0 && successDouble)
	{
		success = true;
		if(extClkFreq != tempDouble)
		{
			extClkFreq = tempDouble;
			stateChange = true;
		}
		
		if(ExternalClock)
		{
			PLLmultiplier = static_cast<uInt32>(floor(sampleFreq / extClkFreq)); 
			rawEvent.setValue( "Switch Mode" ); //	addr = 0x01 to change PLL Multiplier, so just run switch mode
		}
	}
	else if(key.compare("Phase") == 0 && successDouble)
	{
		success = true;
		if(tempDouble >= 0 && dds_parameters.at(ActiveChannel).PhaseInDegrees != tempDouble)
		{
//			dds_parameters.at(ActiveChannel).PhaseInDegrees = tempDouble;
			ddsValue.phase.number( tempDouble );
			stateChange = true;
		}
		else
		{
			ddsValue.phase.number( dds_parameters.at(ActiveChannel).PhaseInDegrees );
		}
	
		ddsValue.ampl.number( dds_parameters.at(ActiveChannel).AmplitudeInPercent );
		ddsValue.freq.number( dds_parameters.at(ActiveChannel).FrequencyInMHz );
		rawEvent.setValue(ddsValue);
	}
	else if(key.compare("Frequency") == 0 && successDouble)
	{
		success = true;
		if(tempDouble >= 0.0 && tempDouble <= 250.0 && dds_parameters.at(ActiveChannel).FrequencyInMHz != tempDouble)
		{
//			dds_parameters.at(ActiveChannel).FrequencyInMHz = tempDouble;
			ddsValue.freq.number( tempDouble );
			stateChange = true;
		}
		else
		{
			ddsValue.freq.number( dds_parameters.at(ActiveChannel).FrequencyInMHz );
		}
		
		ddsValue.ampl.number( dds_parameters.at(ActiveChannel).AmplitudeInPercent );
		ddsValue.phase.number( dds_parameters.at(ActiveChannel).PhaseInDegrees );
		rawEvent.setValue(ddsValue);
	}
	
	else if(key.compare("Amplitude") == 0 && successDouble)
	{
		success = true;
		if(tempDouble >= 0.0 && tempDouble <= 100.0 && dds_parameters.at(ActiveChannel).AmplitudeInPercent != tempDouble)
		{
//			dds_parameters.at(ActiveChannel).AmplitudeInPercent = tempDouble;
			ddsValue.ampl.number( tempDouble );
			stateChange = true;
		}
		else
		{
			ddsValue.ampl.number( dds_parameters.at(ActiveChannel).AmplitudeInPercent );
		}
		
		ddsValue.freq.number( dds_parameters.at(ActiveChannel).FrequencyInMHz );
		ddsValue.phase.number( dds_parameters.at(ActiveChannel).PhaseInDegrees );
		rawEvent.setValue(ddsValue);
	}
	
	else if(key.compare("Rising Sweep Ramp Rate(%)") == 0 && successDouble)
	{
		success = true;
		if(tempDouble <= 99.0 && tempDouble >= 0.0)
		{
			if(dds_parameters.at(ActiveChannel).risingSweepRampRateInPercent != tempDouble)
			{
				dds_parameters.at(ActiveChannel).risingSweepRampRate = static_cast<uInt32>(floor(((100.0 - tempDouble) / 100.0) * 255.0));
				dds_parameters.at(ActiveChannel).risingSweepRampRateInPercent = tempDouble;
				stateChange = true;
			}
		}
		else
		{
			std::cerr << "Enter a value between 0 and 100 (non-inclusive)" << std::endl;
			//std::cerr << "temp double is: " << tempDouble << std::endl;
		}
		
		rawEvent.setValue( "Set Sweep Parameters" ); //	sets all sweep parameters
	}
	
	else if(key.compare("Falling Sweep Ramp Rate(%)") == 0 && successDouble)
	{
		success = true;
		if(tempDouble <= 99.0 && tempDouble >= 0.0)
		{
			if(dds_parameters.at(ActiveChannel).fallingSweepRampRateInPercent != tempDouble)
			{
				dds_parameters.at(ActiveChannel).fallingSweepRampRate = static_cast<uInt32>(floor(((100.0 - tempDouble) / 100.0) * 255.0));
				dds_parameters.at(ActiveChannel).fallingSweepRampRateInPercent = tempDouble;
				stateChange = true;
			}
		}
		else
		{
			std::cerr << "Enter a value between 0 and 100 (non-inclusive)" << std::endl;
			//std::cerr << "temp double is: " << tempDouble << std::endl;
		}
		
		rawEvent.setValue( "Set Sweep Parameters" ); //	sets all sweep parameters
	}
	
	else if(key.compare("Rising Delta Word") == 0 && successDouble)
	{
		success = true;
		if(dds_parameters.at(ActiveChannel).risingDeltaWordInMHz != tempDouble)
		{
			dds_parameters.at(ActiveChannel).risingDeltaWord  = generateDDSfrequency(tempDouble);
			dds_parameters.at(ActiveChannel).risingDeltaWordInMHz = tempDouble;
			stateChange = true;
		}

		rawEvent.setValue( "Set Sweep Parameters" ); //	sets all sweep parameters
	}
	
	else if(key.compare("Falling Delta Word") == 0 && successDouble)
	{
		success = true;
		if(dds_parameters.at(ActiveChannel).fallingDeltaWordInMHz != tempDouble)
		{
			dds_parameters.at(ActiveChannel).fallingDeltaWord = generateDDSfrequency(tempDouble);
			dds_parameters.at(ActiveChannel).fallingDeltaWordInMHz = tempDouble;
			stateChange = true;
		}
		
		rawEvent.setValue( "Set Sweep Parameters" ); //	sets all sweep parameters
	}
	
	else if(key.compare("Sweep End Point") == 0 && successDouble)
	{
		success = true;
		
		if((tempDouble > dds_parameters.at(ActiveChannel).FrequencyInMHz && tempDouble <= 250.0) || (tempDouble < dds_parameters.at(ActiveChannel).FrequencyInMHz && tempDouble > 0.0 && dds_parameters.at(ActiveChannel).LinearSweepEnable == false)) //for sweep mode, can't have end point < start point
		{
			if(dds_parameters.at(ActiveChannel).sweepEndPointInMHz != tempDouble)
			{
				dds_parameters.at(ActiveChannel).sweepEndPoint = generateDDSfrequency(tempDouble);
				dds_parameters.at(ActiveChannel).sweepEndPointInMHz = tempDouble;
				stateChange = true;
			}
		}
			

		rawEvent.setValue( "Set Sweep Parameters" ); //	sets all sweep parameters
	}
	
	else if(key.compare("Active Channel") == 0)
	{
		success = true;

		if(value.compare("0") == 0 && ActiveChannel != 0)
		{
			rawEvent.setChannel(0);
			stateChange = true;
		}
		else if(value.compare("1") == 0 && ActiveChannel != 1)
		{
			rawEvent.setChannel(1);
			stateChange = true;
		}
		else if(value.compare("2") == 0 && ActiveChannel != 2)
		{
			rawEvent.setChannel(2);
			stateChange = true;
		}
		else if(value.compare("3") == 0 && ActiveChannel != 3)
		{
			rawEvent.setChannel(3);
			stateChange = true;
		}
		else
			success = false;

		rawEvent.setValue( "Switch Mode" ); //	addr = 0x03 to change DAC Current, so just run switch mode
	}
	else if(key.compare("DAC Current") == 0)
	{
		success = true;

		if(value.compare("Off") == 0 && dds_parameters.at(ActiveChannel).DACCurrentControl != 0)
		{
			dds_parameters.at(ActiveChannel).DACCurrentControl = 0;
			stateChange = true;
		}
		else if(value.compare("Low") == 0 && dds_parameters.at(ActiveChannel).DACCurrentControl != 1)
		{
			dds_parameters.at(ActiveChannel).DACCurrentControl = 1;
			stateChange = true;
		}
		else if(value.compare("Medium") == 0 && dds_parameters.at(ActiveChannel).DACCurrentControl != 2)
		{
			dds_parameters.at(ActiveChannel).DACCurrentControl = 2;
			stateChange = true;
		}
		else if(value.compare("High") == 0 && dds_parameters.at(ActiveChannel).DACCurrentControl != 3)
		{
			dds_parameters.at(ActiveChannel).DACCurrentControl = 3;
			stateChange = true;
		}
		else
			success = false;

		rawEvent.setValue( "Switch Mode" ); //	addr = 0x03 to change DAC Current, so just run switch mode
	}
	else
	{
		success = false;
	}

	if(success && updateDDS && stateChange)
	{
		playSingleEvent(rawEvent); //runs parseDeviceEvents on rawEvent and executes a short timing sequence
	}

	return success;

}

void STF_DDS_Device::defineChannels()
{
	addOutputChannel(0, ValueDDSTriplet);
	addOutputChannel(1, ValueDDSTriplet);
	addOutputChannel(2, ValueDDSTriplet);
	addOutputChannel(3, ValueDDSTriplet);
}

std::string STF_DDS_Device::execute(int argc, char **argv)
{
	return "";
}

void STF_DDS_Device::definePartnerDevices()
{
}

short STF_DDS_Device::wordsPerEvent()
{
	return 3;
}


void STF_DDS_Device::parseDeviceEvents(const RawEventMap &eventsIn, 
		boost::ptr_vector<SynchronousEvent>  &eventsOut) throw(std::exception)
{
	
	RawEventMap::const_iterator events;

	double eventSpacing = 800; //minimum time between events, same as holdoff time
	double eventTime = 0;
	double holdoffTime = 0;
	uInt32 eventTypeSize = 1;
	uInt32 sweepModeChangeSize = 5+4+1;

	bool successOutputAddr = false;
	uInt32 outputAddr = 0;

	string tempErr;

	uInt32 RSSR;
	double numberOfPoints;

	DDSEventType freqEvent = None;
	DDSEventType amplEvent = None;
	DDSEventType phaseEvent = None;

	double startVal, endVal, rampTime;
	double newFreq, newAmpl, newPhase;
	uInt32 deltaWord;


	//main loop over rawEvents
	for(events = eventsIn.begin(); events != eventsIn.end(); events++)
	{
		if(events->second.size() > 1)
			throw EventConflictException(events->second.at(0), events->second.at(1),
						"The DDS currently only supports one event at each time.");

		// compute total setback needed
		holdoffTime = events->first; //start time before we add in the setbacks

		for(unsigned i = 0; i < events->second.size(); i++) //step through all channels at this time
		{
			freqEvent = None;
			amplEvent = None;
			phaseEvent = None;

			dds_parameters.at(events->second.at(i).channel()).ClearSweep = true; //always keep the sweep counter cleared, unless we're actively sweeping
			dds_parameters.at(events->second.at(i).channel()).startSweep = false;	

			if(events->second.at(i).type() == ValueDDSTriplet)
			{
				eventTypeSize = 0;
				
				//frequency event
				if( events->second.at(i).ddsValue().freq._d() == STI::Types::DDSNumber )
				{
					newFreq = events->second.at(i).ddsValue().freq.number();
					
					if( newFreq != dds_parameters.at(events->second.at(i).channel()).FrequencyInMHz)
					{
						if( newFreq < 0 || newFreq > sampleFreq)
						{
							tempErr= string("DDS frequency must be between 0 and ") 
								+ valueToString(sampleFreq) + string(" MHz.");
							throw EventParsingException(events->second.at(0), tempErr);
						}

						dds_parameters.at(events->second.at(0).channel()).Frequency = generateDDSfrequency(newFreq);
						dds_parameters.at(events->second.at(0).channel()).FrequencyInMHz = newFreq;

						freqEvent = Change;
						eventTypeSize += 1;
					}
				}
				//amplitude event
				if( events->second.at(i).ddsValue().ampl._d() == STI::Types::DDSNumber )
				{
					newAmpl = events->second.at(i).ddsValue().ampl.number();
					
					if( newAmpl != dds_parameters.at(events->second.at(i).channel()).AmplitudeInPercent)
					{
						if( newAmpl < 0 || newAmpl > 100 )
						{
							throw EventParsingException(events->second.at(0), 
								"DDS amplitude must be between 0 and 100 percent.");
						}
						
						dds_parameters.at(events->second.at(0).channel()).Amplitude = generateDDSamplitude(newAmpl);
						dds_parameters.at(events->second.at(0).channel()).AmplitudeInPercent = newAmpl;

						amplEvent = Change;
						eventTypeSize += 1;
					}
				}				
				
				//phase event
				if( events->second.at(i).ddsValue().phase._d() == STI::Types::DDSNumber )
				{
					newPhase = events->second.at(i).ddsValue().phase.number();
					
					if( newPhase != dds_parameters.at(events->second.at(i).channel()).PhaseInDegrees)
					{
						if( newPhase < 0 || newPhase > 360)
						{
							throw EventParsingException(events->second.at(0), 
								"DDS phase must be between 0 and 360 degrees.");
						}

						dds_parameters.at(events->second.at(0).channel()).Phase = generateDDSphase(newPhase);
						dds_parameters.at(events->second.at(0).channel()).PhaseInDegrees = newPhase;

						phaseEvent = Change;
						eventTypeSize += 1;
					}
				}

				//frequency sweep
				if( events->second.at(i).ddsValue().freq._d() == STI::Types::DDSSweep )
				{
					freqEvent = Sweep;
					eventTypeSize += sweepModeChangeSize;

					startVal = events->second.at(i).ddsValue().freq.sweep().startVal;	//in MHz
					endVal   = events->second.at(i).ddsValue().freq.sweep().endVal;		//in MHz
					rampTime = events->second.at(i).ddsValue().freq.sweep().rampTime;	//in ns

					if( startVal < 0 || startVal > sampleFreq || endVal < 0 || endVal > sampleFreq )
					{
						tempErr= string("Ramp frequencies must be between 0 and ") 
							+ valueToString(sampleFreq) + string(" MHz.");
						throw EventParsingException(events->second.at(0), tempErr);
					}

					if(rampTime < 0)
						throw EventParsingException(events->second.at(0), "The sweep time must be positive.");

					//SSR = ;
					//deltaWordInMHz = ;  //AD9959 spec sheet, page 25: deltaFreq = (DW / 2^32) * SYSCLK (Hz)
					//DW = ;  //AD9959 spec sheet, page 25: deltaPhase = (DW / 2^14) * 360 (degrees)
					//DW = ;  //AD9959 spec sheet, page 25: deltaAmpl = (DW / 2^10) * 1024 (DAC full scale current)
			
					//deltaF = endVal - startVal;

					RSSR = 1;
					numberOfPoints = (rampTime * SYNC_CLK) / (RSSR * 1000);		// rampTime (ns) * SYNC_CLK (MHz) --> 10^-3

					if(numberOfPoints < 1)
					{
						tempErr= string("The minimum sweep time is ") 
							+ valueToString(1000 / SYNC_CLK) + string(" ns.");
						throw EventParsingException(events->second.at(0), tempErr);
					}

					if (endVal > startVal)
					{
						deltaWord = generateDDSfrequency( (endVal - startVal) / numberOfPoints );
					}
					else
					{
						deltaWord = generateDDSfrequency( (startVal - endVal) / numberOfPoints );
					}

					if (deltaWord == 0)
					{
						tempErr = string("The minimum sweep range is ") 
							+ valueToString( generateDDSfrequencyInMHz(1) * 1000000 ) + string(" Hz.");
						throw EventParsingException(events->second.at(0), tempErr);
					}
				//	else if (deltaWord > 4294967295)
				//	{
						// This should never happen.
				//		tempErr = string("The maximum step size during a sweep is ") 
				//			+ valueToString( sampleFreq ) + string(" MHz.");
				//		throw EventParsingException(events->second.at(0), tempErr);
				//	}

					dds_parameters.at(events->second.at(0).channel()).fallingSweepRampRate = RSSR;
					dds_parameters.at(events->second.at(0).channel()).fallingSweepRampRateInPercent = 100;

					if (endVal > startVal)
					{
						dds_parameters.at(events->second.at(0).channel()).risingDeltaWord  = deltaWord;
						dds_parameters.at(events->second.at(0).channel()).risingDeltaWordInMHz = generateDDSfrequencyInMHz( deltaWord );
					}
					else
					{
						dds_parameters.at(events->second.at(0).channel()).fallingDeltaWord  = deltaWord;
						dds_parameters.at(events->second.at(0).channel()).fallingDeltaWordInMHz = generateDDSfrequencyInMHz( deltaWord );
					}

					if(dds_parameters.at(events->second.at(0).channel()).FrequencyInMHz != startVal)
						eventTypeSize++;
				}
				//amplitude sweep
				if( events->second.at(i).ddsValue().ampl._d() == STI::Types::DDSSweep )
				{
					amplEvent = Sweep;
					eventTypeSize += sweepModeChangeSize;

					startVal = events->second.at(i).ddsValue().ampl.sweep().startVal;
					endVal   = events->second.at(i).ddsValue().ampl.sweep().endVal;
					rampTime = events->second.at(i).ddsValue().ampl.sweep().rampTime;

					if(dds_parameters.at(ActiveChannel).AmplitudeInPercent != startVal)
						eventTypeSize++;
				}
				//phase sweep
				if( events->second.at(i).ddsValue().phase._d() == STI::Types::DDSSweep )
				{
					phaseEvent = Sweep;
					eventTypeSize += sweepModeChangeSize;
											
					startVal = events->second.at(i).ddsValue().phase.sweep().startVal;
					endVal   = events->second.at(i).ddsValue().phase.sweep().endVal;
					rampTime = events->second.at(i).ddsValue().phase.sweep().rampTime;

					if(dds_parameters.at(ActiveChannel).PhaseInDegrees != startVal)
						eventTypeSize++;
				}

				// Make sure nothing else changes during a ramp
				if( (freqEvent  == Sweep && (amplEvent != None && phaseEvent != None)) ||
					(amplEvent  == Sweep && (freqEvent != None && phaseEvent != None)) ||
					(phaseEvent == Sweep && (amplEvent != None && freqEvent  != None))
					)
				{
					throw EventParsingException(events->second.at(i),
						"Only the sweeping parameter can change during a DDS sweep event. The other parameters must be ''.");
				}

				if(freqEvent == Sweep || amplEvent == Sweep || phaseEvent == Sweep)
				{
					if(rampTime < 0)
						throw EventParsingException(events->second.at(i),
						"The DDS sweep time must be positive.");

			//		dds_parameters.at(events->second.at(i).channel()).ClearSweep = false;
			//		dds_parameters.at(events->second.at(i).channel()).startSweep = true;		
				}
			}
			else if(events->second.at(i).type() == ValueString)
			{
				if(events->second.at(i).stringValue() == "Switch Mode")
					eventTypeSize = 4;
				else if(events->second.at(i).stringValue() == "Set Sweep Parameters")
					eventTypeSize = 4;
				else
					eventTypeSize = 1;
			}
			else
			{
				std::cerr << "The DDS does not support that data type." << std::endl;
				throw EventParsingException(events->second.at(i),
						"The DDS does not support that data type.");
			}


			holdoffTime = holdoffTime - eventSpacing * eventTypeSize;
		}

		holdoffTime = holdoffTime - eventSpacing * events->second.size(); //add in the time required to change channels
		if(events->second.at(0).channel() == ActiveChannel)
			holdoffTime = holdoffTime + eventSpacing; //shorten result if we only need i-1 channel changes

		//std::cerr << "The start time is: " << holdoffTime << std::endl;
		//std::cerr << "The event time is: " << eventTime << std::endl;


		if(holdoffTime < eventTime) //check to see if the start is at negative time
			{
				std::cerr << "There is not enough time allowed between events. Make sure there is at least 10 microseconds." << std::endl;
				throw EventParsingException(events->second.at(0),
					"There is not enough time allowed between events. Make sure at least 10 microseconds are allowed before the 1st event for initialization.");
			}
		else
			eventTime = holdoffTime;

		for(unsigned i = 0; i < events->second.size(); i++) //step through all channels at this time
		{
			//first set the channel
			if(events->second.at(i).channel() != ActiveChannel)
			{
				std::cerr << "Old Channel: " << ActiveChannel << std::endl;

				ActiveChannel = events->second.at(i).channel();

				std::cerr << "New Channel: " << ActiveChannel << std::endl;

				eventsOut.push_back( 
					generateDDScommand(eventTime, 0x00)
					);

				//std::cerr << "I changed my channel because it wasn't correct." << std::endl;

				eventTime = eventTime + eventSpacing; //set holdoffTime for next event
			}

			switch(events->second.at(i).type())
			{
				case ValueNumber:
					std::cerr << "The value of a DDS event must be a tuple (freq, ampl, phase)." << std::endl;
					throw EventParsingException(events->second.at(i),
						"The value of a DDS event must be a tuple (freq, ampl, phase).");
					break;
				case ValueString:
					
					if(events->second.at(i).stringValue() == "Switch Mode")
					{
						//update Channel Registers, FR1, CFR, and Amplitude
						IOUpdate = false;
						eventsOut.push_back( 
								generateDDScommand(eventTime, 0x00)
								); //update channel
						eventTime = eventTime + eventSpacing;
						
						eventsOut.push_back( 
								generateDDScommand(eventTime, 0x01)
								); //update function registers
						eventTime = eventTime + eventSpacing;
						
						eventsOut.push_back( 
								generateDDScommand(eventTime, 0x03)
								); // update channel function registers
						eventTime = eventTime + eventSpacing;
						
						IOUpdate = true;
						eventsOut.push_back( 
								generateDDScommand(eventTime, 0x06)
								); // update amplitude enable
						eventTime = eventTime + eventSpacing;
					}
					else if(events->second.at(i).stringValue() == "Set Sweep Parameters")
					{
						//update RSRR & FSRR, RDW, 
						IOUpdate = false;
						eventsOut.push_back( 
								generateDDScommand(eventTime, 0x07)
								); //update rising and falling ramp rates
						eventTime = eventTime + eventSpacing;
						
						eventsOut.push_back( 
								generateDDScommand(eventTime, 0x08)
								); //update rising delta word
						eventTime = eventTime + eventSpacing;
						
						eventsOut.push_back( 
								generateDDScommand(eventTime, 0x09)
								); //update falling delta word
						eventTime = eventTime + eventSpacing;
						
						IOUpdate = true;
						eventsOut.push_back( 
								generateDDScommand(eventTime, 0x0A)
								); //update end word
						eventTime = eventTime + eventSpacing;
					}
					else if(events->second.at(i).stringValue() == "Non Register Command")
					{
						IOUpdate = false;
						eventsOut.push_back( 
								generateDDScommand(eventTime, 0x0c)
								); //write to some random address, we don't care, just as long as an event gets written
						eventTime = eventTime + eventSpacing;
					}
					else
					{
						//must mean it's an address...
						successOutputAddr = stringToValue(events->second.at(i).stringValue(), outputAddr);
						if(successOutputAddr)
						{
							IOUpdate = true; //we probably want to update
							eventsOut.push_back( 
								generateDDScommand(eventTime, outputAddr)
								);
							eventTime = eventTime + eventSpacing;
						}
					}
					break;
				case ValueDDSTriplet:

					IOUpdate = false;

					//simple freq change event
					if(freqEvent == Change)
					{
						//only IOUpdate here if there is no ampl or phase event
						IOUpdate = !( amplEvent == Change || phaseEvent == Change);

						//set Frequency @ addr 0x04
						eventsOut.push_back( generateDDScommand(eventTime, 0x04) );
						eventTime = eventTime + eventSpacing;
					}
					if(amplEvent == Change)
					{
						//only IOUpdate here if there is no phase event
						IOUpdate = !( phaseEvent == Change);

						//set Amplitude @ addr 0x06
						eventsOut.push_back( generateDDScommand(eventTime, 0x06) );
						eventTime = eventTime + eventSpacing;

					}
					if(phaseEvent == Change)
					{
						IOUpdate = true;

						//set Phase @ addr 0x05
						eventsOut.push_back( generateDDScommand(eventTime, 0x05) );
						eventTime = eventTime + eventSpacing;
					}
						
					if(freqEvent == Sweep)
					{
						if(dds_parameters.at(ActiveChannel).FrequencyInMHz != startVal)
						{
							//add an event to make the frequency equal the correct start value
							//set Frequency @ addr 0x04
							eventsOut.push_back( generateDDScommand(eventTime, 0x04) );
							eventTime = eventTime + eventSpacing;
						}



						//Switch Mode

						dds_parameters.at(ActiveChannel).AFPSelect = 2;
						dds_parameters.at(ActiveChannel).LinearSweepEnable = true;
						dds_parameters.at(ActiveChannel).AmplitudeEnable = false;
						dds_parameters.at(ActiveChannel).mode = 2;



						//update Channel Registers, FR1, CFR, and Amplitude
						IOUpdate = false;
						eventsOut.push_back( 
								generateDDScommand(eventTime, 0x00)
								); //update channel
						eventTime = eventTime + eventSpacing;
						
						eventsOut.push_back( 
								generateDDScommand(eventTime, 0x01)
								); //update function registers
						eventTime = eventTime + eventSpacing;
						
						eventsOut.push_back( 
								generateDDScommand(eventTime, 0x03)
								); // update channel function registers
						eventTime = eventTime + eventSpacing;
						
						eventsOut.push_back( 
								generateDDScommand(eventTime, 0x06)
								); // update amplitude enable
						eventTime = eventTime + eventSpacing;







						//"Set Sweep Parameters"					
						//update RSRR & FSRR, RDW, 
						IOUpdate = false;
						eventsOut.push_back( 
							generateDDScommand(eventTime, 0x07)); //update rising and falling ramp rates
						eventTime = eventTime + eventSpacing;

						eventsOut.push_back( 
							generateDDScommand(eventTime, 0x08)); //update rising delta word
						eventTime = eventTime + eventSpacing;

						eventsOut.push_back( 
							generateDDScommand(eventTime, 0x09)); //update falling delta word
						eventTime = eventTime + eventSpacing;
		
						eventsOut.push_back( 
							generateDDScommand(eventTime, 0x0A)); //update end word
						eventTime = eventTime + eventSpacing;


						IOUpdate = true;
						dds_parameters.at(events->second.at(i).channel()).ClearSweep = false;
						
						eventsOut.push_back( 
							generateDDScommand(eventTime, 0x03)); //parameters...
						eventTime = eventTime + eventSpacing;


						dds_parameters.at(events->second.at(i).channel()).startSweep = true;		
												
						IOUpdate = false;
						eventsOut.push_back( 
								generateDDScommand(eventTime, 0x0c)
								); //write to some random address, we don't care, just as long as an event gets written
						eventTime = eventTime + eventSpacing;


					}



					////OLD
					//dds_parameters.at(ActiveChannel).Frequency = generateDDSfrequency(events->second.at(i).ddsValue().freq);
					//dds_parameters.at(ActiveChannel).Phase = generateDDSphase(events->second.at(i).ddsValue().phase);
					//dds_parameters.at(ActiveChannel).Amplitude = generateDDSamplitude(events->second.at(i).ddsValue().ampl);
					//
					////set Amplitude @ addr 0x06
					//IOUpdate = false;
					//eventsOut.push_back( 
					//		generateDDScommand(eventTime, 0x06)
					//		);
					//eventTime = eventTime + eventSpacing;
					//	
					////set Frequency @ addr 0x04
					//eventsOut.push_back( 
					//		generateDDScommand(eventTime, 0x04)
					//		);
					//eventTime = eventTime + eventSpacing;

					////set Phase @ addr 0x05
					//IOUpdate = true;
					//eventsOut.push_back( 
					//		generateDDScommand(eventTime, 0x05)
					//		);

					//std::cerr << "I created an event using a dds triplet. I set all 3 values, ampl, freq, phase." << std::endl;
					break;
				case ValueMeas:
					std::cerr << "The DDS does not support ValueMeas events." << std::endl;
					throw EventParsingException(events->second.at(i),
						"The DDS does not support ValueMeas events.");
					break;
				default:
					std::cerr << "The DDS does not support whatever you tried to give it." << std::endl;
					throw EventParsingException(events->second.at(i),
						"The DDS does not support whatever you tried to give it.");
					break;
			}
		}
	}
	
	notInitialized = false;
	//std::cerr << "The DDS parsed." << std::endl;

}
uInt32 STF_DDS_Device::generateDDSphase(double doublePhase)
{
	uInt32 hexPhase = 0;
	hexPhase = static_cast<uInt32>(floor((doublePhase / 360.0) * 16383.0));
	return hexPhase;
}
uInt32 STF_DDS_Device::generateDDSfrequency(double doubleFrequency)
{
	uInt32 hexFrequency = 0;
	if(ExternalClock)
		hexFrequency = static_cast<uInt32>(floor((doubleFrequency / sampleFreq) * 2147483647.0 * 2));
	else
		hexFrequency = static_cast<uInt32>(floor((doubleFrequency / sampleFreq) * 2147483647.0 * 2));
	
	return hexFrequency;
}

double STF_DDS_Device::generateDDSfrequencyInMHz(uInt32 hexFrequency)
{
	double doubleFrequency = 0;
	if(ExternalClock)
		doubleFrequency = static_cast<double>((hexFrequency * sampleFreq) / 2147483647.0 * 2);
	else
		doubleFrequency = static_cast<double>((hexFrequency * sampleFreq) / 2147483647.0 * 2);
	
	return doubleFrequency;
}

uInt32 STF_DDS_Device::generateDDSamplitude(double doubleAmplitude)
{
	uInt32 hexAmplitude = 0;
	hexAmplitude = static_cast<uInt32>(floor((doubleAmplitude / 100.0) * 1023.0)); //in percent
	return hexAmplitude;
}
STF_DDS_Device::DDS_Event* STF_DDS_Device::generateDDScommand(double time, uInt32 addr)
{
	
	//DDSParameters& ddsCh = dds_parameters.at(ActiveChannel); //a shorthand way to use a reference

	DDS_Event* ddsCommand = new DDS_Event(time, 0, 0, this);
	ddsCommand->setBits(0);
	ddsCommand->setBits(addr, 32, 36);	//5 bit address
	ddsCommand->setBits(!IOUpdate, 48, 48); //if we do not want an IO Update, set this bit high
	ddsCommand->setBits(ExternalClock, 40, 40);
	
	ddsCommand->setBits(dds_parameters.at(ActiveChannel).startSweep, 41 + ActiveChannel, 41 + ActiveChannel); //selects which channel to sweep based on active channel


	if (addr == 0x00)	//set active channel
	{
		ddsCommand->setBits(1, 45, 47);		//3 bit length (number of bytes in command)
		ddsCommand->setBits(true, 28 + ActiveChannel, 28 + ActiveChannel);
	}
	else if (addr == 0x01)
	{
		ddsCommand->setBits(3, 45, 47);		//3 bit length (number of bytes in command)
		ddsCommand->setBits(VCOEnable, 31, 31); //turn on VCO control
		ddsCommand->setBits(PLLmultiplier, 26, 30); // set PLLmultiplier value (allowed 4-20)
		ddsCommand->setBits(ModulationLevel, 16, 17);
	}
	else if (addr == 0x02)	//???
	{
		ddsCommand->setBits(2, 45, 47);		//3 bit length (number of bytes in command)	
	}
	else if (addr == 0x03)	//parameters...
	{
		ddsCommand->setBits(3, 45, 47);		//3 bit length (number of bytes in command)
		ddsCommand->setBits(dds_parameters.at(ActiveChannel).AFPSelect, 30, 31);
		ddsCommand->setBits(dds_parameters.at(ActiveChannel).LSnoDwell, 23, 23);
		ddsCommand->setBits(dds_parameters.at(ActiveChannel).LinearSweepEnable, 22, 22);
		ddsCommand->setBits(dds_parameters.at(ActiveChannel).LoadSRR, 21, 21);
		ddsCommand->setBits(0, 18, 18); //Must be 0
		ddsCommand->setBits(dds_parameters.at(ActiveChannel).DACCurrentControl, 16, 17); //DAC full scale current control - set to default value of 0x03
		ddsCommand->setBits(dds_parameters.at(ActiveChannel).AutoclearSweep, 12, 12);
		ddsCommand->setBits(dds_parameters.at(ActiveChannel).ClearSweep, 11, 11);
		ddsCommand->setBits(dds_parameters.at(ActiveChannel).AutoclearPhase, 10, 10);
		ddsCommand->setBits(dds_parameters.at(ActiveChannel).ClearPhase, 9, 9);
		ddsCommand->setBits(dds_parameters.at(ActiveChannel).SinCos, 8, 8);		
	}
	else if (addr == 0x04)	//Set frequency
	{
		ddsCommand->setBits(4, 45, 47);		//3 bit length (number of bytes in command)
		ddsCommand->setBits(dds_parameters.at(ActiveChannel).Frequency, 0, 31);
	}
	else if (addr == 0x05)	//Set phase
	{
		ddsCommand->setBits(2, 45, 47);		//3 bit length (number of bytes in command)
		ddsCommand->setBits(dds_parameters.at(ActiveChannel).Phase, 16, 31);
	}
	else if (addr == 0x06)	//Set amplitude
	{
		ddsCommand->setBits(3, 45, 47);		//3 bit length (number of bytes in command)
		ddsCommand->setBits(dds_parameters.at(ActiveChannel).AmplitudeEnable, 20, 20);
		ddsCommand->setBits(dds_parameters.at(ActiveChannel).Amplitude, 8, 17);
		ddsCommand->setBits(dds_parameters.at(ActiveChannel).LoadARR, 18, 18);
		ddsCommand->setBits(dds_parameters.at(ActiveChannel).RuRd, 19, 19);
	}
	else if (addr == 0x07)
	{
		ddsCommand->setBits(2, 45, 47);		//3 bit length (number of bytes in command)
		ddsCommand->setBits(dds_parameters.at(ActiveChannel).fallingSweepRampRate, 24, 31); //RSRR has 8 bit resolution
		ddsCommand->setBits(dds_parameters.at(ActiveChannel).risingSweepRampRate, 16, 23); //FSRR has 8 bit resolution
	}
	else if (addr == 0x08)
	{
		ddsCommand->setBits(4, 45, 47);		//3 bit length (number of bytes in command)
		ddsCommand->setBits(dds_parameters.at(ActiveChannel).risingDeltaWord, 0, 31); //Frequency has 32 bit resolution
	}
	else if (addr == 0x09)
	{
		ddsCommand->setBits(4, 45, 47);		//3 bit length (number of bytes in command)
		ddsCommand->setBits(dds_parameters.at(ActiveChannel).fallingDeltaWord, 0, 31); //Frequency has 32 bit resolution
	}
	else if (addr == 0x0a)
	{
		ddsCommand->setBits(4, 45, 47);		//3 bit length (number of bytes in command)
		ddsCommand->setBits(dds_parameters.at(ActiveChannel).sweepEndPoint, 0, 31); //Frequency has 32 bit resolution
	}
	else
	{
		ddsCommand->setBits(4, 45, 47);		//3 bit length (number of bytes in command)
	}

	std::cerr << "ddsCommand: " ;
	
	for(int i = 63; i >= 0; i--)
	{
		std::cerr << ( ddsCommand->getBits(i,i) ? "1" : "0" );
	}
	std::cerr << std::endl;

	return ddsCommand;

}


STF_DDS_Device::DDS_Event::DDS_Event(double time, uInt32 command, uInt32 value, FPGA_Device* device) : 
BitLineEvent<64>(time, device), device_f(device)
{
	setBits(command, 32, 63);
	setBits(value, 0, 31);
}

void STF_DDS_Device::DDS_Event::setupEvent()
{
	time32 = static_cast<uInt32>( getTime() / 10 );	//in clock cycles! (1 cycle = 10 ns)
	timeAddress  = device_f->ramBlock.getWrappedAddress( 3*getEventNumber() );
	commandAddress = device_f->ramBlock.getWrappedAddress( 3*getEventNumber() + 1 );
	valueAddress = device_f->ramBlock.getWrappedAddress( 3*getEventNumber() + 2 );
}

void STF_DDS_Device::DDS_Event::loadEvent()
{
	//write the event to RAM
	device_f->ramBus->writeDataToAddress( time32, timeAddress );
	device_f->ramBus->writeDataToAddress( getBits(32, 63), commandAddress );
	device_f->ramBus->writeDataToAddress( getBits(0, 31), valueAddress );
}
STF_DDS_Device::DDS_Parameters::DDS_Parameters()
{
	//define initial values	
	mode = 0;
	ChargePumpControl = 0; // higher values increase the charge pump current
	ProfilePinConfig = 0; // Determines how the profile pins are configured
	RuRd = 0; // Ramp Up / Ramp Down control
	AFPSelect = 0;
	LSnoDwell = false;
	LinearSweepEnable = false;
	LoadSRR = false;
	AutoclearSweep = false;
	ClearSweep = false;
	AutoclearPhase = false;
	ClearPhase = false;
	SinCos = false;
	DACCurrentControl = 3;
	Phase = 0;
	PhaseInDegrees = 0;
	Frequency = 0;
	FrequencyInMHz = 10;
	Amplitude = 0;
	AmplitudeInPercent = 100;
	AmplitudeEnable = 0; // Default setting on DDS chip start-up
	LoadARR = false;
	risingDeltaWord = 0;
	risingDeltaWordInMHz = 0;
	fallingDeltaWord = 0;
	fallingDeltaWordInMHz = 0;
	sweepEndPoint = 0;
	sweepEndPointInMHz = 0;
	risingSweepRampRate = 0;
	risingSweepRampRateInPercent = 0;
	fallingSweepRampRate = 0;
	fallingSweepRampRateInPercent = 0;
	startSweep = false;
}
