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

#ifndef __HDX_HIP_HOST_ID_H__
#define __HDX_HIP_HOST_ID_H__

#include "HipParameter.h"
#include "HipECKey.h"

namespace HDX
{
	enum TDIType
	{
		tNone = 0,
		tFQDN = 1,
		tNAI = 2,
	};

	class CHipHostId : public MHipParameter
	{
		public:
			CHipHostId();
			CHipHostId(TDIType,
					const TCharArray&,
					const TCharArray&);
			static MHipParameter* Create();
			~CHipHostId();

			unsigned short Type() const;
			unsigned short Size() const;
			TCharArray GetBytes() const;
			bool SetBytes(const TCharArray&);
			bool SetBytes(const TCharArray&,
				std::size_t, std::size_t);

			unsigned char GetDiType() const;
			void SetDiType(unsigned char);
			const TCharArray& GetHostId() const;
			void SetHostId(const TCharArray&);
			const TCharArray& GetDomainId() const;
			void SetDomainId(const TCharArray&);

		private:
			unsigned char iDiType;
			TCharArray iDomainId;
			TCharArray iHostId;
	};
};

#endif
