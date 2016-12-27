/**
 * @file	AtomicNum.h
 * @class	AtomicNum
 * @brief	Thread-Safe한 숫자형을 구현한 클래스
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#pragma once

#include "LockableEx.h"

namespace JsCPPUtils
{

	template <typename T>
	class AtomicNum : public LockableEx
	{
	private:
		T m_value;

	public:
		AtomicNum() : 
			LockableEx(),
			m_value(0)
		{
		}

		AtomicNum(int initialvalue) : 
			LockableEx(),
			m_value(initialvalue)
		{
		}
	
		~AtomicNum()
		{
		}

		void operator=(T y)
		{
			lock();
			m_value = y;
			unlock();
		}

		void operator+=(T y)
		{
			lock();
			m_value += y;
			unlock();
		}

		void operator-=(T y)
		{
			lock();
			m_value -= y;
			unlock();
		}

		void operator*=(T y)
		{
			lock();
			m_value *= y;
			unlock();
		}

		void operator/=(T y)
		{
			lock();
			m_value /= y;
			unlock();
		}

		void operator%=(T y)
		{
			lock();
			m_value %= y;
			unlock();
		}

		void operator<<=(T y)
		{
			lock();
			m_value <<= y;
			unlock();
		}

		void operator>>=(T y)
		{
			lock();
			m_value >>= y;
			unlock();
		}

		void operator^=(T y)
		{
			lock();
			m_value ^= y;
			unlock();
		}

		void operator&=(T y)
		{
			lock();
			m_value &= y;
			unlock();
		}

		void operator|=(T y)
		{
			lock();
			m_value |= y;
			unlock();
		}

		bool operator==(T y)
		{
			bool retval;
			lock();
			retval = m_value == y;
			unlock();
			return retval;
		}

		bool operator!=(T y)
		{
			bool retval;
			lock();
			retval = m_value != y;
			unlock();
			return retval;
		}

		bool operator>(T y)
		{
			bool retval;
			lock();
			retval = m_value > y;
			unlock();
			return retval;
		}

		bool operator<(T y)
		{
			bool retval;
			lock();
			retval = m_value < y;
			unlock();
			return retval;
		}

		bool operator>=(T y)
		{
			bool retval;
			lock();
			retval = m_value >= y;
			unlock();
			return retval;
		}

		bool operator<=(T y)
		{
			bool retval;
			lock();
			retval = m_value <= y;
			unlock();
			return retval;
		}

		operator T()
		{
			T value;
			lock();
			value = m_value;
			unlock();
			return value;
		}

		T get()
		{
			T value;
			lock();
			value = m_value;
			unlock();
			return value;
		}

		T getset(T value)
		{
			T old;
			lock();
			old = m_value;
			m_value = value;
			unlock();
			return old;
		}

		T getifset(T value, T ifvalue)
		{
			T old;
			lock();
			old = m_value;
			if(old == ifvalue)
				m_value = value;
			unlock();
			return old;
		}

		T getifnset(T value, T ifnvalue)
		{
			T old;
			lock();
			old = m_value;
			if(old != ifnvalue)
				m_value = value;
			unlock();
			return old;
		}

	};

}