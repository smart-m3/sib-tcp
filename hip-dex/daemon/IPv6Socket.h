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

#ifndef __HDX_IPV6_SOCKET_H__
#define __HDX_IPV6_SOCKET_H__

#include "HipParamFactory.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string>

namespace HDX
{
	class CIPv6Socket
	{
		public:
			CIPv6Socket();
			bool Create(const std::string&);
			bool Connect(const std::string&);
			bool Send(const TCharArray&) const;
			bool Recv(TCharArray&) const;
			virtual ~CIPv6Socket();
			bool Bind();
			void Close();
			long GetIP();

		protected:
			int iSocket;
			sockaddr_in6 iAddr;
	};
};

#endif
