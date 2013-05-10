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

#include "HipParameter.h"
#include "HipDefines.h"
#include <algorithm>

using namespace HDX;

MHipParameter::MHipParameter()
{
};

MHipParameter::~MHipParameter()
{
};

unsigned short MHipParameter::Length() const
{
	unsigned short tLength =
		HDX_SIZE_PARAM_HDR + Size();

	return (tLength + ((tLength % 8)
		? (8-(tLength % 8)) : 0));
};

bool MHipParameter::FromBytes(const TCharArray& aBytes, std::size_t aOffset)
{
	if ((aBytes.size()-aOffset) < HDX_SIZE_PARAM_HDR)
		return false;

	unsigned short tTemp = ((aBytes[aOffset]&0xFF)<<8)
		| (aBytes[aOffset+1]&0xFF);

	if (tTemp != Type()) return false;

	tTemp = (((aBytes[aOffset+2]&0xFF)<<8)
		|(aBytes[aOffset+3]&0xFF));

	unsigned short tFull =
		tTemp + HDX_SIZE_PARAM_HDR;

	tFull += ((tFull%8)?(8-(tFull%8)):0);

	if (tFull > (aBytes.size()-aOffset))
		return false;

	return SetBytes(aBytes,	aOffset
		+ HDX_SIZE_PARAM_HDR, tTemp);
};

TCharArray MHipParameter::ToBytes() const
{
	TCharArray tBytes1(Length());

	tBytes1[0] = (Type()>>8) & 0xFF;
	tBytes1[1] = Type() & 0xFF;

	tBytes1[2] = (Size()>>8) & 0xFF;
	tBytes1[3] = Size() & 0xFF;

	TCharArray tBytes2 = GetBytes();

	std::copy(&tBytes2[0], &tBytes2.back()+1,
		&tBytes1[HDX_SIZE_PARAM_HDR]);

	return tBytes1;
};
