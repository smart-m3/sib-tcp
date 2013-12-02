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
#include "HipUtils.h"
#include "HipHipMac3.h"
#include "HipPacket.h"
#include "AesCmac.h"
#include <algorithm>
#include <cstring>
#include <iostream>

using namespace HDX;

const CHipParamFactory sParamFactory;

MHipPacket::MHipPacket(unsigned char aType, const TCharArray& aSendHit, const TCharArray& aRecvHit)
	: iNextHdr(HDX_IPPROTO_NONE), iType(aType), iVersion(2), iCtrls(0), iSenderHit(aSendHit), iReceiverHit(aRecvHit)
{
};

MHipPacket::~MHipPacket()
{
	std::for_each(iParams.begin(),
		iParams.end(), FDelete<MHipParameter>());
};

bool MHipPacket::FromBytes(TCharArray& aBytes)
{
	unsigned short tChecksum =
		(((aBytes[4]&0xFF)<<8)|(aBytes[5]&0xFF));

	aBytes[4] = 0;
	aBytes[5] = 0;

	if (tChecksum != CalcChecksum(aBytes))
		return false;

	iNextHdr = aBytes[0];
	iVersion = ((aBytes[3]>>4)&0x0F);
	iCtrls = (((aBytes[6]&0xFF)<<8)|(aBytes[7]&0xFF));

	std::memcpy(&iSenderHit[0], &aBytes[8], 16);
	std::memcpy(&iReceiverHit[0], &aBytes[24], 16);

	std::size_t leng = 40;

	while (aBytes.size()-leng > 0)
	{
		MHipParameter* pParam =
			sParamFactory.Create(aBytes,
			leng, aBytes.size()-leng);

		if (pParam == NULL)
			return false;

		iParams.push_back(pParam);
		leng += pParam->Length();
	}

	return true;
};

TCharArray MHipPacket::ToBytes() const
{
	std::size_t tLength = 40;

	for (std::size_t i = 0; i < iParams.size(); ++i)
		tLength += iParams[i]->Length();

	TCharArray tBytes;
	tBytes.reserve(tLength);

	tBytes.push_back(iNextHdr);
	tBytes.push_back((tLength-8)>>3);
	tBytes.push_back(iType & 0x7F);
	tBytes.push_back(((iVersion&0xFF)<<4)|0x01);

	tBytes.push_back(0);
	tBytes.push_back(0);

	tBytes.push_back((iCtrls>>8) & 0xFF);
	tBytes.push_back(iCtrls & 0xFF);

	tBytes.insert(tBytes.end(), &iSenderHit[0], &iSenderHit[0]+16);
	tBytes.insert(tBytes.end(), &iReceiverHit[0], &iReceiverHit[0]+16);

	for (std::size_t i = 0; i < iParams.size(); ++i)
	{
		TCharArray tArray = iParams[i]->ToBytes();
		tBytes.insert(tBytes.end(), tArray.begin(),
			tArray.end());
	}

	unsigned short tChecksum = CalcChecksum(tBytes);

	tBytes[4] = (tChecksum>>8) & 0xFF;
	tBytes[5] = tChecksum & 0xFF;

	return tBytes;
};

unsigned short MHipPacket::CalcChecksum(const TCharArray& aBytes) const
{
	unsigned int tChecksum = 0;

	for (std::size_t i = 0; i < aBytes.size(); ++i)
	{
		tChecksum += ((i%2==0) ? (aBytes[i]<<8) : aBytes[i]);
	}

	while ((tChecksum>>16) != 0)
		tChecksum = (tChecksum&0xFFFF)+(tChecksum>>16);

	return ~tChecksum;
};

TCharArray MHipPacket::CalculateCmac(const TCharArray& aKey) const
{
	std::size_t tLength = 0;

	for (std::size_t i = 0; i < iParams.size(); ++i)
	{
		if (iParams[i]->Type() >= HDX_TYPE_HIP_MAC_3)
			continue;

		tLength += iParams[i]->Length();
	}

	TCharArray tBytes(40);

	tBytes[0] = iNextHdr;
	tBytes[1] = ((tLength-8)>>3);
	tBytes[2] = iType & 0x7F;
	tBytes[3] = ((iVersion&0xFF)<<4)|0x01;
	tBytes[6] = (iCtrls>>8) & 0xFF;
	tBytes[7] = iCtrls & 0xFF;

	std::memcpy(&tBytes[8], &iSenderHit[0], 16);
	std::memcpy(&tBytes[24], &iReceiverHit[0], 16);

	CAesCmac tCmac(tKey128, aKey);
	tCmac.UpdateBlock(tBytes);

	for (std::size_t i = 0; i < iParams.size(); ++i)
	{
		if (iParams[i]->Type() >= HDX_TYPE_HIP_MAC_3)
			continue;

		tCmac.UpdateBlock(iParams[i]->ToBytes());
	}

	return tCmac.CalculateCmac();
};

unsigned int MHipPacket::Length() const
{
	std::size_t tLength = 40;

	for (std::size_t i = 0; i < iParams.size(); ++i)
		tLength += iParams[i]->Length();

	return tLength;
};

unsigned char MHipPacket::Type() const
{
	return iType;
};

void MHipPacket::AddParameter(MHipParameter* aParam)
{
	iParams.push_back(aParam);
};

MHipParameter* MHipPacket::GetParameter(unsigned short aType)
{
	for (std::size_t i = 0; i < iParams.size(); ++i)
		if (iParams[i]->Type() == aType)
			return iParams[i];

	return NULL;
};

bool MHipPacket::VerifyCmac(const TCharArray& aKey)
{
	MHipParameter* pParam = GetParameter(HDX_TYPE_HIP_MAC_3);

	if (pParam)
	{
		CHipHipMac3* pMac = reinterpret_cast<CHipHipMac3*>(pParam);
		if (pMac) return CalculateCmac(aKey) == pMac->GetData();
	}

	return false;
};

TCharArray MHipPacket::GetSenderHit() const
{
	return iSenderHit;
}

void MHipPacket::SetSenderHit(const TCharArray& aSenderHit)
{
	iSenderHit = aSenderHit;
};

TCharArray MHipPacket::GetReceiverHit() const
{
	return iReceiverHit;
}

void MHipPacket::SetReceiverHit(const TCharArray& aReceiverHit)
{
	iReceiverHit = aReceiverHit;
};
