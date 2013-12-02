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

#ifndef __HDX_CLIENT_HANDLER_H__
#define __HDX_CLIENT_HANDLER_H__

#include "HipPacketFactory.h"
#include "HipParamFactory.h"
#include "Connection.h"
#include "IPSocket.h"
#include "HipPacket.h"
#include "Context.h"
#include "Runnable.h"
#include "SafePtr.h"
#include "DexDaemon.h"

namespace HDX
{
	const CHipPacketFactory sPacketFactory;

	class CClientHandler : public IRunnable
	{
		public:
			CClientHandler(CDexDaemon* aDaemon, MIPSocket* aSocket, const TCharArray& aBytes)
				: iDaemon(aDaemon), iSocket(aSocket), iBytes(aBytes) {};
			~CClientHandler() { delete iSocket; };
			void Run();

		private:
			CDexDaemon* iDaemon;
			MIPSocket* iSocket;
			TCharArray iBytes;
	};

	void CClientHandler::Run()
	{
		MHipPacket* pMsg = sPacketFactory.Create(iBytes);

		if (pMsg != NULL)
		{
			CSafePtr<CConnection> tConn = (*iDaemon->
				iConnFactory)->Create(pMsg->GetSenderHit());

			if (tConn)
			{
				CContext tContext(iDaemon, &((CConnection&)(*tConn)), iSocket, iDaemon->iHipECKey);
				(*tConn)->Handle(pMsg, &tContext);
			}

			delete pMsg;
		}
	};
};

#endif
