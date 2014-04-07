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

#ifndef __HDX_HIP_MAC_3_H__
#define __HDX_HIP_MAC_3_H__

#include "HipParameter.h"

namespace HDX
{
	class CHipHipMac3 : public MHipParameter
	{
		public:
			CHipHipMac3();
			explicit CHipHipMac3(const TCharArray&);
			static MHipParameter* Create();
			~CHipHipMac3();

			unsigned short Type() const;
			unsigned short Size() const;
			TCharArray GetBytes() const;
			bool SetBytes(const TCharArray&);
			bool SetBytes(const TCharArray&,
				std::size_t, std::size_t);

			void SetData(const TCharArray&);
			TCharArray GetData() const;

		private:
			TCharArray iData;
	};
};

#endif
