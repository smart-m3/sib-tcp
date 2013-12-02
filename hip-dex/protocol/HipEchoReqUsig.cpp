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
#include "HipEchoReqUsig.h"

using namespace HDX;

CHipEchoReqUsig::CHipEchoReqUsig()
{
};

CHipEchoReqUsig::CHipEchoReqUsig(const TCharArray& aData)
	: iData(aData)
{
};

CHipEchoReqUsig::~CHipEchoReqUsig()
{
};

MHipParameter* CHipEchoReqUsig::Create()
{
	return new CHipEchoReqUsig;
};

unsigned short CHipEchoReqUsig::Type() const
{
	return HDX_TYPE_ECHO_REQ_USIG;
};

unsigned short CHipEchoReqUsig::Size() const
{
	return iData.size();
};

TCharArray CHipEchoReqUsig::GetBytes() const
{
	return iData;
};

bool CHipEchoReqUsig::SetBytes(const TCharArray& aBytes)
{
	return SetBytes(aBytes, 0, aBytes.size());
};

bool CHipEchoReqUsig::SetBytes(const TCharArray& aBytes, std::size_t aOffset, std::size_t aLength)
{
	if (aBytes.empty()) return false;
	iData = aBytes;
	return true;
};

TCharArray CHipEchoReqUsig::GetData() const
{
	return iData;
};

void CHipEchoReqUsig::SetData(const TCharArray& aData)
{
	iData = aData;
};
