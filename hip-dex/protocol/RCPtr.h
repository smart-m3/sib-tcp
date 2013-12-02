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

#ifndef __HDX_RC_PTR_H__
#define __HDX_RC_PTR_H__

#include <cstddef>

namespace HDX
{
	template <class T> class CRCPtr
	{
		public:
			explicit CRCPtr(T* aPtr = 0);
			CRCPtr(const CRCPtr& aRef);
			CRCPtr& operator=(const CRCPtr& aRef);
			const T& operator*() const { return *iPtr; };
			const T* operator->() const { return iPtr; };
			operator bool() const { return iPtr; };
			T& operator*() { return *iPtr; };
			T* operator->() { return iPtr; };
			void Unique();
			~CRCPtr();

		private:
			T* iPtr;
			std::size_t* iCount;
	};

	template <class T> CRCPtr<T>::CRCPtr(T* aPtr)
		: iPtr(aPtr), iCount(new std::size_t(1))
	{
	};

	template <class T> CRCPtr<T>::~CRCPtr()
	{
		if (--*iCount == 0)
		{
			delete iPtr;
			delete iCount;
		}
	};

	template <class T> CRCPtr<T>::CRCPtr(const CRCPtr& aRef)
		: iPtr(aRef.iPtr), iCount(aRef.iCount)
	{
		++*iCount;
	};

	template <class T> CRCPtr<T>& CRCPtr<T>::operator=(const CRCPtr& aRef)
	{
		if (this->iPtr == aRef.iPtr)
			return *this;

		++*aRef.iCount;

		if (--*iCount == 0)
		{
			delete iPtr;
			delete iCount;
		}

		iPtr = aRef.iPtr;
		iCount = aRef.iCount;

		return *this;
	};

	template <class T> void CRCPtr<T>::Unique()
	{
		if (*iCount == 1) return;

		--*iCount;
		iCount = new std::size_t(1);
		iPtr = iPtr ? new T(*iPtr) : 0;
	};
};

#endif
