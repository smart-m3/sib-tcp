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

#include "I2Handler.h"
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
#include "HipDhGroupList.h"
#include "HipECKey.h"
#include "HipUtils.h"
#include "Context.h"
#include "Connection.h"
#include "DexDaemon.h"
#include "HipPacketR2.h"
#include "HipPacketI2.h"
#include "HipHipMac3.h"
#include "AesCbc.h"
//#include "AesCtr.h"

#include <iostream>


using namespace HDX;

CI2Handler::CI2Handler()
	: MPacketHandler(tUnassociated|tEstablished
	|tI1Sent|tI2Sent|tR2Sent|tClosing|tClosed,
	tR2Sent)
{
};

CI2Handler::~CI2Handler()
{
};

MPacketHandler* CI2Handler::Create()
{
	return new CI2Handler;
};

void CI2Handler::Handle(MHipPacket* aPacket, CContext* aContext)
{
	if (CHipPacketI2* tPacket = dynamic_cast<CHipPacketI2*>(aPacket))
	{
		CHipSolution* tSolution = (CHipSolution*)tPacket->GetParameter(HDX_TYPE_SOLUTION);
		CHipHostId* tHostId = (CHipHostId*)tPacket->GetParameter(HDX_TYPE_HOST_ID);
		CHipEncryptedKey* tEncryptedKey = (CHipEncryptedKey*)tPacket->GetParameter(HDX_TYPE_ENCRYPTED_KEY);

		CHipPuzzleUtil tUtil;
		if (!tUtil.VerifyPuzzle(tSolution->GetRandomI(), tSolution->GetSolutionJ(),
			tPacket->GetSenderHit(), tPacket->GetReceiverHit(), TCharArray(),
			TCharArray())) return;

		TCharArray tShared = hdx_ecdh(*tHostId, *(aContext->iKey));
#ifdef HDX_DEBUG_OUTPUT
		std::cout << "Shared key:" << std::endl;
		print_array(tShared);
#endif
		CHipKeyDeriv tDeriv(16, 16);
		tDeriv.DeriveKeys(tPacket->GetSenderHit(),
			tPacket->GetReceiverHit(),
			tSolution->GetRandomI(), tShared);

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

		if (!tPacket->VerifyCmac(aContext->iConn->remoteIK))
			return;

		CAesCbc tOpen(tKey128,
			aContext->iConn->remoteEK,
			tSolution->GetRandomI());

		TCharArray tDecrypted =
			tOpen.Decrypt(tEncryptedKey->GetData());

		if (!std::equal(tDecrypted.begin()+tKey128,
			tDecrypted.end(), tSolution->GetRandomI()
			.begin())) return;

		tDecrypted.erase(tDecrypted.begin()+tKey128,
			tDecrypted.end());

		aContext->iConn->sessionX = tDecrypted;

#ifdef HDX_DEBUG_OUTPUT
		std::cout << "Session-X:" << std::endl;
		print_array(aContext->iConn->sessionX);
#endif
		CAesCbc tAes(tKey128,
			aContext->iConn->localEK,
			tSolution->GetRandomI());

		CAesCmac tMak(tKey128, TCharArray(tKey128));
		TCharArray tConcat = tMak.RandomKey();
		aContext->iConn->sessionY = tConcat;

#ifdef HDX_DEBUG_OUTPUT
		std::cout << "Session-Y:" << std::endl;
		print_array(aContext->iConn->sessionY);
#endif
		TCharArray tXY = aContext->iConn->sessionX;
		tXY.insert(tXY.end(), tConcat.begin(), tConcat.end());

		CHipKeyDeriv tDeriv2(16, 16);
		tDeriv2.DeriveKeys(tPacket->GetSenderHit(),
			tPacket->GetReceiverHit(),
			tSolution->GetRandomI(), tXY);

#ifdef HDX_DEBUG_OUTPUT
		std::cout << "New derived keys (from Session-XY):" << std::endl;
		print_array(tDeriv2.iIIK);
		print_array(tDeriv2.iIEK);
		print_array(tDeriv2.iRIK);
		print_array(tDeriv2.iREK);
#endif

		aContext->iDaemon->sadb.StoreSA(tPacket->GetSenderHit(), tPacket->GetReceiverHit(), tDeriv2.iIEK, tDeriv2.iREK, aContext->iSocket->GetIP());

		tConcat.insert(tConcat.end(),
			tSolution->GetRandomI().begin(),
			tSolution->GetRandomI().end());

		CHipEncryptedKey* tEKey = new CHipEncryptedKey(tAes.Encrypt(tConcat));
		CHipDhGroupList* tList = new CHipDhGroupList(DH_GROUP_ECP192);

		CHipPacketR2 tPacketR2(tEKey, tList);
		tPacketR2.SetReceiverHit(tPacket->GetSenderHit());
		tPacketR2.SetSenderHit(tPacket->GetReceiverHit());

		CHipHipMac3* tMac = new CHipHipMac3(tPacketR2.
			CalculateCmac(aContext->iConn->localIK));
		tPacketR2.AddParameter(tMac);

		*(aContext->iSocket) << tPacketR2.ToBytes();
		
		
	}
};

void CI2Handler::print_array(const TCharArray& aArray)
{
	for (size_t i = 0; i < aArray.size(); ++i)
		std::cout << std::hex << (int)aArray[i]<< " ";

	std::cout << std::endl;
};
