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

#include "AesCmac.h"

using namespace HDX;

CAesCmac::CAesCmac(TAesKeyLength aLength, const TCharArray& aKey)
	: iContext(CMAC_CTX_new())
{
	switch (aLength)
	{
		case tKey128:
			iCipher = EVP_aes_128_cbc();
			break;
		case tKey192:
			iCipher = EVP_aes_192_cbc();
			break;
		case tKey256:
			iCipher = EVP_aes_256_cbc();
			break;
		default:
			iCipher = 0;
	};

	if (iContext && (aKey.size() == (unsigned)aLength))
		CMAC_Init(iContext, &aKey[0], aLength, iCipher, 0);
};

CAesCmac::~CAesCmac()
{
	if (iContext)
		CMAC_CTX_free(iContext);
};

void CAesCmac::UpdateBlock(const TCharArray& aBlock)
{
	CMAC_Update(iContext, &aBlock[0], aBlock.size());
};

void CAesCmac::UpdateBlock(unsigned char* aBlock, std::size_t aLen)
{
	CMAC_Update(iContext, aBlock, aLen);
};

TCharArray CAesCmac::CalculateCmac() const
{
	std::size_t tSize = iCipher->block_size;
	TCharArray tArray(tSize);

	CMAC_Final(iContext, &tArray[0], &tSize);
	return tArray;
};

TCharArray CAesCmac::RandomKey() const
{
	TCharArray tKey(iCipher->key_len);
	RAND_bytes(&tKey[0], tKey.size());
	return tKey;
};
