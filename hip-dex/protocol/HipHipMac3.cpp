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
#include "HipHipMac3.h"
#include <iostream>

using namespace HDX;

CHipHipMac3::CHipHipMac3()
{
};

CHipHipMac3::CHipHipMac3(const TCharArray& aData)
	: iData(aData)
{
};

CHipHipMac3::~CHipHipMac3()
{
};

MHipParameter* CHipHipMac3::Create()
{
	return new CHipHipMac3;
};

unsigned short CHipHipMac3::Type() const
{
	return HDX_TYPE_HIP_MAC_3;
};

unsigned short CHipHipMac3::Size() const
{
	return iData.size();
};

TCharArray CHipHipMac3::GetBytes() const
{
	return iData;
};

bool CHipHipMac3::SetBytes(const TCharArray& aBytes)
{
	return SetBytes(aBytes, 0, aBytes.size());
};

bool CHipHipMac3::SetBytes(const TCharArray& aBytes, std::size_t aOffset, std::size_t aLength)
{
	if (aBytes.size() < aOffset+aLength)
		return false;

	iData.assign(&aBytes[aOffset], &aBytes[aOffset]+aLength);
	return true;
};

TCharArray CHipHipMac3::GetData() const
{
	return iData;
};

void CHipHipMac3::SetData(const TCharArray& aData)
{
	iData = aData;
};
