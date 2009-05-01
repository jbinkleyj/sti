/*! \file lockDevice.cpp
 *  \author Jason Michael Hogan
 *  \brief Source-file for the class lockDevice
 *  \section license License
 *
 *  Copyright (C) 2008 Jason Hogan <hogan@stanford.edu>\n
 *  This file is part of the Stanford Timing Interface (STI).
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



#include "lockDevice.h"
#include "STI_Device.h"
#include <types.h>

#include <sstream>
#include <string>
#include <map>
using std::string;
using std::map;

#include <iostream>
using namespace std;


lockDevice::lockDevice(ORBManager*    orb_manager, 
					   std::string    DeviceName, 
					   std::string    Address, 
					   unsigned short ModuleNumber) : 
STI_Device(orb_manager, DeviceName, Address, ModuleNumber)
{

#ifndef _MSC_VER
	currentSettingFileBase = "/var/log/lock";
#endif
#ifdef _MSC_VER
	currentSettingFileBase = ".\\lock";
#endif

	//Initialize the Cs Lock board
	lockBoard = new CsLock("CsLock Board", 0);		//init address 0
	
	lockBoard->whichCircuit = 0;

	currentSettingFile = constructSettingFileName(lockBoard->address());

	//read in current setting from file; also writes these to the board
	getCurrentSettings(currentSettingFile);
}

lockDevice::~lockDevice()
{
}


void lockDevice::defineAttributes() 
{
	addAttribute("Serial address (0 - 15)", lockBoard->address());
	addAttribute("Circuit # (0 or 1)",      lockBoard->whichCircuit, "0, 1");

	addAttribute("P   [0,1]",     lockBoard->getPropGain() );
	addAttribute("I   [0,1]",     lockBoard->getInt1Gain() );
	addAttribute("D   [0,1]",     lockBoard->getDiffGain() );
	addAttribute("I^2 [0,1]",     lockBoard->getInt2Gain() );
	addAttribute("Enable",        (lockBoard->getOutputEnable() ? "On" : "Off"), "Off, On");
	addAttribute("Enable I",      (lockBoard->getInt1Enable() ? "On" : "Off"), "Off, On");
	addAttribute("Enable I^2",    (lockBoard->getInt2Enable() ? "On" : "Off"), "Off, On");
	addAttribute("Offset [-1,1]", lockBoard->getOffset() );
}

void lockDevice::refreshAttributes() 
{
//	setAttribute("Serial address (0 - 15)", lockBoard->address());
	setAttribute("Circuit # (0 or 1)",      lockBoard->whichCircuit);
	setAttribute("P   [0,1]",     lockBoard->getPropGain() );
	setAttribute("I   [0,1]",     lockBoard->getInt1Gain() );
	setAttribute("D   [0,1]",     lockBoard->getDiffGain() );
	setAttribute("I^2 [0,1]",     lockBoard->getInt2Gain() );
	setAttribute("Enable",        (lockBoard->getOutputEnable() ? "On" : "Off") );
	setAttribute("Enable I",      (lockBoard->getInt1Enable() ? "On" : "Off") );
	setAttribute("Enable I^2",    (lockBoard->getInt2Enable() ? "On" : "Off") );
	setAttribute("Offset [-1,1]", lockBoard->getOffset() );



	//Synchronize the Device Attributes with the current setting file
	currentSettingFile = constructSettingFileName(lockBoard->address());
	printCurrentSettingsToFile(currentSettingFile);
	showTextMenu();
}

bool lockDevice::updateAttribute(string key, string value)
{
	double tempDouble;
	int tempInt;

	bool successDouble = stringToValue(value, tempDouble);
	bool successInt = stringToValue(value, tempInt);

	bool success = successDouble || successInt;

	if(success)
	{
		if(key.compare("Serial address (0 - 15)") == 0)
		{
			if(tempInt >= 0 && tempInt <= 15)
			{
				lockBoard->setAddress(tempInt);
				currentSettingFile = constructSettingFileName(lockBoard->address());

				//read in current setting from file; also writes these to the board
				getCurrentSettings(currentSettingFile);
			}
			else
			{
				success = false;
			}
		}

		if(key.compare("Circuit # (0 or 1)") == 0)
			lockBoard->whichCircuit = tempInt;

		if(key.compare("P   [0,1]") == 0)
			lockBoard->setPropGain(tempDouble);
		else if(key.compare("I   [0,1]") == 0)
			lockBoard->setInt1Gain(tempDouble);
		else if(key.compare("D   [0,1]") == 0)
			lockBoard->setDiffGain(tempDouble);
		else if(key.compare("I^2 [0,1]") == 0)
			lockBoard->setInt2Gain(tempDouble);
		else if(key.compare("Offset [-1,1]") == 0)
			lockBoard->setOffset(tempDouble);
	}

	if(key.compare("Enable") == 0)
	{
		lockBoard->setOutputEnable( value.compare("On") == 0 );
		success = true;
	}
	if(key.compare("Enable I") == 0)
	{
		lockBoard->setInt1Enable( value.compare("On") == 0 );
		success = true;
	}
	if(key.compare("Enable I^2") == 0)
	{
		lockBoard->setInt2Enable( value.compare("On") == 0 );
		success = true;
	}

	return success;
}

void lockDevice::defineChannels()
{
}

bool lockDevice::writeChannel(const RawEvent& Event)
{
	return false;
}

bool lockDevice::readChannel(ParsedMeasurement& Measurement)
{
	return false;
}

void lockDevice::parseDeviceEvents(const RawEventMap& eventsIn, 
        SynchronousEventVector& eventsOut) throw(std::exception)
{
	
}
void lockDevice::definePartnerDevices()
{
}

void lockDevice::stopEventPlayback()
{
}

std::string lockDevice::execute(int argc, char **argv)
{
	return parseCommandLineArgs(argc, argv);
}

bool lockDevice::deviceMain(int argc, char **argv)
//int main(int argc, char* argv[])
{

//	int argc = 1;
//	char** argv = new char*[1];


	int serialAddress, circuitNum, selection;
	string dummy;
	bool showMenu;
	double tempGain;


	//parse command line arguments; if none, then show menu
	cout << parseCommandLineArgs(argc, argv);

	showMenu = (argc < 2);

	// Menu Interface
	while(showMenu)
	{
		serialAddress = -1;
		while(serialAddress < 0 || serialAddress > 15)
		{
			cout << "Serial address (0 - 15): ";
			cin >> serialAddress;
			if (cin.fail())
			{
				cin.clear();
				cin >> dummy;
			}
			else if(serialAddress >= 0 && serialAddress <= 15)
			{
				lockBoard->setAddress(serialAddress);
				currentSettingFile = constructSettingFileName(lockBoard->address());
				//read in current setting from file; also writes these to the board
				getCurrentSettings(currentSettingFile);
			}
		}

		cout << "Circuit # (0 or 1): ";
		cin >> circuitNum;
		selection = 0;
		
		if (cin.fail())
		{
			cin.clear();
			cin >> dummy;
		}

		switch(circuitNum)
		{
		case 0:
		case 1:
			lockBoard->whichCircuit = circuitNum;
			break;
		case -1:
			showMenu = false;
			selection = 9;			
		default:
			selection = 9;
			break;
		}

		while(selection != 9)
		{
			refreshAttributes();

			cin >> selection;

			if (cin.fail())
			{
				selection = 0;
				cin.clear();
				cin >> dummy;
			}

			switch(selection)
			{
			case 1:
				cout << "P = ";
				cin >> tempGain;
				lockBoard->setPropGain(tempGain);
				break;
			case 2:
				cout << "I = ";
				cin >> tempGain;
				lockBoard->setInt1Gain(tempGain);
				break;
			case 3:
				cout << "D = ";
				cin >> tempGain;
				lockBoard->setDiffGain(tempGain);
				break;
			case 4:
				cout << "I^2 = ";
				cin >> tempGain;
				lockBoard->setInt2Gain(tempGain);
				break;
			case 5:
				lockBoard->setOutputEnable(!lockBoard->getOutputEnable());
				break;
			case 6:
				lockBoard->setInt1Enable(!lockBoard->getInt1Enable());
				break;
			case 7:
				lockBoard->setInt2Enable(!lockBoard->getInt2Enable());
				break;
			case 8:
				cout << "Offset = ";
				cin >> tempGain;
				lockBoard->setOffset(tempGain);
				break;
			default:
				break;
			}
		}
	}

	refreshAttributes();
	
	return false;
}



void lockDevice::showTextMenu()
{
	cout << endl 
		<< "-------------------" << endl
		<< "Serial Address: " << lockBoard->address() << endl
		<< "Circuit # " << lockBoard->whichCircuit << endl;
	cout << "(1) P   [0,1]     : " << lockBoard->getPropGain() << endl;
	cout << "(2) I   [0,1]     : " << lockBoard->getInt1Gain() << endl;
	cout << "(3) D   [0,1]     : " << lockBoard->getDiffGain() << endl;
	cout << "(4) I^2 [0,1]     : " << lockBoard->getInt2Gain() << endl;
	cout << "(5) Enable        : " << (lockBoard->getOutputEnable() ? "On" : "Off") << endl;
	cout << "(6) Enable I      : " << (lockBoard->getInt1Enable() ? "On" : "Off") << endl;
	cout << "(7) Enable I^2    : " << (lockBoard->getInt2Enable() ? "On" : "Off") << endl;
	cout << "(8) Offset [-1,1] : " << lockBoard->getOffset() << endl;
	cout << "(9) Back" << endl;
}


void lockDevice::printUsage(void)
{
	terminalStream 	<< "Usage: " << endl 
		<< "lock                        <Menu interface>" << endl
		<< "lock --help | -h            <Print detailed usage>" << endl;
	terminalStream 	<< "lock [filename]             <Load from file>" << endl << endl;

	terminalStream	<< "<Command line syntax>" << endl;
	terminalStream 	<< "lock --addr=serAddress" << endl
		<< "     [--p1=p1Gain]     [--p2=p2Gain]     [--i1=i1Gain]       [--i2=i2Gain]"   << endl
		<< "     [--d1=d1Gain]     [--d2=d2Gain]     [--ii1=ii1Gain]     [--ii2=ii2Gain]" << endl
		<< "     [--o1=offset1]    [--o2=offset2]    [--e1=enable_1]     [--e2=enable_2]" << endl
		<< "     [--ei1=enable_i1] [--ei2=enable_i2] [--eii1=enable_ii1] [--eii2=enable_ii2]" << endl << endl;
}

void lockDevice::getCurrentSettings(string settingsFile)
{
	// For now, output the values in settingsFile to the board
	// In the future, it should be possible to get the setting without
	// writting to the board.

	cout << parseCommandLineArgs( getStringArgsFromFile(settingsFile) );
}

void lockDevice::printCurrentSettingsToFile(const string &fileName)
{
	ofstream file(fileName.c_str(), ios::out);
	string arg;

	if(!file)	//Check for valid file stream
	{
		terminalStream << "Error: Failed to open current settings file: " 
			<< fileName.c_str() << endl;
	}
	else
	{
		file 	<< "--addr="   << lockBoard->address() << " ";

		file 	<< "--p1="   << lockBoard->getPropGain(CsLock::LaserTypeF3) << " "
			<< "--i1="   << lockBoard->getInt1Gain(CsLock::LaserTypeF3) << " "
			<< "--d1="   << lockBoard->getDiffGain(CsLock::LaserTypeF3) << " "
			<< "--ii1="  << lockBoard->getInt2Gain(CsLock::LaserTypeF3) << " "
			<< "--o1="   << lockBoard->getOffset(CsLock::LaserTypeF3) << " "
			<< "--e1="   << lockBoard->getOutputEnable(CsLock::LaserTypeF3) << " "
			<< "--ei1="  << lockBoard->getInt1Enable(CsLock::LaserTypeF3) << " "
			<< "--eii1=" << lockBoard->getInt2Enable(CsLock::LaserTypeF3) << " ";

		file 	<< "--p2="   << lockBoard->getPropGain(CsLock::LaserTypeF4) << " "
			<< "--i2="   << lockBoard->getInt1Gain(CsLock::LaserTypeF4) << " "
			<< "--d2="   << lockBoard->getDiffGain(CsLock::LaserTypeF4) << " "
			<< "--ii2="  << lockBoard->getInt2Gain(CsLock::LaserTypeF4) << " "
			<< "--o2="   << lockBoard->getOffset(CsLock::LaserTypeF4) << " "
			<< "--e2="   << lockBoard->getOutputEnable(CsLock::LaserTypeF4) << " "
			<< "--ei2="  << lockBoard->getInt1Enable(CsLock::LaserTypeF4) << " "
			<< "--eii2=" << lockBoard->getInt2Enable(CsLock::LaserTypeF4) << endl;
	}
}

lockDevice::commandArg lockDevice::parseArg(string arg)
{
	int equalsLoc;
	string argString;
	double argValue;

	commandArg cmdArg;
	cmdArg.circuit = 0;	//default

	if(arg.at(0) != '-')	//Assume it's filename
	{
		cmdArg.argType = filename;
	}
	else if(arg.length() < 2) //invalid
	{
		cmdArg.argType = invalid;
	}
	else if(arg.compare("-h")==0 || arg.compare("--help")==0)
	{
		cmdArg.argType = help;
	}
	else
	{
		equalsLoc = arg.find("=", 0);
		argString = arg.substr(0, equalsLoc);
		argValue = atof(arg.substr(equalsLoc + 1).c_str());
		
		// Any other non-numeric value results in argValue=0
		if(arg.substr(equalsLoc + 1).compare("true")==0 || 
			arg.substr(equalsLoc + 1).compare("True")==0 ||
			arg.substr(equalsLoc + 1).compare("TRUE")==0)
		{
			argValue = 1;
		}
/*
		if(argValue==0 && arg.substr(equalsLoc + 1,1).compare("0")!=0)
		{
			cmdArg.argType = invalid;
			cerr << "Error: Gain value must be a floating point number." << endl;
		}
*/
		if(argString.compare("--addr")==0)
		{
			cmdArg.argType = serialAddress;
			cmdArg.value = argValue;

			if(argValue < 0 || argValue > 15)
			{
				terminalStream << "Serial address must be in the range 0-15." << endl << endl;
				cmdArg.argType = invalid;
			}
		}
		else if(argString.compare("--p1")==0)
		{
			cmdArg.circuit = 0;
			cmdArg.argType = pGain;
			cmdArg.value = argValue;
		}
		else if(argString.compare("--p2")==0)
		{
			cmdArg.circuit = 1;
			cmdArg.argType = pGain;
			cmdArg.value = argValue;
		}
		else if(argString.compare("--i1")==0)
		{
			cmdArg.circuit = 0;
			cmdArg.argType = iGain;
			cmdArg.value = argValue;
		}
		else if(argString.compare("--i2")==0)
		{
			cmdArg.circuit = 1;
			cmdArg.argType = iGain;
			cmdArg.value = argValue;
		}
		else if(argString.compare("--d1")==0)
		{
			cmdArg.circuit = 0;
			cmdArg.argType = dGain;
			cmdArg.value = argValue;
		}
		else if(argString.compare("--d2")==0)
		{
			cmdArg.circuit = 1;
			cmdArg.argType = dGain;
			cmdArg.value = argValue;
		}
		else if(argString.compare("--ii1")==0)
		{
			cmdArg.circuit = 0;
			cmdArg.argType = iiGain;
			cmdArg.value = argValue;
		}
		else if(argString.compare("--ii2")==0)
		{
			cmdArg.circuit = 1;
			cmdArg.argType = iiGain;
			cmdArg.value = argValue;
		}
		else if(argString.compare("--o1")==0)
		{
			cmdArg.circuit = 0;
			cmdArg.argType = offset;
			cmdArg.value = argValue;
		}
		else if(argString.compare("--o2")==0)
		{
			cmdArg.circuit = 1;
			cmdArg.argType = offset;
			cmdArg.value = argValue;
		}
		else if(argString.compare("--e1")==0)
		{
			cmdArg.circuit = 0;
			cmdArg.argType = oEnable;
			cmdArg.value = argValue;
		}
		else if(argString.compare("--e2")==0)
		{
			cmdArg.circuit = 1;
			cmdArg.argType = oEnable;
			cmdArg.value = argValue;
		}
		else if(argString.compare("--ei1")==0)
		{
			cmdArg.circuit = 0;
			cmdArg.argType = iEnable;
			cmdArg.value = argValue;
		}
		else if(argString.compare("--ei2")==0)
		{
			cmdArg.circuit = 1;
			cmdArg.argType = iEnable;
			cmdArg.value = argValue;
		}
		else if(argString.compare("--eii1")==0)
		{
			cmdArg.circuit = 0;
			cmdArg.argType = iiEnable;
			cmdArg.value = argValue;
		}
		else if(argString.compare("--eii2")==0)
		{
			cmdArg.circuit = 1;
			cmdArg.argType = iiEnable;
			cmdArg.value = argValue;
		}
		else
		{
			cmdArg.argType = invalid;
		}	
	}

	return cmdArg;
}

vector<string> lockDevice::getStringArgsFromFile(const string &fileName)
{
	vector<string> argVector;

	ifstream file(fileName.c_str(), ios::in);
	string arg;

	if(!file)	//Check for valid file stream
	{
		cerr << "Error: Failed to open file " 
			<< fileName.c_str() << "." << endl;
	}
	else
	{
		while(getline(file, arg, ' '))	//gets strings separated by spaces
		{
			argVector.push_back(arg);
		}
	}

	return argVector;
}

std::string lockDevice::parseCommandLineArgs(vector<string> strArgv)
{
	int i;

	// argv is the list of arguments found in the file plus the prepended program name
	int argc = strArgv.size() + 1;
	char** argv = new char*[argc];
	
	argv[0] = new char[5];
	strcpy(argv[0], "lock");	//a dummy name; argv[0] is always the name of the program

	for(i=1; i < argc; i++)
	{
		argv[i] = new char[strArgv[i-1].size() + 1];	//including NULL char
		strcpy(argv[i], strArgv[i-1].c_str());
	}

	string result = parseCommandLineArgs(argc, argv);
	
	// clean up
	for(i = 0; i < argc; i++)
		delete[] argv[i];
	delete[] argv;

	return result;
}	


std::string lockDevice::parseCommandLineArgs(int argc, char* argv[])
{
	terminalStream.str("");

	int i;
	string tempArg;
	commandArg parsedArg;

	bool invalidArg = false;
	bool noArgs = true;

	for(i=1; i < argc; i++)
	{
		noArgs = false;
		tempArg.assign(argv[i]);
		parsedArg = parseArg(tempArg);

		lockBoard->whichCircuit = parsedArg.circuit;

		//first argument must be the serial address
		if(i == 1 && parsedArg.argType != serialAddress)
		{
			parsedArg.argType = invalid;
			terminalStream << "Error: The first argument must be --addr=serAddress." << endl;
		}
		if(i != 1 && parsedArg.argType == serialAddress)
		{
			parsedArg.argType = invalid;
			terminalStream << "Error: --addr=serAddress can only be the first argument." << endl;
		}

		switch(parsedArg.argType)
		{
		case pGain:
			lockBoard->setPropGain(parsedArg.value);
			break;
		case iGain:
			lockBoard->setInt1Gain(parsedArg.value);
			break;
		case dGain:
			lockBoard->setDiffGain(parsedArg.value);
			break;
		case iiGain:
			lockBoard->setInt2Gain(parsedArg.value);
			break;
		case offset:
			lockBoard->setOffset(parsedArg.value);
			break;
		case oEnable:
			lockBoard->setOutputEnable(parsedArg.value==1 ? true : false);
			break;
		case iEnable:
			lockBoard->setInt1Enable(parsedArg.value==1 ? true : false);
			break;
		case iiEnable:
			lockBoard->setInt2Enable(parsedArg.value==1 ? true : false);
			break;
		case help:
			printUsage();
			break;
		case serialAddress:
			lockBoard->setAddress(static_cast<int>(parsedArg.value));
			currentSettingFile = constructSettingFileName(lockBoard->address());
			break;
		case filename:
			terminalStream << "Loading from file " << tempArg << endl;
			terminalStream << parseCommandLineArgs( getStringArgsFromFile(tempArg) );
			break;
		case invalid:
		default:
			invalidArg = true;
			break;
		}

	}
	if(invalidArg)
	{
		terminalStream << "Error: Invalid argument" << endl << endl;
		printUsage();
	}
		
	return terminalStream.str();
}

string lockDevice::constructSettingFileName(int serAddress)
{
	// Use the serial address to construct the filename
	ostringstream serAddressStr;

	serAddressStr << currentSettingFileBase << "." << serAddress;

	return serAddressStr.str();
}

