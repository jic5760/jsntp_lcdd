/**
* @file	Logger.cpp
* @class	Logger
* @author	Jichan (jic5760@naver.com / ablog.jc-lab.net)
* @date	2016/11/02
* @brief	JsLogger
*/

#pragma once

#include <string>
#include <stdio.h>

#include "Lockable.h"

namespace JsCPPUtils
{
	class Logger : public Lockable
	{
	public:
		enum OutputType {
			TYPE_NULL = 0,
			TYPE_STDOUT,
			TYPE_STDERR,
			TYPE_FILE,
			TYPE_CALLBACK,
			TYPE_SYSLOG
		};

		enum LogType {
			LOGTYPE_EMERG = 0,
			LOGTYPE_ALERT = 1,
			LOGTYPE_CRIT = 2,
			LOGTYPE_ERR = 3,
			LOGTYPE_WARNING = 4,
			LOGTYPE_NOTICE = 5,
			LOGTYPE_INFO = 6,
			LOGTYPE_DEBUG = 7
		};

		typedef void (*CallbackFunc_t)(void *userptr, const char *stroutput);

	private:
		Logger *m_pParent;
		std::string m_strPrefixName;

		FILE *m_fp;
		OutputType m_outtype;

		CallbackFunc_t m_cbfunc;
		void *m_cbuserptr;

		int m_lasterrno;

	public:
		Logger(OutputType outputType, const char *szFilePath, CallbackFunc_t cbfunc, void *cbuserptr);
		Logger(Logger *pParent, const std::string& strPrefixName);
		~Logger();
		void printf(LogType logtype, const char* format, ...);
		void _child_puts(LogType logtype, const std::string& strPrefixName, const char* szLog);
	};

}
