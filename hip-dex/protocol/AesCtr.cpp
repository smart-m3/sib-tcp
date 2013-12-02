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

#include "AesCtr.h"
#include <iostream>

using namespace HDX;

CAesCtr::CAesCtr(TAesKeyLength aLength, const TCharArray& aKey, const TCharArray& aIV)
{
	if ((unsigned)aLength == aKey.size())
	{
		AES_set_encrypt_key(&aKey[0], 8*aLength, &iKeyE);
		iBeg = iEnd = iCur = 0;
		iCounter = aIV;
		int i;
		for (i=1; i <= 4; i++) iCounter[iCounter.size() - i] = 0x0;
		Generate();
		std::cout << "Key\n";
		for (i = 0; i < aKey.size(); i++) std::cout << (int)aKey[i] << " ";
		std::cout << "\n";
		std::cout << "Counter\n";
		for (i = 0; i < iCounter.size(); i++) std::cout << (int)iCounter[i] << " ";
		std::cout << "\n";
	}
	
};

CAesCtr::~CAesCtr()
{
};

TCharArray CAesCtr::Encrypt(const TCharArray& aPlain)
{
	TCharArray tCipher(aPlain.size());
	for (int i = 0; i < aPlain.size(); i++) {
	    tCipher[i] = aPlain[i] ^ iCipher[iCur++];
	    if (iCur == iCipher.size()) 
		Generate();
	}
	return tCipher;
};


void CAesCtr::Generate()
{
    int i = 1;
    while (i <= 4) {
        if (iCounter[iCounter.size() - i] == 0xFF) {
    	    iCounter[iCounter.size() - i] = 0x0;
    	    i++;
    	    continue;
        }
        iCounter[iCounter.size() - i]++;
	break;
    }
    iCipher.resize(iCounter.size());
    AES_encrypt(&iCounter[0], &iCipher[0], &iKeyE);
    iBeg = iEnd;
    iEnd = iBeg + iCounter.size();
    iCur = 0;
}
