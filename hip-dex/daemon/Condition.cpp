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

#include "Condition.h"
#include <cstring>

using namespace HDX;

CCondition::CCondition()
{
	pthread_condattr_t tAttr;
	std::memset(&tAttr, 0, sizeof(tAttr));

	int result = 0;

	if ((result = pthread_condattr_init(&tAttr)) != 0)
	{
		throw CSyncException("Could not initialize condition attribute. Reason: "
			+ std::string(strerror(result)));
	}

	if ((result = pthread_cond_init(&iCondition, &tAttr)) != 0)
	{
		throw CSyncException("Could not initialize condition variable. Reason: "
			+ std::string(strerror(result)));
	}

	if ((result = pthread_condattr_destroy(&tAttr)) != 0)
	{
		throw CSyncException("Could not destroy condition attribute. Reason: "
			+ std::string(strerror(result)));
	}
};

CCondition::~CCondition()
{
	int retry = 10;

	while ((pthread_cond_destroy(&iCondition) == EBUSY) && (retry >= 0))
	{
		--retry;
		iMutex.Lock();
		NotifyAll();
		iMutex.Unlock();
	}
};

void CCondition::Wait()
{
	pthread_cond_wait(&iCondition, iMutex);
};

void CCondition::Notify()
{
	pthread_cond_signal(&iCondition);
};

void CCondition::NotifyAll()
{
	pthread_cond_broadcast(&iCondition);
};

void CCondition::Lock()
{
	iMutex.Lock();
};

void CCondition::Unlock()
{
	iMutex.Unlock();
};
