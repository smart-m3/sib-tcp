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
#include "HipPuzzle.h"
#include <iostream>

using namespace HDX;

CHipPuzzle::CHipPuzzle()
	: iComplexity(0), iLifetime(0), iOpaque(0), iRandomI(8)
{
};

CHipPuzzle::CHipPuzzle(unsigned char aComplexity,
	unsigned char aLifetime, unsigned short aOpaque,
	const TCharArray& aRandomI) : iComplexity(aComplexity),
	iLifetime(aLifetime), iOpaque(aOpaque), iRandomI(aRandomI)
{
};

CHipPuzzle::~CHipPuzzle()
{
};

MHipParameter* CHipPuzzle::Create()
{
	return new CHipPuzzle;
};

unsigned short CHipPuzzle::Type() const
{
	return HDX_TYPE_PUZZLE;
};

unsigned short CHipPuzzle::Size() const
{
	return 4+iRandomI.size();
};

TCharArray CHipPuzzle::GetBytes() const
{
	TCharArray tBytes(Size());

	tBytes[0] = iComplexity;
	tBytes[1] = iLifetime;
	tBytes[2] = (iOpaque>>8)&0xFF;
	tBytes[3] = iOpaque&0xFF;

	std::copy(iRandomI.begin(),
		iRandomI.end(), &tBytes[4]);

	return tBytes;
};

bool CHipPuzzle::SetBytes(const TCharArray& aBytes)
{
	return SetBytes(aBytes, 0, aBytes.size());
};

bool CHipPuzzle::SetBytes(const TCharArray& aBytes, std::size_t aOffset, std::size_t aLength)
{
	if (aBytes.size()-aOffset < 4 || aLength < 4)
		return false;

	iComplexity = aBytes[aOffset];
	iLifetime = aBytes[aOffset+1];
	iOpaque = ((aBytes[aOffset+2]&0xFF)<<8)
		|(aBytes[aOffset+3]&0xFF);

	iRandomI.assign(&aBytes[aOffset+4],
		&aBytes[aOffset+aLength]);

	return true;
};

unsigned char CHipPuzzle::GetComplexity() const
{
	return iComplexity;
};

void CHipPuzzle::SetComplexity(unsigned char aComplexity)
{
	iComplexity = aComplexity;
};

unsigned char CHipPuzzle::GetLifetime() const
{
	return iLifetime;
};

void CHipPuzzle::SetLifetime(unsigned char aLifetime)
{
	iLifetime = aLifetime;
};

unsigned short CHipPuzzle::GetOpaque() const
{
	return iOpaque;
};

void CHipPuzzle::SetOpaque(unsigned short aOpaque)
{
	iOpaque = aOpaque;
};

const TCharArray& CHipPuzzle::GetRandomI() const
{
	return iRandomI;
};

void CHipPuzzle::SetRandomI(const TCharArray& aRandomI)
{
	iRandomI = aRandomI;
};
