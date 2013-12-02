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
#include "HipHitSuiteList.h"
#include <iostream>
using namespace HDX;

CHipHitSuiteList::CHipHitSuiteList()
{
};

CHipHitSuiteList::CHipHitSuiteList(const TCharArray& aList)
	: iBytes(aList)
{
};

CHipHitSuiteList::~CHipHitSuiteList()
{
};

MHipParameter* CHipHitSuiteList::Create()
{
	return new CHipHitSuiteList;
};

unsigned short CHipHitSuiteList::Type() const
{
	return HDX_TYPE_HIT_SUITE_LIST;
};

unsigned short CHipHitSuiteList::Size() const
{
	return iBytes.size();
};

TCharArray CHipHitSuiteList::GetBytes() const
{
	return iBytes;
};

bool CHipHitSuiteList::SetBytes(const TCharArray& aBytes)
{
	return SetBytes(aBytes, 0, aBytes.size());
};

bool CHipHitSuiteList::SetBytes(const TCharArray& aBytes, std::size_t aOffset, std::size_t aLength)
{
	if (aLength != 1) return false;
	iBytes.assign(&aBytes[aOffset], &aBytes[aOffset]+aLength);
	return true;
};
