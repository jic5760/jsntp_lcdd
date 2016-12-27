/**
 * @file	Common.cpp
 * @author	Jichan (jic5760@naver.com)
 * @date	2016/09/27
 * @brief	JsCPPUtils Common file
 */

#include <time.h>

#include "Common.h"

namespace JsCPPUtils
{
	int64_t Common::getTickCount()
	{
#if defined(JSCUTILS_OS_WINDOWS)
		return GetTickCount64();
#elif defined(JSCUTILS_OS_LINUX)
		struct timespec ts = {0};
		int64_t ticks = 0;
		clock_gettime(CLOCK_MONOTONIC, &ts);
		ticks  = ((int64_t)(ts.tv_nsec / 1000000));
		ticks += ((int64_t)(ts.tv_sec)) * 1000;
		return ticks;
#endif
	}
}