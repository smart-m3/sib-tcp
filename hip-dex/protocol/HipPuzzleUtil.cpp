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

#include "HipPuzzleUtil.h"
#include "HipUtils.h"
#include "AesCmac.h"

using namespace HDX;

CHipPuzzleUtil::CHipPuzzleUtil(unsigned int aComplexity)
	: iComplexity(aComplexity), iRound(0)
{
	// TODO: Initialize AES key pool here!
};

CHipPuzzleUtil::~CHipPuzzleUtil()
{
};

TCharArray CHipPuzzleUtil::CalculateI(const TCharArray& aHitI, const TCharArray& aHitR,
	const TCharArray& aLocalAddress, const TCharArray& aRemoteAddress)
{
	// TODO: Remove this hardcoded key and implement key generation!
	unsigned char aes_key_128[16] = {
		0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
		0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
	};

	TCharArray tKey(&aes_key_128[0], &aes_key_128[0]+16);
	CAesCmac tAes(tKey128, tKey);

	tAes.UpdateBlock(aHitI);
	tAes.UpdateBlock(aHitR);
	tAes.UpdateBlock(aLocalAddress);
	tAes.UpdateBlock(aRemoteAddress);

	return tAes.CalculateCmac();
};

TCharArray CHipPuzzleUtil::SolveR(const TCharArray& aPuzzleI, const TCharArray& aHitI,
	const TCharArray& aHitR, unsigned int aComplexity)
{
	TCharArray tSolution;

	while (true)
	{
		CAesCmac tAes(tKey128, aPuzzleI);
		tSolution = tAes.RandomKey();

		tAes.UpdateBlock(aHitI);
		tAes.UpdateBlock(aHitR);
		tAes.UpdateBlock(tSolution);

		TCharArray tVerify =
			::hdx_ltrunc(tAes.CalculateCmac(), aComplexity);

		bool tVerifyOk = true;

		for (std::size_t i = 0; i < tVerify.size(); ++i)
		{
			if (tVerify[i] != 0)
			{
				tVerifyOk = false;
				break;
			}
		}

		if (tVerifyOk) break;
	}

	return tSolution;
};

bool CHipPuzzleUtil::VerifyPuzzle(const TCharArray& aPuzzleI, const TCharArray& aPuzzleR,
	const TCharArray& aHitI, const TCharArray& aHitR, const TCharArray& aLocalAddress,
	const TCharArray& aRemoteAddress)
{
	TCharArray tCorrectI = CalculateI(aHitI, aHitR, aLocalAddress, aRemoteAddress);

	if (tCorrectI != aPuzzleI) return false;

	CAesCmac tAes(tKey128, tCorrectI);

	tAes.UpdateBlock(aHitI);
	tAes.UpdateBlock(aHitR);
	tAes.UpdateBlock(aPuzzleR);

	TCharArray tVerify =
		hdx_ltrunc(tAes.CalculateCmac(), iComplexity);

	for (std::size_t i = 0; i < tVerify.size(); ++i)
		if (tVerify[i] != 0) return false;

	return true;
};
