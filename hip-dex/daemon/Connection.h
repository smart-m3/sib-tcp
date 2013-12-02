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

#ifndef __HDX_CONNECTION_H__
#define __HDX_CONNECTION_H__

#include "HipParamFactory.h"
#include <map>

namespace HDX
{
	enum TDexState
	{
		tUnassociated	= 0x01,
		tEstablished	= 0x02,
		tI1Sent 		= 0x04,
		tI2Sent			= 0x08,
		tR2Sent			= 0x10,
		tClosing		= 0x20,
		tClosed 		= 0x40,
	};

	struct TState
	{
		TCharArray tDhGroupList;
		TDexState currentState;
		TCharArray tRandomI;
		TCharArray localEK;
		TCharArray localIK;
		TCharArray remoteEK;
		TCharArray remoteIK;
		TCharArray sessionX;
		TCharArray sessionY;
	};

	class MHipPacket;
	class CContext;

	class CConnection : public TState
	{
		public:
			CConnection();
			~CConnection();
			void Handle(MHipPacket*, CContext*);
	};
};

#endif
