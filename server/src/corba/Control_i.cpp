/*! \file Control_i.cpp
 *  \author Jason Michael Hogan
 *  \brief Source-file for the class Control_i
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
#include <cassert>

#include "Control_i.h"
#include "STI_Server.h"
#include <ExperimentDocumenter.h>
#include <SequenceDocumenter.h>

Control_i::Control_i(STI_Server* server) : sti_Server(server)
{
	modeHandler = NULL;
	expSequence = NULL;
	parser = NULL;
}


Control_i::~Control_i()
{
}



void Control_i::add_Parser(Parser_i* var)
{
	assert(var != NULL);

	if(parser != NULL)
	{
		// Remove reference to the current ModeHandler_i servant, allowing
		// for the possibility that var is a new instance of the servant.
		parser->_remove_ref();
	}

	parser = var;
	parser->_add_ref();
}

void Control_i::remove_Parser()
{
	if(parser != NULL)
	{
		parser->_remove_ref();
	}

	parser = NULL;
}

void Control_i::add_ExpSequence(ExpSequence_i* var)
{
	assert(var != NULL);

	if(expSequence != NULL)
	{
		// Remove reference to the current ModeHandler_i servant, allowing
		// for the possibility that var is a new instance of the servant.
		parser->_remove_ref();
	}

	expSequence = var;
	expSequence->_add_ref();
}

void Control_i::remove_ExpSequence()
{
	if(expSequence != NULL)
	{
		expSequence->_remove_ref();
	}

	expSequence = NULL;
}



void Control_i::add_ModeHandler(ModeHandler_i* var)
{
	assert(var != NULL);

	if(modeHandler != NULL)
	{
		// Remove reference to the current ModeHandler_i servant, allowing
		// for the possibility that var is a new instance of the servant.
		modeHandler->_remove_ref();
	}

	modeHandler = var;
	modeHandler->_add_ref();
}

void Control_i::remove_ModeHandler()
{
	if(modeHandler != NULL)
	{
		modeHandler->_remove_ref();
	}

	modeHandler = NULL;
}






STI::Types::TStatus Control_i::status()
{
	STI::Types::TStatus dummy;
	dummy.curTime = 0;
	return dummy;
}


void Control_i::reset()
{
}


void Control_i::setDirect()
{
}


void Control_i::runSingle(::CORBA::Boolean documented, const STI::Types::TExpRunInfo& info)
{
	if( !sti_Server->requestPlay() )
		return;

	sti_Server->playEvents();
	cout << "played" << endl;
	
	if (documented)
	{
		ExperimentDocumenter documenter(info);
		documenter.addTimingFiles( parser->getTimingFiles() );
		documenter.addVariables( parser->getParsedVars() );

		const std::vector<std::string>& devicesWithEvents = sti_Server->getDevicesWithEvents();
		const RemoteDeviceMap& registeredDevices = sti_Server->getRegisteredDevices();

		for(unsigned i = 0; i < devicesWithEvents.size(); i++)
		{
			documenter.addDeviceData( *registeredDevices.find(devicesWithEvents.at(i))->second );
		}
	
		documenter.writeToDisk();
	}
}


void Control_i::runSequence(::CORBA::Boolean documented, const STI::Types::TExpSequenceInfo& info)
{
/*
<mySequences>
	*mySequence.xml
		<mySequence>
			* myTrial_1.xml
			* myTrial_2.xml
			<timing>
				* myTiming.py
				* myChannels.py
*/

	STI::Types::TExpRunInfo currentExperimentInfo;
	currentExperimentInfo.isSequenceMember = true;

	SequenceDocumenter sequence(info, parser);

	if(documented)
	{
		sequence.writeDirectoryStructureToDisk();
		sequence.copyTimingFiles();
		sequence.createSequenceXML();
	}

	bool parsingSuccess;
	bool runsRemaining = expSequence->getNextExperiment();

	unsigned experimentNumber = 0;

	while(runsRemaining)
	{
		if(documented)
		{
			currentExperimentInfo.filename 
				= sequence.generateExperimentFilename("_" + experimentNumber).c_str();
			
			currentExperimentInfo.serverBaseDirectory 
				= sequence.getExperimentAbsDirectory().c_str();
			
			currentExperimentInfo.sequenceRelativePath 
				= sequence.getSequenceRelativePath().c_str();	//includes directory and filename
		}

		parser->overwritten( expSequence->getCurrentOverwritten() );
		parsingSuccess = !parser->parseSequenceTimingFile();

		if( !parsingSuccess )
			break;

		runSingle(documented, currentExperimentInfo);
		expSequence->setCurrentExperimentToDone();

		if(documented)
			sequence.addExperiment(currentExperimentInfo);

		runsRemaining = expSequence->getNextExperiment();	//sets up overwritten variables in parser
		experimentNumber++;

	}

	if(documented)
	{
		sequence.writeSequenceXML();
	}
}


void Control_i::_cxx_continue()
{
}


void Control_i::stop()
{
	sti_Server->stopServer();
	sti_Server->stopAllDevices();
}

void Control_i::pause()
{
	sti_Server->pauseServer(false);
}

void Control_i::resume()
{
//	if( !sti_Server->isPausedByDevice() )
//		sti_Server->playEvents();

	sti_Server->unpauseServer(false);

}


STI::Client_Server::ExpSequence_ptr Control_i::expSeq()
{
	STI::Client_Server::ExpSequence_ptr dummy = 0;
	return dummy;
}

char* Control_i::errMsg()
{
	const char* dummy = "dummy";
	return CORBA::string_dup(dummy);
}

char* Control_i::transferErr(const char* deviceID)
{
	CORBA::String_var error( sti_Server->getTransferErrLog(deviceID).c_str() );
	return error._retn();
}

STI::Types::TExpRunInfo* Control_i::getDefaultRunInfo()
{
	std::string defaultSingleRunPath = "c:/code";
	std::string defaultSingleRunFilename = "trial";


	using STI::Types::TExpRunInfo;
	using STI::Types::TExpRunInfo_var;
	
	TExpRunInfo_var tRunInfo( new TExpRunInfo() );

	tRunInfo->serverBaseDirectory = defaultSingleRunPath.c_str();
	tRunInfo->filename = defaultSingleRunFilename.c_str();
	tRunInfo->description = (parser->getParsedDescription()).c_str();

	return tRunInfo._retn();
}

STI::Types::TExpSequenceInfo* Control_i::getDefaultSequenceInfo()
{
	std::string defaultSequenceFilename = "timingSeq.xml";
	std::string defaultSequencePath = "c:/code";
	std::string defaultSequenceFilenameBase = "timingSeq";


	using STI::Types::TExpSequenceInfo;
	using STI::Types::TExpSequenceInfo_var;
	
	TExpSequenceInfo_var tSeqInfo( new TExpSequenceInfo() );

	tSeqInfo->sequenceDescription = "";
	tSeqInfo->filename = defaultSequenceFilename.c_str();
	tSeqInfo->serverBaseDirectory = defaultSequencePath.c_str();
	tSeqInfo->trialFilenameBase = defaultSequenceFilenameBase.c_str();

	return tSeqInfo._retn();

}

