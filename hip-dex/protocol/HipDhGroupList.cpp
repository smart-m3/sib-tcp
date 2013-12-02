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
#include "HipDhGroupList.h"

using namespace HDX;

CHipDhGroupList::CHipDhGroupList()
{
};

CHipDhGroupList::CHipDhGroupList(unsigned char aCipher)
	: iBytes(1, aCipher)
{
};

CHipDhGroupList::~CHipDhGroupList()
{
};

MHipParameter* CHipDhGroupList::Create()
{
	return new CHipDhGroupList;
};

unsigned short CHipDhGroupList::Type() const
{
	return HDX_TYPE_DH_GROUP_LIST;
};

unsigned short CHipDhGroupList::Size() const
{
	return iBytes.size();
};

TCharArray CHipDhGroupList::GetBytes() const
{
	return iBytes;
};

bool CHipDhGroupList::SetBytes(const TCharArray& aBytes)
{
	return SetBytes(aBytes, 0, aBytes.size());
};

bool CHipDhGroupList::SetBytes(const TCharArray& aBytes, std::size_t aOffset, std::size_t aLength)
{
	if (aBytes.empty()) return false;
	iBytes.assign(aBytes.begin()+aOffset, aBytes.begin()+aOffset+aLength);
	return true;
};

const TCharArray& CHipDhGroupList::GetList() const
{
	return iBytes;
};

void CHipDhGroupList::SetList(const TCharArray& aBytes)
{
	iBytes = aBytes;
};
