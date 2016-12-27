/**
 * @file	Lock.h
 * @class	Lock
 * @author	Jichan (jic5760@naver.com)
 * @date	2016/10/14
 * @brief	Lock. It can help to thread-safe.
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#pragma once

#include "Common.h"

#if defined(JSCUTILS_OS_WINDOWS)
#include <windows.h>
#elif defined(JSCUTILS_OS_LINUX)
#include <pthread.h>
#endif

namespace JsCPPUtils
{

	class Lockable
	{
	private:
#if defined(JSCUTILS_OS_WINDOWS)
		HANDLE m_hMutex;
#elif defined(JSCUTILS_OS_LINUX)
		pthread_mutex_t m_mutex;
#endif

	public:
#if defined(JSCUTILS_OS_WINDOWS)
		Lockable()
		{
			volatile DWORD dwerr = 0;
			m_hMutex = ::CreateMutex(NULL, FALSE, NULL);
			if(m_hMutex == INVALID_HANDLE_VALUE)
			{
				dwerr = GetLastError();
				throw dwerr;
			}
		}

		~Lockable()
		{
			CloseHandle(m_hMutex);
			m_hMutex = INVALID_HANDLE_VALUE;
		}

		void lock()
		{
			DWORD dwWait;
			DWORD dwErr;
			dwWait = WaitForSingleObject(m_hMutex, INFINITE);
			if(dwWait != 0)
			{
				dwErr = GetLastError();
				throw dwErr;
			}
		}

		void unlock()
		{
			DWORD dwErr;
			if(!ReleaseMutex(m_hMutex))
			{
				dwErr = GetLastError();
				throw dwErr;
			}
		}
#elif defined(JSCUTILS_OS_LINUX)
		Lockable()
		{
			pthread_mutex_init(&m_mutex, NULL);
		}

		~Lockable()
		{
			// pthread_mutex_destory(&m_mutex);
		}

		void lock()
		{
			pthread_mutex_lock(&m_mutex);
		}

		void unlock()
		{
			pthread_mutex_unlock(&m_mutex);
		}
#endif
	};

}
