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

#include "AesCbc.h"
#include <iostream>

using namespace HDX;

CAesCbc::CAesCbc(TAesKeyLength aLength, const TCharArray& aKey, const TCharArray& aIV)
	: iIV(aIV)
{
	if ((unsigned)aLength == aKey.size())
	{
		AES_set_encrypt_key(&aKey[0], 8*aLength, &iKeyE);
		AES_set_decrypt_key(&aKey[0], 8*aLength, &iKeyD);
	}
};

CAesCbc::~CAesCbc()
{
};

TCharArray CAesCbc::Encrypt(const TCharArray& aPlain)
{
	TCharArray tCipher(aPlain.size());
	AES_cbc_encrypt(&aPlain[0], &tCipher[0],
		aPlain.size(), &iKeyE, &iIV[0], 1);
	return tCipher;
};

TCharArray CAesCbc::Decrypt(const TCharArray& aCipher)
{
	TCharArray tPlain(aCipher.size());
	AES_cbc_encrypt(&aCipher[0], &tPlain[0],
		aCipher.size(), &iKeyD, &iIV[0], 0);
	return tPlain;
};
