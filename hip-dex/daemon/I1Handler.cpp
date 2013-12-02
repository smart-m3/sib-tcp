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

#include "HipDefines.h"
#include "I1Handler.h"
#include "HipHostId.h"
#include "IPSocket.h"
#include "HipPacketI1.h"
#include "HipPacketR1.h"
#include "HipPuzzleUtil.h"
#include "HipDhGroupList.h"
#include "HipHitSuiteList.h"
#include "HipHipCipher.h"
#include "HipPuzzle.h"
#include "Connection.h"
#include "Context.h"
#include <iostream>

using namespace HDX;

CI1Handler::CI1Handler()
	: MPacketHandler(tUnassociated|tR2Sent, tUnassociated)
{
};

CI1Handler::~CI1Handler()
{
};

MPacketHandler* CI1Handler::Create()
{
	return new CI1Handler;
};

void CI1Handler::Handle(MHipPacket* aPacket, CContext* aContext)
{
	if (CHipPacketI1* tPacket = dynamic_cast<CHipPacketI1*>(aPacket))
	{
		CHipPuzzleUtil tUtil;

		TCharArray tI = tUtil.CalculateI(tPacket->GetSenderHit(),
			tPacket->GetReceiverHit(), TCharArray(), TCharArray());

		CHipPuzzle* tPuzzle = new CHipPuzzle(8, 40, 0, tI);
		CHipCipher* tCipher = new CHipCipher(0x02); // AES-128-CBC
		CHipHitSuiteList* tHitList = new CHipHitSuiteList(TCharArray(1, 0x08)); // HIP-DEX
		CHipHostId* tHostId = new CHipHostId(tNone, aContext->iKey->ToHostId(), TCharArray());
		CHipDhGroupList* tList = new CHipDhGroupList(DH_GROUP_ECP192);

		CHipPacketR1 tPacketR1(tPuzzle, tCipher, tHostId, tHitList, tList);
		tPacketR1.SetReceiverHit(tPacket->GetSenderHit());
		tPacketR1.SetSenderHit(tPacket->GetReceiverHit());

		*(aContext->iSocket) << tPacketR1.ToBytes();
	}
};
