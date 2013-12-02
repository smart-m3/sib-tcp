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

#include "HipPacketFactory.h"
#include "HipDefines.h"
#include "HipPacket.h"

#include "HipPacketI1.h"
#include "HipPacketR1.h"
#include "HipPacketI2.h"
#include "HipPacketR2.h"

using namespace HDX;

CHipPacketFactory::CHipPacketFactory()
{
/*	iPackets[HDX_TYPE_PACKET_I1] = &(CHipPacketI1::Create);
	iPackets[HDX_TYPE_PACKET_R1] = &(CHipPacketR1::Create);
	iPackets[HDX_TYPE_PACKET_I2] = &(CHipPacketI2::Create);
	iPackets[HDX_TYPE_PACKET_R2] = &(CHipPacketR2::Create);*/
};

CHipPacketFactory::~CHipPacketFactory()
{
};

MHipPacket* CHipPacketFactory::Create(TCharArray& aBytes) const
{
	if (aBytes.size() < HDX_SIZE_PACKET_HDR)
		return NULL;

	unsigned char tType = aBytes[2] & 0x7F;

	MHipPacket* tPacket = NULL;
	switch (tType) {
	case HDX_TYPE_PACKET_I1:
		tPacket = CHipPacketI1::Create();
		break;
	case HDX_TYPE_PACKET_R1:
		tPacket = CHipPacketR1::Create();
		break;
	case HDX_TYPE_PACKET_I2:
		tPacket = CHipPacketI2::Create();
		break;
	case HDX_TYPE_PACKET_R2:
		tPacket = CHipPacketR2::Create();
		break;
	}

	if (tPacket != NULL)
	{
		if (tPacket->FromBytes(aBytes))
			return tPacket;
		delete tPacket;
	}

	return NULL;
};
