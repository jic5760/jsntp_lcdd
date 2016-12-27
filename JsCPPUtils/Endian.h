/**
 * @file	Endian.h
 * @class	Endian
 * @author	Jichan (jic5760@naver.com)
 * @date	2016/09/27
 * @brief	Auto detect endian
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#pragma once

#include "Common.h"

namespace JsCPPUtils
{

	class Endian
	{
	public:
		static int getEndian()
		{
			volatile uint32_t x = 0x12345678;
			volatile unsigned char *p = (volatile unsigned char *)&x;
			if((p[0] == 0x78) && (p[1] == 0x56) && (p[2] == 0x34) && (p[3] == 0x12))
				return 0;
			if((p[0] == 0x12) && (p[1] == 0x34) && (p[2] == 0x56) && (p[3] == 0x78))
				return 1;
			return -1;
		}
	};

}