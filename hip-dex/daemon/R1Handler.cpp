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

#include "AesCmac.h"
#include "IPSocket.h"
#include "HipHipCipher.h"
#include "HipSolution.h"
#include "HipPuzzle.h"
#include "HipDefines.h"
#include "R1Handler.h"
#include "HipKeyDeriv.h"
#include "HipPuzzleUtil.h"
#include "HipEncryptedKey.h"
#include "HipPacketR1.h"
#include "HipECKey.h"
#include "HipUtils.h"
#include "Context.h"
#include "Connection.h"
#include "DexDaemon.h"
#include "HipPacketI2.h"
#include "HipHipMac3.h"
#include "AesCbc.h"

#include <iostream>

using namespace HDX;

CR1Handler::CR1Handler()
	: MPacketHandler(tI1Sent, tI2Sent)
{
};

CR1Handler::~CR1Handler()
{
};

MPacketHandler* CR1Handler::Create()
{
	return new CR1Handler;
};

void CR1Handler::Handle(MHipPacket* aPacket, CContext* aContext)
{
	if (CHipPacketR1* tPacket = dynamic_cast<CHipPacketR1*>(aPacket))
	{
		CHipPuzzle* tPuzzle = (CHipPuzzle*)tPacket->GetParameter(HDX_TYPE_PUZZLE);
		CHipHostId* tHostId = (CHipHostId*)tPacket->GetParameter(HDX_TYPE_HOST_ID);

		TCharArray tShared = hdx_ecdh(*tHostId, *(aContext->iKey));

#ifdef HDX_DEBUG_OUTPUT
		std::cout << "Shared key: " << std::endl;
		print_array(tShared);
#endif

		CHipPuzzleUtil tUtil;
		TCharArray tSolution = tUtil.SolveR(tPuzzle->GetRandomI(),
			tPacket->GetReceiverHit(), tPacket->GetSenderHit(),
			tPuzzle->GetComplexity());

		aContext->iConn->tRandomI = tPuzzle->GetRandomI();

		CHipKeyDeriv tDeriv(16, 16);
		tDeriv.DeriveKeys(tPacket->GetSenderHit(),
			tPacket->GetReceiverHit(), tPuzzle->GetRandomI(),
			tShared);

		aContext->iConn->localIK = tDeriv.iIIK;
		aContext->iConn->localEK = tDeriv.iIEK;
		aContext->iConn->remoteIK = tDeriv.iRIK;
		aContext->iConn->remoteEK = tDeriv.iREK;

#ifdef HDX_DEBUG_OUTPUT
		std::cout << "Derived keys:" << std::endl;
		print_array(tDeriv.iIIK);
		print_array(tDeriv.iIEK);
		print_array(tDeriv.iRIK);
		print_array(tDeriv.iREK);
#endif

		CAesCbc tAes(tKey128,
				aContext->iConn->localEK,
			tPuzzle->GetRandomI());

		CAesCmac tMak(tKey128, TCharArray(tKey128));
		TCharArray tConcat = tMak.RandomKey();
		aContext->iConn->sessionX = tConcat;

#ifdef HDX_DEBUG_OUTPUT
		std::cout << "Session-X: " << std::endl;
		print_array(aContext->iConn->sessionX);
#endif

		tConcat.insert(tConcat.end(),
			tPuzzle->GetRandomI().begin(),
			tPuzzle->GetRandomI().end());

		CHipEncryptedKey* tEKey = new CHipEncryptedKey(tAes.Encrypt(tConcat));
		CHipSolution* tSolutionP = new CHipSolution(tPuzzle->GetComplexity(),
			tPuzzle->GetOpaque(), tPuzzle->GetRandomI(), tSolution);

		CHipHostId* tOurHostId = new CHipHostId(tNone,
			aContext->iKey->ToHostId(), TCharArray());
		CHipCipher* tCipher = new CHipCipher(0x02);

		CHipPacketI2 tPacketI2(tSolutionP, tCipher, tEKey, tOurHostId);
		tPacketI2.SetReceiverHit(tPacket->GetSenderHit());
		tPacketI2.SetSenderHit(tPacket->GetReceiverHit());

		CHipHipMac3* tMac = new CHipHipMac3(tPacketI2.
			CalculateCmac(aContext->iConn->localIK));
		tPacketI2.AddParameter(tMac);

		*(aContext->iSocket) << tPacketI2.ToBytes();
	}
};

void CR1Handler::print_array(const TCharArray& aArray)
{
	for (size_t i = 0; i < aArray.size(); ++i)
		std::cout << std::hex << (int)aArray[i]<< " ";

	std::cout << std::endl;
};
