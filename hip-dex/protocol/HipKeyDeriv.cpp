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

#include "HipKeyDeriv.h"
#include "AesCmac.h"

using namespace HDX;

CHipKeyDeriv::CHipKeyDeriv(std::size_t aEKLen, std::size_t aIKLen)
{
	iIEK.reserve(aEKLen);
	iIIK.reserve(aIKLen);
	iREK.reserve(aEKLen);
	iRIK.reserve(aIKLen);
};

CHipKeyDeriv::~CHipKeyDeriv()
{
};

void CHipKeyDeriv::DeriveKeys(const TCharArray& aHitI,
	const TCharArray& aHitR, const TCharArray& iIarr,
	const TCharArray& input)
{
	unsigned char buff[] = "CKDF-Extract";
	TCharArray tStr(buff, buff+12);

	CAesCmac tAes(tKey128, iIarr);
	tAes.UpdateBlock(input);

	if (aHitI < aHitR)
	{
		tAes.UpdateBlock(aHitI);
		tAes.UpdateBlock(aHitR);
	}
	else
	{
		tAes.UpdateBlock(aHitR);
		tAes.UpdateBlock(aHitI);
	}

	tAes.UpdateBlock(tStr);
	TCharArray tNew = tAes.CalculateCmac();

	TCharArray keymat;
	keymat.reserve(2*iIEK.capacity()+2*iIIK.capacity());
	TCharArray rrr;

	std::size_t offset = 0;
	for (std::size_t i = 0; offset < keymat.capacity(); ++i)
	{
		CAesCmac tAes2(tKey128, tNew);
		tAes2.UpdateBlock(rrr);

		if (aHitI < aHitR)
			{
				tAes2.UpdateBlock(aHitI);
				tAes2.UpdateBlock(aHitR);
			}
			else
			{
				tAes2.UpdateBlock(aHitR);
				tAes2.UpdateBlock(aHitI);
			}

		unsigned char buff2[] = "CKDF-Expand";
		TCharArray tStr2(buff2, buff2+11);
		tAes2.UpdateBlock(tStr2);

		// FIXME: check byte order; now machine specific
		tAes2.UpdateBlock((unsigned char*)&i, 1);
		rrr = tAes2.CalculateCmac();

		keymat.insert(keymat.end(), rrr.begin(), rrr.end());
		offset += rrr.size();
	}

	if (aHitI > aHitR)
	{
		iIEK.insert(iIEK.end(), keymat.begin(), keymat.begin()+16);
		iIIK.insert(iIIK.end(), keymat.begin()+16, keymat.begin()+32);
		iREK.insert(iREK.end(), keymat.begin()+32, keymat.begin()+48);
		iRIK.insert(iRIK.end(), keymat.begin()+48, keymat.end());
	}
	else
	{
		iREK.insert(iREK.end(), keymat.begin(), keymat.begin()+16);
		iRIK.insert(iRIK.end(), keymat.begin()+16, keymat.begin()+32);
		iIEK.insert(iIEK.end(), keymat.begin()+32, keymat.begin()+48);
		iIIK.insert(iIIK.end(), keymat.begin()+48, keymat.end());
	}
};
