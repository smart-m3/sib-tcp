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

#ifndef __HDX_CONNECTION_HANDLER_H__
#define __HDX_CONNECTION_HANDLER_H__

#include "Runnable.h"
#include "ThreadPool.h"
#include "ClientHandler.h"

namespace HDX
{
	template <class T>
	class CConnectionHandler : public IRunnable
	{
		public:
			CConnectionHandler(CDexDaemon* aDaemon)
				: iDaemon(aDaemon), iSocket(HDX_PROTOCOL_NAME) {};
			~CConnectionHandler() { iSocket.Close(); };
			void Run();

		private:
			CDexDaemon* iDaemon;
			CIPSocket<T> iSocket;
	};

	template <class T>
	void CConnectionHandler<T>::Run()
	{
		while (iDaemon->iRunning)
		{
			TCharArray tStr;
			iSocket >> tStr;

			if (!tStr.empty())
			{
				CClientHandler* pHandler =
					new CClientHandler(iDaemon,
					new CIPSocket<T>(iSocket), tStr);

				iDaemon->iPool->Assign(pHandler);
			}
		}
	};
};

#endif
