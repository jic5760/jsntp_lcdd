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

#include "Lockable.h"
#include "TSSimpleMap.h"

namespace JsCPPUtils
{
	class LockableEx
	{
	private:
		Lockable m_lock;
#if defined(JSCUTILS_OS_WINDOWS)
		TSSimpleMap<DWORD, int> m_tinfo;
#elif defined(JSCUTILS_OS_LINUX)
		TSSimpleMap<pthread_t, int> m_tinfo;
#endif

	public:
		LockableEx() : 
			m_lock()
		{
			
		}

		~LockableEx()
		{
			
		}

#if defined(JSCUTILS_OS_WINDOWS)
		void lock()
		{
			DWORD dwTid = GetCurrentThreadId();
			int _locked = m_tinfo.get(dwTid);
			if(_locked++ == 0)
			{
				m_lock.lock();
			}
			m_tinfo.set(dwTid, _locked);
		}

		void unlock()
		{
			DWORD dwTid = GetCurrentThreadId();
			int _locked = m_tinfo.get(dwTid);
			if(--_locked == 0)
			{
				m_lock.unlock();
			}
			m_tinfo.set(dwTid, _locked);
		}
#elif defined(JSCUTILS_OS_LINUX)
		void lock()
		{
			pthread_t curthread = pthread_self();
			int _locked = m_tinfo.get(curthread);
			if(_locked++ == 0)
			{
				m_lock.lock();
			}
			m_tinfo.set(curthread, _locked);
		}

		void unlock()
		{
			pthread_t curthread = pthread_self();
			int _locked = m_tinfo.get(curthread);
			if(--_locked == 0)
			{
				m_lock.unlock();
			}
			m_tinfo.set(curthread, _locked);
		}
#endif

	};
}
