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

#ifndef __HDX_HIP_UTILS_H__
#define __HDX_HIP_UTILS_H__

#include "HipParamFactory.h"
#include "HipHostId.h"
#include "HipECKey.h"

namespace HDX
{
	TCharArray hdx_ltrunc(const TCharArray&, unsigned int);
	TCharArray hdx_ecdh(const CHipHostId&, const CHipECKey&);

	template<class T> struct FDelete
	{
		FDelete& operator()(T* pPtr)
		{
			delete pPtr;
			return *this;
		}
	};
};

#endif
