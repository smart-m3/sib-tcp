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

#include "HipECKey.h"
#include "HipDefines.h"
#include <iostream>

using namespace HDX;

const unsigned char cHitPrefix[4] = {0x20, 0x01, 0x00, 0x15};

CHipECKey::CHipECKey(TECType aCurve, const TCharArray& aPriv, const TCharArray& aPub)
{
	if ((iKey = EC_KEY_new_by_curve_name(aCurve)))
	{
		EC_GROUP* pGroup = EC_GROUP_new_by_curve_name(aCurve);

		BIGNUM* pX = BN_new();
		EC_POINT* pY = EC_POINT_new(pGroup);

		EC_POINT_oct2point(pGroup, pY, &aPub[0], aPub.size(), 0);
		BN_bin2bn(&aPriv[0], aPriv.size(), pX);

		EC_KEY_set_private_key(iKey, pX);
		EC_KEY_set_public_key(iKey, pY);

		EC_GROUP_free(pGroup);
		EC_POINT_free(pY);
		BN_free(pX);

		// TODO: Add error handling here!
		// if (EC_KEY_check_key(iKey) != 1)
	}
};

CHipECKey::CHipECKey(TECType aCurve)
{
	if ((iKey = EC_KEY_new_by_curve_name(aCurve)))
	{
		EC_KEY_generate_key(iKey);
	}
};

CHipECKey::~CHipECKey()
{
	if (iKey)
		EC_KEY_free(iKey);
};

TCharArray CHipECKey::ToHostId() const
{
	const EC_POINT* apub = EC_KEY_get0_public_key(iKey);
	const EC_GROUP* group = EC_KEY_get0_group(iKey);

	TCharArray tPub(EC_POINT_point2oct(group, apub,
		POINT_CONVERSION_UNCOMPRESSED, 0, 0, 0)+2);

	if (tPub.size() == 40+3)
		tPub[1] = CURVE_SECP160R1;
	else if (tPub.size() == 48+3)
		tPub[1] = CURVE_SECP192R1;
	else if (tPub.size() == 56+3)
		tPub[1] = CURVE_SECP224R1;

	EC_POINT_point2oct(group, apub,
		POINT_CONVERSION_UNCOMPRESSED,
		&tPub[2], tPub.size()-2, 0);

	return tPub;
};

TCharArray CHipECKey::ToHit() const
{
	const EC_POINT* tPoint = EC_KEY_get0_public_key(iKey);
	const EC_GROUP* tGroup = EC_KEY_get0_group(iKey);

	TCharArray tPub(EC_POINT_point2oct(tGroup, tPoint,
		POINT_CONVERSION_UNCOMPRESSED, 0, 0, 0));

	EC_POINT_point2oct(tGroup, tPoint,
		POINT_CONVERSION_UNCOMPRESSED,
		&tPub[0], tPub.size(), 0);

	TCharArray tHit(HDX_LENGTH_HIT);
	std::copy(cHitPrefix, cHitPrefix+4, &tHit[0]);
	std::copy(&tPub[1], &tPub[13], &tHit[4]);

	return tHit;
};
