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

#ifndef __HDX_HIP_PACKET_H__
#define __HDX_HIP_PACKET_H__

#include "HipParamFactory.h"
#include "HipParameter.h"

namespace HDX
{
	class MHipPacket
	{
		public:
			explicit MHipPacket(unsigned char,
				const TCharArray& = TCharArray(16),
				const TCharArray& = TCharArray(16));
			unsigned short CalcChecksum(const TCharArray& aBytes) const;
			TCharArray CalculateCmac(const TCharArray&) const;
			MHipParameter* GetParameter(unsigned short);
			bool VerifyCmac(const TCharArray&);
			TCharArray GetSenderHit() const;
			void SetSenderHit(const TCharArray&);
			TCharArray GetReceiverHit() const;
			void SetReceiverHit(const TCharArray&);
			void AddParameter(MHipParameter* aParam);

			bool FromBytes(TCharArray&);
			TCharArray ToBytes() const;
			unsigned char Type() const;
			unsigned int Length() const;
			virtual ~MHipPacket() = 0;

		protected:
			unsigned char iNextHdr;
			unsigned char iType;
			unsigned char iVersion;
			unsigned short iCtrls;
			TCharArray iSenderHit;
			TCharArray iReceiverHit;

			std::vector<MHipParameter*> iParams;
	};
};

#endif
