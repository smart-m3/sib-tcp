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
#include "HipEncrypted.h"

using namespace HDX;

CHipEncrypted::CHipEncrypted() : iIV(8)
{
};

CHipEncrypted::CHipEncrypted(const TCharArray& aIV, const TCharArray& aData)
	: iIV(aIV), iData(aData)
{
};

CHipEncrypted::~CHipEncrypted()
{
};

MHipParameter* CHipEncrypted::Create()
{
	return new CHipEncrypted;
};

unsigned short CHipEncrypted::Type() const
{
	return HDX_TYPE_ENCRYPTED;
};

unsigned short CHipEncrypted::Size() const
{
	return iIV.size()+iData.size();
};

TCharArray CHipEncrypted::GetBytes() const
{
	TCharArray tBytes(iIV.size()+iData.size());

	std::copy(&iIV[0], &iIV[8], &tBytes[0]);

	std::copy(iData.begin(), iData.end(),
		&tBytes[0]+iIV.size());

	return tBytes;
};

bool CHipEncrypted::SetBytes(const TCharArray& aBytes)
{
	return SetBytes(aBytes, 0, aBytes.size());
};

bool CHipEncrypted::SetBytes(const TCharArray& aBytes, std::size_t aOffset, std::size_t aLength)
{
	if (aBytes.size() < 8) return false;

	iIV.assign(&aBytes[0], &aBytes[8]);
	iData.assign(&aBytes[8], &aBytes.back());

	return true;
};

TCharArray CHipEncrypted::GetIV() const
{
	return iIV;
};

void CHipEncrypted::SetIV(const TCharArray& aIV)
{
	iIV = aIV;
};

TCharArray CHipEncrypted::GetData() const
{
	return iData;
};

void CHipEncrypted::SetData(const TCharArray& aData)
{
	iData = aData;
};
