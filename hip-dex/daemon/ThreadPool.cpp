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

#include "ThreadPool.h"

using namespace HDX;

CThreadPool::CThreadPool(std::size_t aThreads)
	: iClosed(false), iPool(aThreads)
{
	for (std::size_t i = 0; i < iPool.size(); ++i)
	{
		iPool[i] = new CWorkerThread(this);
		iPool[i]->Start();
	}
};

CThreadPool::~CThreadPool()
{
	for (std::size_t i = 0; i < iPool.size(); ++i)
	{
		delete iPool[i];
	}
};

void CThreadPool::Assign(IRunnable* aTask)
{
	iCondition.Lock();

	if ((!iClosed) && (aTask != NULL))
	{
		iQueue.push(aTask);
		iCondition.Notify();
	}

	iCondition.Unlock();
};

void CThreadPool::Close()
{
	iCondition.Lock();

	iClosed = true;
	iCondition.NotifyAll();

	iCondition.Unlock();

	for (std::size_t i = 0; i < iPool.size(); ++i)
	{
		iPool[i]->Join();
	}
};

CThreadPool::CWorkerThread::CWorkerThread(CThreadPool* aPool)
	: iPool(aPool)
{
};

CThreadPool::CWorkerThread::~CWorkerThread()
{
};

void CThreadPool::CWorkerThread::Run()
{
	while (true)
	{
		IRunnable* pTask = NULL;
		iPool->iCondition.Lock();

		while (iPool->iQueue.empty())
		{
			if (iPool->iClosed)
			{
				iPool->iCondition.Unlock();
				return;
			}

			iPool->iCondition.Wait();
		}

		pTask = iPool->iQueue.front();
		iPool->iQueue.pop();

		iPool->iCondition.Unlock();

		if (pTask == NULL)
			return;

		pTask->Run();
		delete pTask;
	}
};
