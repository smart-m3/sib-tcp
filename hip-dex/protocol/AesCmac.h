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

#ifndef __HDX_AES_CMAC_H__
#define __HDX_AES_CMAC_H__

#include <openssl/cmac.h>
#include <openssl/rand.h>
#include "HipParameter.h"

namespace HDX
{
	enum TAesKeyLength
	{
		tKey128 = 16,
		tKey192 = 24,
		tKey256 = 64,
	};

	class CAesCmac
	{
		public:
			CAesCmac(TAesKeyLength, const TCharArray&);
			void UpdateBlock(unsigned char*, std::size_t);
			void UpdateBlock(const TCharArray&);
			TCharArray CalculateCmac() const;
			TCharArray RandomKey() const;
			~CAesCmac();

		private:
			CAesCmac(const CAesCmac&) {};
			CAesCmac& operator=(const CAesCmac&)
				{ return *this; }

			CMAC_CTX* iContext;
			const EVP_CIPHER* iCipher;
	};
};

#endif
