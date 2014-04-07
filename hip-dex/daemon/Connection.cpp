/**
 * Host Identity Protocol (HIP) Diet Exchange (DEX) C++ Library (HDX++)
 *
 * HDX++ is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HDX++ is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License and GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with HDX++. If not, see <http://www.gnu.org/licenses/>.
 *
 * This file is part of the HDX++ library and authored by Jani Pellikka
 * <jani.pellikka@iki.fi>. Copyright (C) 2011 by University of Oulu.
 */

#include "Connection.h"
#include "DexDaemon.h"
#include "PacketHandler.h"
#include "HandlerFactory.h"
#include "HipPacket.h"
#include "Context.h"

using namespace HDX;

const CHandlerFactory sHandlerFactory;

CConnection::CConnection()
{
};

CConnection::~CConnection()
{
};

void CConnection::Handle(MHipPacket* aPacket, CContext* aContext)
{
	MPacketHandler* pHandler = sHandlerFactory.Create(aPacket->Type());

	if (pHandler)
	{
		if ((pHandler->AllowedStates() & currentState))
		{
			pHandler->Handle(aPacket, aContext);
			currentState = pHandler->NextState();
		}

		delete pHandler;
	}
};
