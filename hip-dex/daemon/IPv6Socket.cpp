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

#include "IPv6Socket.h"
#include "Defines.h"
#include <iostream>
#include <cstring>
#include <cerrno>
#include <fcntl.h>

using namespace HDX;

CIPv6Socket::CIPv6Socket()
	: iSocket(-1)
{
	memset(&iAddr, 0, sizeof(iAddr));
};

CIPv6Socket::~CIPv6Socket()
{
};

bool CIPv6Socket::Create(const std::string& aProto)
{
	struct protoent* tProto;

	if (!(tProto = getprotobyname(aProto.c_str())))
	{
		errno = EPROTONOSUPPORT;
		return false;
	}

	if ((iSocket = socket(AF_INET6, SOCK_RAW, tProto->p_proto)) == -1)
		return false;

	int tOpt = 1;

	if (setsockopt(iSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&tOpt,
		sizeof(tOpt)) == -1) return false;

	return true;
};

bool CIPv6Socket::Bind()
{
	if (iSocket == -1)
		return false;

	struct sockaddr_in6 tAddr;
	memset(&tAddr, 0, sizeof(tAddr));
	tAddr.sin6_addr = in6addr_any;
	tAddr.sin6_family = AF_INET6;

	if (bind(iSocket, (struct sockaddr*)&tAddr,
		sizeof(tAddr)) == -1) return false;

	struct timeval tTv;
	tTv.tv_sec = HDX_SOCK_RCVTIMEO / 1000;
	tTv.tv_usec = (HDX_SOCK_RCVTIMEO%1000)*1000;

	if (setsockopt(iSocket, SOL_SOCKET, SO_RCVTIMEO,
		(char*)&tTv, sizeof(tTv)) < 0) return false;

	return true;
};

bool CIPv6Socket::Connect(const std::string& aAddr)
{
	if (iSocket == -1)
		return false;

	if (inet_pton(AF_INET6, aAddr.c_str(), &iAddr.sin6_addr) < 1)
		return false;

	iAddr.sin6_family = AF_INET6;

	return true;
};

bool CIPv6Socket::Send(const TCharArray& aBytes) const
{
	if (sendto(iSocket, &aBytes[0], aBytes.size(), 0,
		(struct sockaddr*)&iAddr, sizeof(iAddr)) < 1)
		return false;

	return true;
};

bool CIPv6Socket::Recv(TCharArray& aBytes) const
{
	char tBuf[HDX_LEN_RECV_BUF] = {0};
	unsigned int tLen = sizeof(iAddr);
	ssize_t tBytes;

	if ((tBytes = recvfrom(iSocket, tBuf, HDX_LEN_RECV_BUF, 0,
		(struct sockaddr*)&iAddr, (socklen_t*)&tLen)) < 1)
	{
		return (errno == EAGAIN ||
			errno == EINTR) ? true : false;
	}

	aBytes.assign(tBuf, tBuf+tBytes);
	return true;
};

void CIPv6Socket::Close()
{
	if (iSocket != -1)
	{
		close(iSocket);
		iSocket = -1;
	}
};

long CIPv6Socket::GetIP()
{
	return iAddr.sin6_addr.s6_addr[15] << 24 +
	       iAddr.sin6_addr.s6_addr[14] << 16 +
	       iAddr.sin6_addr.s6_addr[13] << 8 +
	       iAddr.sin6_addr.s6_addr[12];
};
