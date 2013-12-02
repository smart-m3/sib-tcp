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

#include "Thread.h"
#include <cstdio>
#include <cerrno>
#include <cstring>

using namespace HDX;

MThread::MThread() : iThreadId(0)
{
};

MThread::~MThread()
{
};

bool MThread::operator==(const MThread& aThrd) const
{
	return (pthread_equal(iThreadId, aThrd.iThreadId) != 0);
};

bool MThread::operator!=(const MThread& aThrd) const
{
	return !operator==(aThrd);
};

void* MThread::ThreadFunction(void* aPtr)
{
	MThread* pThrd = reinterpret_cast<MThread*>(aPtr);

	if (pThrd)
		pThrd->Run();

	return NULL;
};

void MThread::Start() throw (MException)
{
	int result = 0;

	if ((result = pthread_create(&iThreadId, NULL,
		&(MThread::ThreadFunction), this)) != 0)
	{
		throw CSyncException("Failed to start thread. Reason: "
			+ std::string(strerror(result)));
	}
};

void MThread::Stop() throw (MException)
{
	int result = 0;

	if ((result = pthread_cancel(iThreadId)) != 0)
	{
		throw CSyncException("Failed to cancel thread. Reason: "
			+ std::string(strerror(result)));
	}
};

void MThread::Join() throw (MException)
{
	int status = 0;
	int result = 0;

	if ((result = pthread_join(iThreadId, (void**)&status)) != 0)
	{
		throw CSyncException("Failed to join thread. Reason: "
			+ std::string(strerror(result)));
	}
};

int MThread::Self() const
{
	return (int)iThreadId;
};
