/*! \file PartnerDevice.cpp
 *  \author Jason Michael Hogan
 *  \brief Source-file for the class PartnerDevice
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
#include "device.h"
#include <CommandLine_i.h>
#include <PartnerDevice.h>

#include <iostream>
#include <string>
using std::cerr;
using std::endl;
using std::string;

PartnerDevice::PartnerDevice()
{
	registered = false;
	partnerName = "";
}

PartnerDevice::PartnerDevice(std::string PartnerName, STI_Server_Device::CommandLine_ptr commandLine) :
partnerName(PartnerName)
{
	registered = false;
	setCommandLine(commandLine);
	partnerDeviceID = commandLine->deviceID();
}

PartnerDevice::~PartnerDevice()
{
}


string PartnerDevice::name() const
{
	return partnerName;
}

string PartnerDevice::deviceID() const
{
	return partnerDeviceID;
}
bool PartnerDevice::isRegistered() const
{
	return registered;
}
bool PartnerDevice::isAlive()
{
	if( isRegistered() )
	{
		try {
			commandLine_l->deviceID();	//try to contact partner
			return true;
		}
		catch(CORBA::TRANSIENT& ex) {
			cerr << "Caught system exception CORBA::" << ex._name() 
				<< " when trying to contact partner device " << name() 
				<< "." << endl;
		}
		catch(CORBA::SystemException& ex) {
			cerr << "Caught a CORBA::" << ex._name()
			<< " when trying to contact partner device " << name() 
			<< "." << endl;
		}
	}
	return false;
}

void PartnerDevice::setCommandLine(STI_Server_Device::CommandLine_ptr commandLine)
{
	commandLine_l = commandLine;
	registered = true;
}

string PartnerDevice::execute(string args)
{
	string result = "";

	if(!registered)		//this partner has not been registered by the server
		return result;

	try {

cerr << "PartnerDevice::execute" << endl;

		result = commandLine_l->execute(args.c_str());
	}
	catch(CORBA::TRANSIENT& ex) {
		cerr << "Caught system exception CORBA::" << ex._name() 
			<< " when trying to execute a partner device command: " 
			<< endl << "--> partner(\"" << name() << "\").execute(" 
			<< args << ")" << endl;
	}
	catch(CORBA::SystemException& ex) {
		cerr << "Caught a CORBA::" << ex._name()
			<< " when trying to execute a partner device command: " 
			<< endl << "--> partner(\"" << name() << "\").execute(" 
			<< args << ")" << endl;
	}

	return result;
}

bool PartnerDevice::setAttribute(std::string key, std::string value)
{
	if( !registered )		//this partner has not been registered by the server
		return false;
	
	try {
		return commandLine_l->setAttribute( key.c_str(), value.c_str() );
	}
	catch(CORBA::TRANSIENT& ex) {
		cerr << "Caught system exception CORBA::" << ex._name() 
			<< " when trying to execute a partner device command: " 
			<< endl << "--> partner(\"" << name() << "\").setAttribute(" 
			<< key << ", " << value << ")" << endl;
	}
	catch(CORBA::SystemException& ex) {
		cerr << "Caught a CORBA::" << ex._name()
			<< " when trying to execute a partner device command: " 
			<< endl << "--> partner(\"" << name() << "\").setAttribute(" 
			<< key << ", " << value << ")" << endl;
	}

	return false;
}

std::string PartnerDevice::getAttribute(std::string key)
{
	string value = "";

	if(!registered)		//this partner has not been registered by the server
		return value;
	
	try {
		value = commandLine_l->getAttribute( key.c_str() );
	}
	catch(CORBA::TRANSIENT& ex) {
		cerr << "Caught system exception CORBA::" << ex._name() 
			<< " when trying to execute a partner device command: " 
			<< endl << "--> partner(\"" << name() << "\").getAttribute(" 
			<< key << ")" << endl;
	}
	catch(CORBA::SystemException& ex) {
		cerr << "Caught a CORBA::" << ex._name()
			<< " when trying to execute a partner device command: " 
			<< endl << "--> partner(\"" << name() << "\").getAttribute(" 
			<< key << ")" << endl;
	}

	return value;
}


