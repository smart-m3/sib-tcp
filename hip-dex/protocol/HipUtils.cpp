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

#include "HipUtils.h"
#include <iostream>
#include <openssl/ecdh.h>
#include <openssl/objects.h>
#include <openssl/rand.h>
#include <openssl/bn.h>

using namespace HDX;

TCharArray HDX::hdx_ltrunc(const TCharArray& aInput, unsigned int iNumBits)
{
	if (iNumBits == 8*aInput.size())
		return aInput;

	TCharArray tResult(aInput.begin(), aInput.begin()+((iNumBits+7)>>3));

	if (iNumBits < 8*tResult.size())
	{
		for (std::size_t i = 0; i < 8*tResult.size()-iNumBits; ++i)
		{
			tResult[tResult.size()-1] &= (unsigned char)~(1<<i);
		}
	}

	return tResult;
};

TCharArray HDX::hdx_ecdh(const CHipHostId& aHostId, const CHipECKey& iECKey)
{
	EC_GROUP* group = EC_GROUP_new_by_curve_name(NID_X9_62_prime192v1);
	EC_POINT* point = EC_POINT_new(group);

	TCharArray tHostId = aHostId.GetHostId();
	tHostId = TCharArray(tHostId.begin()+2, tHostId.end());

	EC_POINT_oct2point(group, point, &tHostId[0], tHostId.size(), 0);

	TCharArray tECDH(24);
	EC_KEY* pCopy = EC_KEY_dup(iECKey.GetECKey());

	ECDH_compute_key(&tECDH[0], 24, point, pCopy, 0);

	EC_POINT_free(point);
	EC_GROUP_free(group);
	EC_KEY_free(pCopy);

	return tECDH;
};
