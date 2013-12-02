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

#ifndef __HDX_HIP_EC_KEY_H__
#define __HDX_HIP_EC_KEY_H__

#include <openssl/ecdh.h>
#include <openssl/objects.h>
#include <openssl/rand.h>
#include <openssl/bn.h>

#include "HipParamFactory.h"

namespace HDX
{
	enum TECType
	{
		tPrime192 = NID_X9_62_prime192v1,
		tPrime250 = NID_X9_62_prime256v1,
	};

	class CHipECKey
	{
		public:
			explicit CHipECKey(TECType);
			CHipECKey(TECType,
				const TCharArray&,
				const TCharArray&);
			TCharArray ToHit() const;
			TCharArray ToHostId() const;
			const EC_KEY* GetECKey() const
				{return iKey; };
			~CHipECKey();

		private:
			EC_KEY* iKey;
	};

};

#endif
