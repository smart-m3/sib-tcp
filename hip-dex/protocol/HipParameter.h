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

#ifndef __HDX_HIP_PARAMETER_H__
#define __HDX_HIP_PARAMETER_H__

#include "HipParamFactory.h"

namespace HDX
{
	class MHipParameter
	{
		public:
			explicit MHipParameter();
			TCharArray ToBytes() const;
			unsigned short Length() const;
			virtual unsigned short Type() const = 0;
			bool FromBytes(const TCharArray&, std::size_t = 0);
			virtual ~MHipParameter() = 0;

		protected:
			virtual unsigned short Size() const = 0;
			virtual TCharArray GetBytes() const = 0;
			virtual bool SetBytes(const TCharArray&) = 0;
			virtual bool SetBytes(const TCharArray&,
				std::size_t, std::size_t) = 0;
	};
};

#endif
