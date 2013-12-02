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

#ifndef __HDX_SAFE_PTR_H__
#define __HDX_SAFE_PTR_H__

#include "RCPtr.h"
#include "Mutex.h"

namespace HDX
{
	template <class T> class CSafePtr
	{
		public:
			explicit CSafePtr(T* aRef = 0)
				: iMutex(new CMutex), iValue(aRef) {};
			operator bool() const { return iValue; };
			void Unique();
			~CSafePtr() {};

			class CPtrProxy
			{
				public:
					CPtrProxy(CMutex& aMutex, T& aValue);
					CPtrProxy(const CPtrProxy& aProxy);
					CPtrProxy& operator=(const T& aValue);
					CPtrProxy& operator=(const CPtrProxy& aProxy);
					const T* operator->() const { return &iValue; };
					operator const T&() const { return iValue; };
					T* operator->() { return &iValue; };
					operator T&() { return iValue; };
					~CPtrProxy();

				private:
					CMutex& iMutex;
					T& iValue;
			};

			CPtrProxy operator*() { return CPtrProxy(*iMutex, *iValue); };
			const CPtrProxy operator*() const { return CPtrProxy(*iMutex, *iValue); };

		private:
			CRCPtr<CMutex> iMutex;
			CRCPtr<T> iValue;
	};

	template <class T> void CSafePtr<T>::Unique()
	{
		iMutex = CRCPtr<CMutex>(new CMutex);
		iValue.Unique();
	};

	template <class T> CSafePtr<T>::CPtrProxy::CPtrProxy(CMutex& aMutex, T& aValue)
		: iMutex(aMutex), iValue(aValue)
	{
		iMutex.Lock();
	};

	template <class T> CSafePtr<T>::CPtrProxy::~CPtrProxy()
	{
		iMutex.Unlock();
	};

	template <class T> CSafePtr<T>::CPtrProxy::CPtrProxy(const CPtrProxy& aPtr)
		: iMutex(aPtr.iMutex), iValue(aPtr.iValue)
	{
		iMutex.Lock();
	};

	template <class T> typename CSafePtr<T>::CPtrProxy& CSafePtr<T>::CPtrProxy::operator=(const CPtrProxy& aProxy)
	{
		if (this == &aProxy) return *this;

		iMutex.Unlock();

		iMutex = aProxy.iMutex;
		iValue = aProxy.iValue;

		return *this;
	};

	template <class T> typename CSafePtr<T>::CPtrProxy& CSafePtr<T>::CPtrProxy::operator=(const T& aValue)
	{
		iValue = aValue;
		return *this;
	};
};

#endif
