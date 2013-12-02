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
#include "HipR1Counter.h"

using namespace HDX;

CHipR1Counter::CHipR1Counter()
	: iCounter(8)
{
};

CHipR1Counter::CHipR1Counter(const TCharArray& aCounter)
	: iCounter(aCounter)
{
};

CHipR1Counter::~CHipR1Counter()
{
};

MHipParameter* CHipR1Counter::Create()
{
	return new CHipR1Counter;
};

unsigned short CHipR1Counter::Type() const
{
	return HDX_TYPE_R1_COUNTER;
};

unsigned short CHipR1Counter::Size() const
{
	return 8;
};

TCharArray CHipR1Counter::GetBytes() const
{
	return iCounter;
};

bool CHipR1Counter::SetBytes(const TCharArray& aBytes)
{
	return SetBytes(aBytes, 0, aBytes.size());
};

bool CHipR1Counter::SetBytes(const TCharArray& aBytes, std::size_t aOffset, std::size_t aLength)
{
	if (aBytes.size() != 8)	return false;

	iCounter = aBytes;
	return true;
};

const TCharArray& CHipR1Counter::GetCounter() const
{
	return iCounter;
};

void CHipR1Counter::SetCounter(const TCharArray& aCounter)
{
	iCounter = aCounter;
};
