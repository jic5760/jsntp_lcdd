/**
 * @file	TSSimpleMap.h
 * @class	TSSimpleMap
 * @brief	Thread-Safe한 Map을 구현한 클래스
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
*/

#pragma once

#include <map>

#include "AtomicNum.h"

namespace JsCPPUtils
{
	template <typename TKEY, typename TVALUE>
	class TSSimpleMap : private Lockable
	{
	private:
		std::map<TKEY, TVALUE > m_map;

	public:
		TSSimpleMap() : 
			JsCPPUtils::Lockable()
		{
		}
		~TSSimpleMap()
		{
		}

		/*
		TVALUE& operator[] (const TKEY& key) {
			lock();
			TVALUE& valueref = m_map[key];
			unlock();
			return valueref;
		}
		*/

		TVALUE get(const TKEY& key) {
			TVALUE value;
			lock();
			value = m_map[key];
			unlock();
			return value;
		}

		void set(const TKEY& key, const TVALUE& value) {
			lock();
			m_map[key] = value;
			unlock();
		}
	};

}
