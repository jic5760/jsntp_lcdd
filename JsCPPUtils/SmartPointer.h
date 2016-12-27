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

#include "Lockable.h"

namespace JsCPPUtils
{
	template <class T>
	class SmartPointer {
	private:
		bool m_isRoot;
		T *m_ptr;
		SmartPointer<T> *m_root_smartptr;

		Lockable *pLock;
		int m_refCount;

	protected:
		SmartPointer(T* p, bool isRoot) : 
			 pLock(NULL),
			 m_refCount(0)
		{
			m_isRoot = isRoot;
			if(isRoot)
			{
				m_ptr = p;
				m_root_smartptr = NULL;
				pLock = new Lockable();
			}else{
				m_ptr = NULL;
				m_root_smartptr = new SmartPointer(p);
			}
		}

	private:
		void addRef()
		{
			if(m_isRoot)
			{
				pLock->lock();
				m_refCount++;
				pLock->unlock();
			}else{
				m_root_smartptr->addRef();
			}
		}

		void delRef()
		{
			if(m_isRoot)
			{
				pLock->lock();
				m_refCount--;
				if(m_refCount <= 0)
				{
					if(m_ptr != NULL)
						delete m_ptr;
					delete this;
					return;
				}
				pLock->unlock();
			}else{
				if(m_root_smartptr != NULL)
					m_root_smartptr->delRef();
			}
		}

	public:
		SmartPointer() :
			m_isRoot(false),
			m_ptr(NULL),
			m_root_smartptr(NULL),
			pLock(NULL),
			m_refCount(0)
		{
		}
		
		SmartPointer(T* p) :
			m_isRoot(false),
			m_ptr(NULL),
			m_root_smartptr(NULL),
			pLock(NULL),
			m_refCount(0)
		{
			m_root_smartptr = new SmartPointer(p, true);
			m_ptr = m_root_smartptr->m_ptr;
			m_root_smartptr->addRef();
		}

		SmartPointer(const SmartPointer<T>& refObj) :
			m_isRoot(false),
			m_ptr(NULL),
			m_root_smartptr(NULL),
			pLock(NULL),
			m_refCount(0)
		{
			m_root_smartptr = refObj.m_root_smartptr;
			if(m_root_smartptr != NULL)
			{
				m_ptr = m_root_smartptr->m_ptr;
				m_root_smartptr->addRef();
			}
		}

		SmartPointer(const SmartPointer<T>* pRefObj) :
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

		~SmartPointer()
		{
			if(!m_isRoot)
			{
				delRef();
			}
			if(pLock != NULL)
			{
				delete pLock;
				pLock = NULL;
			}
		}

		T* operator->() const
		{
			return m_ptr;
		}

		/*
		T& operator *() const
		{
			return *m_ptr;
		}
		*/

		T* getPtr() const
		{
			return m_ptr;
		}

		SmartPointer<T>& operator=(T* p)
		{
			if(m_root_smartptr != NULL)
				m_root_smartptr->delRef();
			m_root_smartptr = new SmartPointer(p, true);
			m_ptr = m_root_smartptr->m_ptr;
			m_root_smartptr->addRef();
			return *this;
		}

		SmartPointer<T>& operator=(const SmartPointer<T>& refObj)
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

		bool operator==(SmartPointer<T> p) const
		{
			return m_ptr == p.m_ptr;
		}

		bool operator!=(SmartPointer<T> p) const
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

		SmartPointer<T> *detach()
		{
			if(m_root_smartptr != NULL)
				m_root_smartptr->addRef();
			return m_root_smartptr;
		}

		void attach(SmartPointer<T> *p)
		{
			if(m_root_smartptr != NULL)
				m_root_smartptr->delRef();
			m_root_smartptr = p;
			if(m_root_smartptr != NULL)
				m_ptr = m_root_smartptr->m_ptr;
		}
	};

}
