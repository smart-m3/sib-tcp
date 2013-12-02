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

#include "HipDefines.h"
#include "HipHostId.h"
#include <iostream>

using namespace HDX;

CHipHostId::CHipHostId() : iDiType(tNone)
{
};

CHipHostId::CHipHostId(TDIType aDiType, const TCharArray& aHostId, const TCharArray& aDomainId)
	: iDiType(aDiType&0x0F), iDomainId(aDomainId), iHostId(aHostId)
{
};

CHipHostId::~CHipHostId()
{
};

MHipParameter* CHipHostId::Create()
{
	return new CHipHostId;
};

unsigned short CHipHostId::Type() const
{
	return HDX_TYPE_HOST_ID;
};

unsigned short CHipHostId::Size() const
{
	return 6+iHostId.size()+iDomainId.size();
};

TCharArray CHipHostId::GetBytes() const
{
	TCharArray tBytes(Size());

	tBytes[0] = (iHostId.size()>>8) & 0xFF;
	tBytes[1] = iHostId.size() & 0xFF;

	tBytes[2] = ((iDiType<<4) & 0xF0) |
		((iDomainId.size()>>8)&0x0F);
	tBytes[3] = iDomainId.size() & 0xFF;

	tBytes[4] = 0x00;
	tBytes[5] = ALGORITHM_ECDH;

	std::copy(iHostId.begin(), iHostId.end(), &tBytes[6]);
	std::copy(iDomainId.begin(), iDomainId.end(),
		&tBytes[6]+iHostId.size());

	return tBytes;
};

bool CHipHostId::SetBytes(const TCharArray& aBytes)
{
	return SetBytes(aBytes, 0, aBytes.size());
};

bool CHipHostId::SetBytes(const TCharArray& aBytes, std::size_t aOffset, std::size_t aLength)
{
	if (aBytes.size()-aOffset < 6) return false;

	std::size_t tHiLen = ((aBytes[aOffset]&0xFF)<<8)|(aBytes[aOffset+1]&0xFF);
	std::size_t tDiLen = ((aBytes[aOffset+2]&0x0F)<<8)|(aBytes[aOffset+3]&0xFF);
	std::size_t tAlgo = ((aBytes[aOffset+4]&0xFF)<<8)|(aBytes[aOffset+5]&0xFF);

	if (6+tHiLen+tDiLen > aBytes.size()) return false;
	if (tAlgo != ALGORITHM_ECDH) return false;

	iDiType = (aBytes[2] >> 4) & 0x0F;

	iHostId.assign(&aBytes[aOffset+6], &aBytes[aOffset+6+tHiLen]);
	iDomainId.assign(&aBytes[aOffset+6+tHiLen], &aBytes[aOffset+aLength]);

	return true;
};

unsigned char CHipHostId::GetDiType() const
{
	return iDiType;
};

void CHipHostId::SetDiType(unsigned char aDiType)
{
	iDiType = aDiType & 0x0F;
};

const TCharArray& CHipHostId::GetHostId() const
{
	return iHostId;
};

void CHipHostId::SetHostId(const TCharArray& aHostId)
{
	iHostId = aHostId;
};

const TCharArray& CHipHostId::GetDomainId() const
{
	return iDomainId;
};

void CHipHostId::SetDomainId(const TCharArray& aDomainId)
{
	iDomainId = aDomainId;
};
