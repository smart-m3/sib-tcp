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
#include "HipSolution.h"

using namespace HDX;

CHipSolution::CHipSolution()
	: iComplexity(0), iOpaque(0), iRandomI(8), iSolutionJ(8)
{
};

CHipSolution::CHipSolution(unsigned char aComplexity,
	unsigned short aOpaque,	const TCharArray& aRandomI,
	const TCharArray& aSolutionJ) : iComplexity(aComplexity),
	iOpaque(aOpaque), iRandomI(aRandomI), iSolutionJ(aSolutionJ)
{
};

CHipSolution::~CHipSolution()
{
};

MHipParameter* CHipSolution::Create()
{
	return new CHipSolution;
};

unsigned short CHipSolution::Type() const
{
	return HDX_TYPE_SOLUTION;
};

unsigned short CHipSolution::Size() const
{
	return 4+iRandomI.size()+iSolutionJ.size();
};

TCharArray CHipSolution::GetBytes() const
{
	TCharArray tBytes(Size());

	tBytes[0] = iComplexity;
	tBytes[1] = 0x00;
	tBytes[2] = (iOpaque>>8)&0xFF;
	tBytes[3] = iOpaque&0xFF;

	std::copy(iRandomI.begin(),
		iRandomI.end(), &tBytes[4]);

	std::copy(iSolutionJ.begin(),
		iSolutionJ.end(), &tBytes[4]
		+ iRandomI.size());

	return tBytes;
};

bool CHipSolution::SetBytes(const TCharArray& aBytes)
{
	return SetBytes(aBytes, 0, aBytes.size());
};

bool CHipSolution::SetBytes(const TCharArray& aBytes, std::size_t aOffset, std::size_t aLength)
{
	if (aBytes.size()-aOffset < 4 || aLength < 4)
		return false;

	iComplexity = aBytes[aOffset];
	iOpaque = ((aBytes[aOffset+2]&0xFF)<<8)
		| (aBytes[aOffset+3]&0xFF);

	iRandomI.assign(&aBytes[aOffset+4],
		&aBytes[aOffset+4+((aLength-4)>>1)]);

	iSolutionJ.assign(&aBytes[aOffset+4+((aLength-4)>>1)],
		&aBytes[aOffset+aLength]);

	return true;
};

unsigned char CHipSolution::GetComplexity() const
{
	return iComplexity;
};

void CHipSolution::SetComplexity(unsigned char aComplexity)
{
	iComplexity = aComplexity;
};

unsigned short CHipSolution::GetOpaque() const
{
	return iOpaque;
};

void CHipSolution::SetOpaque(unsigned short aOpaque)
{
	iOpaque = aOpaque;
};

const TCharArray& CHipSolution::GetRandomI() const
{
	return iRandomI;
};

void CHipSolution::SetRandomI(const TCharArray& aRandomI)
{
	iRandomI = aRandomI;
};

const TCharArray& CHipSolution::GetSolutionJ() const
{
	return iSolutionJ;
};

void CHipSolution::SetSolutionJ(const TCharArray& aSolutionJ)
{
	iSolutionJ = aSolutionJ;
};
