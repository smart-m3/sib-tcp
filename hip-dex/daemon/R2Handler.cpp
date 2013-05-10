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

#include "R2Handler.h"
#include "HipDefines.h"
#include "HipPacketR2.h"
#include "HipKeyDeriv.h"
#include "HipEncryptedKey.h"
#include "HipDhGroupList.h"
#include "Connection.h"
#include "HipHipMac3.h"
#include "Context.h"
#include "HipECKey.h"
#include "HipUtils.h"
#include "AesCbc.h"
#include "DexDaemon.h"
#include <iostream>

using namespace HDX;

CR2Handler::CR2Handler()
	: MPacketHandler(tI2Sent, tEstablished)
{
};

CR2Handler::~CR2Handler()
{
};

MPacketHandler* CR2Handler::Create()
{
	return new CR2Handler;
};

void CR2Handler::Handle(MHipPacket* aPacket, CContext* aContext)
{
	if (CHipPacketR2* tPacket = dynamic_cast<CHipPacketR2*>(aPacket))
	{
		CHipEncryptedKey* tEncryptedKey = (CHipEncryptedKey*)tPacket->GetParameter(HDX_TYPE_ENCRYPTED_KEY);
		CHipDhGroupList* tDhList = (CHipDhGroupList*)tPacket->GetParameter(HDX_TYPE_DH_GROUP_LIST);

		if (!tPacket->VerifyCmac(aContext->iConn->remoteIK))
			return;

		if (tDhList->GetList() != aContext->iConn->tDhGroupList)
			return;

		CAesCbc tOpen(tKey128,
			aContext->iConn->remoteEK,
			aContext->iConn->tRandomI);

		TCharArray tDecrypted =
			tOpen.Decrypt(tEncryptedKey->GetData());

		if (!std::equal(tDecrypted.begin()+tKey128,
			tDecrypted.end(), aContext->iConn->tRandomI
			.begin())) return;

		tDecrypted.erase(tDecrypted.begin()+tKey128,
			tDecrypted.end());

		aContext->iConn->sessionY = tDecrypted;

#ifdef HDX_DEBUG_OUTPUT
		std::cout << "Session-Y:" << std::endl;
		print_array(aContext->iConn->sessionY);
#endif
		TCharArray tXY = aContext->iConn->sessionX;
		tXY.insert(tXY.end(), tDecrypted.begin(), tDecrypted.end());

		CHipKeyDeriv tDeriv2(16, 16);
		tDeriv2.DeriveKeys(tPacket->GetSenderHit(),
			tPacket->GetReceiverHit(),
			aContext->iConn->tRandomI, tXY);


		aContext->iDaemon->sadb.StoreSA(tPacket->GetSenderHit(), tPacket->GetReceiverHit(), tDeriv2.iIEK, tDeriv2.iREK, aContext->iSocket->GetIP());


#ifdef HDX_DEBUG_OUTPUT
		std::cout << "New derived keys (from Session-XY):" << std::endl;
		print_array(tDeriv2.iIIK);
		print_array(tDeriv2.iIEK);
		print_array(tDeriv2.iRIK);
		print_array(tDeriv2.iREK);
#endif
	}
};

void CR2Handler::print_array(const TCharArray& aArray)
{
	for (size_t i = 0; i < aArray.size(); ++i)
		std::cout << std::hex << (int)aArray[i]<< " ";

	std::cout << std::endl;
};
