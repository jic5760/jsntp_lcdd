/**
 * @file	Common.h
 * @author	Jichan (jic5760@naver.com)
 * @date	2016/09/27
 * @brief	JsCUtils Common file
 * @brief	CmdlineParser
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#ifndef __JSCUTILS_COMMON_H__
#define __JSCUTILS_COMMON_H__

#if defined(__linux__)

#define JSCUTILS_OS_LINUX

#include <stdint.h>

#elif defined(_WIN32)

#define JSCUTILS_OS_WINDOWS

#ifdef _MSC_VER
#if _MSC_VER < 1600 // MSVC++ 10.0 _MSC_VER == 1600 (Visual Studio 2010)

#ifndef __JSSTDINT_TYPES__
#define __JSSTDINT_TYPES__
typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;
typedef __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#endif /* __JSSTDINT_TYPES__ */

#else
#include <stdint.h>
#endif /* _MSC_VER < 1600 */
#else
#include <stdint.h>
#endif /* _MSC_VER */

#else

#endif

#endif /* __JSCUTILS_COMMON_H__ */

#ifdef __cplusplus

#pragma once

namespace JsCPPUtils
{
	class Common
	{
	public:
		static int64_t getTickCount();
	};
}

#endif