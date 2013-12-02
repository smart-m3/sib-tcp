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

#ifndef __HDX_IP_SOCKET_H__
#define __HDX_IP_SOCKET_H__

#include "SocketException.h"

#include <cstring>
#include <cerrno>
#include <sys/types.h>

namespace HDX
{
	class MIPSocket
	{
		public:
			MIPSocket() {};
			virtual const MIPSocket& operator << (const TCharArray&) const = 0;
			virtual const MIPSocket& operator >> (TCharArray&) const = 0;
			virtual ~MIPSocket() {};
			virtual void Close() = 0;
			virtual long GetIP() = 0;
	};

	template <class T>
	class CIPSocket : public MIPSocket, private T
	{
		public:
			explicit CIPSocket(const std::string&);
			CIPSocket(const std::string&, const std::string&);
			const CIPSocket& operator << (const TCharArray&) const;
			const CIPSocket& operator >> (TCharArray&) const;
			~CIPSocket() {};
			void Close();
			long GetIP();
	};


	template <class T>
	long CIPSocket<T>::GetIP()
	{
	    return T::GetIP();
	};
	
	template <class T>
	CIPSocket<T>::CIPSocket(const std::string& aProto)
	{
		if (!T::Create(aProto.c_str()))
		{
			throw CSocketException("Creating socket failed. Reason: " + std::string(strerror(errno)));
		}

		if (!T::Bind())
		{
			throw CSocketException("Binding socket failed: Reason: " + std::string(strerror(errno)));
		}
	};

	template <class T>
	CIPSocket<T>::CIPSocket(const std::string& aProto, const std::string& aAddr)
	{
		if (!T::Create(aProto))
		{
			throw CSocketException("Creating socket failed. Reason: " + std::string(strerror(errno)));
		}

		if (!T::Connect(aAddr))
		{
			throw CSocketException("Connecting to host failed. Reason: " + std::string(strerror(errno)));
		}
	};

	template <class T>
	const CIPSocket<T>& CIPSocket<T>::operator << (const TCharArray& s) const
	{
		if (!T::Send(s))
		{
			throw CSocketException("Could not write to socket. Reason: " + std::string(strerror(errno)));
		}

		return *this;
	};

	template <class T>
	const CIPSocket<T>& CIPSocket<T>::operator >> (TCharArray& s) const
	{
		if (!T::Recv(s))
		{
			throw CSocketException("Could not read from socket. Reason: " + std::string(strerror(errno)));
		}

		return *this;
	};

	template <class T>
	void CIPSocket<T>::Close()
	{
		T::Close();
	};
};

#endif
