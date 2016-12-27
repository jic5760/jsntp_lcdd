/**
 * @file	SmartPointer.h
 * @class	SmartPointer
 * @author	Jichan (jic5760@naver.com / ablog.jc-lab.net)
 * @date	2016/09/30
 * @brief	SmartPointer
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#pragma once

#include <stdio.h>

namespace JsCPPUtils
{
	template <class T>
	class SmartPointerNTS {
	private:
		bool m_isRoot;
		T *m_ptr;
		SmartPointerNTS<T> *m_root_smartptr;

		int m_refCount;

	protected:
		SmartPointerNTS(T* p, bool isRoot) : 
			 m_refCount(0)
		{
			m_isRoot = isRoot;
			if(isRoot)
			{
				m_ptr = p;
				m_root_smartptr = NULL;
			}else{
				m_ptr = NULL;
				m_root_smartptr = new SmartPointerNTS(p);
			}
		}

	private:
		void addRef()
		{
			if(m_isRoot)
			{
				m_refCount++;
			}else{
				m_root_smartptr->addRef();
			}
		}

		void delRef()
		{
			if(m_isRoot)
			{
				m_refCount--;
				if(m_refCount <= 0)
				{
					if(m_ptr != NULL)
						delete m_ptr;
					delete this;
					return;
				}
			}else{
				if(m_root_smartptr != NULL)
					m_root_smartptr->delRef();
			}
		}

	public:
		SmartPointerNTS() :
			m_isRoot(false),
			m_ptr(NULL),
			m_root_smartptr(NULL),
			m_refCount(0)
		{
		}
		
		SmartPointerNTS(T* p) :
			m_isRoot(false),
			m_ptr(NULL),
			m_root_smartptr(NULL),
			m_refCount(0)
		{
			m_root_smartptr = new SmartPointerNTS(p, true);
			m_ptr = m_root_smartptr->m_ptr;
			m_root_smartptr->addRef();
		}

		SmartPointerNTS(const SmartPointerNTS<T>& refObj) :
			m_isRoot(false),
			m_ptr(NULL),
			m_root_smartptr(NULL),
			m_refCount(0)
		{
			m_root_smartptr = refObj.m_root_smartptr;
			if(m_root_smartptr != NULL)
			{
				m_ptr = m_root_smartptr->m_ptr;
				m_root_smartptr->addRef();
			}
		}

		SmartPointerNTS(const SmartPointerNTS<T>* pRefObj) :
			m_isRoot(false),
			m_ptr(NULL),
			m_root_smartptr(NULL),
			pLock(NULL),
			m_refCount(0)
		{
			m_root_smartptr = pRefObj->m_root_smartptr;
			if(m_root_smartptr != NULL)
			{
				m_ptr = m_root_smartptr->m_ptr;
				m_root_smartptr->addRef();
			}
		}

		~SmartPointerNTS()
		{
			if(!m_isRoot)
			{
				delRef();
			}
		}

		T* operator->() const
		{
			return m_ptr;
		}

		T& operator *() const
		{
			return *m_ptr;
		}

		T* getPtr() const
		{
			return m_ptr;
		}

		SmartPointerNTS<T>& operator=(T* p)
		{
			if(m_root_smartptr != NULL)
				m_root_smartptr->delRef();
			m_root_smartptr = new SmartPointerNTS(p, true);
			m_ptr = m_root_smartptr->m_ptr;
			m_root_smartptr->addRef();
			return *this;
		}

		SmartPointerNTS<T>& operator=(const SmartPointerNTS<T>& refObj)
		{
			if(m_root_smartptr != NULL)
				m_root_smartptr->delRef();
			m_root_smartptr = refObj.m_root_smartptr;
			if(m_root_smartptr != NULL)
			{
				m_ptr = m_root_smartptr->m_ptr;
				m_root_smartptr->addRef();
			}
			return *this;
		}

		bool operator!() const
		{
			return (m_ptr == NULL);
		}

		bool operator==(SmartPointerNTS<T> p) const
		{
			return m_ptr == p.m_ptr;
		}

		bool operator!=(SmartPointerNTS<T> p) const
		{
			return m_ptr != p.m_ptr;
		}

		bool operator==(T* p) const
		{
			return m_ptr == p;
		}

		bool operator!=(T* p) const
		{
			return m_ptr != p;
		}

		SmartPointerNTS<T> *detach()
		{
			if(m_root_smartptr != NULL)
				m_root_smartptr->addRef();
			return m_root_smartptr;
		}

		void attach(SmartPointerNTS<T> *p)
		{
			if(m_root_smartptr != NULL)
				m_root_smartptr->delRef();
			m_root_smartptr = p;
			if(m_root_smartptr != NULL)
				m_ptr = m_root_smartptr->m_ptr;
		}
	};

}
