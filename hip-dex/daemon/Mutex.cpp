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

#include "Mutex.h"
#include <cstring>

using namespace HDX;

CMutex::CMutex()
	throw (CSyncException)
{
	pthread_mutexattr_t attr;
	memset(&attr, 0, sizeof(attr));

	int result = 0;

	if ((result = pthread_mutexattr_init(&attr)) != 0)
	{
		throw CSyncException("Could not init mutex attribute. Reason: "
			+ std::string(strerror(result)));
   }

	if ((result = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE)) != 0)
	{
		throw CSyncException("Could not set mutext attribute. Reason: "
			+ std::string(strerror(result)));
   }

	if ((result = pthread_mutex_init(&iMutex, &attr)) != 0)
	{
		throw CSyncException("Could not initialize mutex variable. Reason: "
			+ std::string(strerror(result)));
   }
};

CMutex::~CMutex()
{
   int result = 0;
   
   if ((result = pthread_mutex_destroy(&iMutex)) != 0)
   {
		throw CSyncException("Could not destroy mutex variable. Reason: "
			+ std::string(strerror(result)));
	}
};

void CMutex::Lock()
	throw (CSyncException)
{
	int result = 0;

	if ((result = pthread_mutex_lock(&iMutex)) != 0)
	{
		throw CSyncException("Could not lock mutex. Reason: "
			+ std::string(strerror(result)));
	}
};

void CMutex::TryLock()
	throw (CSyncException)
{
	int result = 0;

	if ((result = pthread_mutex_trylock(&iMutex)) != 0)
	{
		throw CSyncException("Could not try-lock mutex. Reason: "
			+ std::string(strerror(result)));
	}
}

void CMutex::Unlock()
	throw (CSyncException)
{
	int result = 0;

	if ((result = pthread_mutex_unlock(&iMutex)) != 0)
	{
		throw CSyncException("Could not unlock mutex. Reason: "
			+  std::string(strerror(result)));
	}
};

CMutex::operator pthread_mutex_t*()
{
	return &iMutex;
};
