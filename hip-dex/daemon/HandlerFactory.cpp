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

#include <cstddef>

#include "HandlerFactory.h"
#include "HipDefines.h"
#include "HipPacket.h"

#include "I1Handler.h"
#include "R1Handler.h"
#include "I2Handler.h"
#include "R2Handler.h"

using namespace HDX;

CHandlerFactory::CHandlerFactory()
{
	iHandlers[HDX_TYPE_PACKET_I1] = &(CI1Handler::Create);
	iHandlers[HDX_TYPE_PACKET_R1] = &(CR1Handler::Create);
	iHandlers[HDX_TYPE_PACKET_I2] = &(CI2Handler::Create);
	iHandlers[HDX_TYPE_PACKET_R2] = &(CR2Handler::Create);
};

CHandlerFactory::~CHandlerFactory()
{
};

MPacketHandler* CHandlerFactory::Create(unsigned short aType) const
{
	HandlerList::const_iterator tIter = iHandlers.find(aType);

	if (tIter != iHandlers.end())
	{
		MPacketHandler* pHandler = (*(tIter->second))();

		if (pHandler)
			return pHandler;
	}

	return NULL;
};
