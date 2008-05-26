/*! \file ExpSequence_i.cpp
 *  \author Jason Michael Hogan
 *  \brief Source-file for the class ExpSequence_i
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

#include "ExpSequence_i.h"


ExpSequence_i::ExpSequence_i()
{
}


ExpSequence_i::~ExpSequence_i()
{
}


::CORBA::Boolean ExpSequence_i::appendRow(const STI_Client_Server::TStringSeq& newRow)
{
	return true;
}


::CORBA::Boolean ExpSequence_i::moveRow(::CORBA::ULong oldPos, ::CORBA::ULong newPos)
{
	return true;
}


::CORBA::Boolean ExpSequence_i::editRow(::CORBA::ULong pos, const STI_Client_Server::TStringSeq& newRow)
{
	return true;
}


void ExpSequence_i::editDone(::CORBA::ULong pos, ::CORBA::Boolean newDone)
{
}


STI_Client_Server::TStringSeq* ExpSequence_i::variables()
{
	STI_Client_Server::TStringSeq* dummy = 0;
	return dummy;
}


STI_Client_Server::TRowSeq* ExpSequence_i::experiments()
{
	STI_Client_Server::TRowSeq* dummy = 0;
	return dummy;
}

