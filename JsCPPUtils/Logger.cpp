/**
 * @file	Logger.cpp
 * @class	Logger
 * @author	Jichan (jic5760@naver.com)
 * @date	2016/11/02
 * @brief	Logger
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include "Logger.h"

#ifdef HAS_SYSLOG
#include <syslog.h>
#endif

namespace JsCPPUtils
{

	Logger::Logger(OutputType outputType, const char *szFilePath, CallbackFunc_t cbfunc, void *cbuserptr) : 
		m_pParent(NULL)
	{
		int nrst;

		m_outtype = outputType;
		m_fp = NULL;
		m_cbfunc = NULL;
		m_cbuserptr = NULL;
		m_lasterrno = 0;

		switch(m_outtype)
		{
		case TYPE_STDOUT:
			m_fp = stdout;
			break;
		case TYPE_STDERR:
			m_fp = stderr;
			break;
		case TYPE_FILE: {
#ifdef _JSCUTILS_MSVC_CRT_SECURE
			errno_t eno;
			eno = fopen_s(&m_fp, szFilePath, "a+");
			m_lasterrno = (int)eno;
#else
			m_fp = fopen(szFilePath, "a+");
			if(m_fp == NULL)
				m_lasterrno = errno;
#endif
			}
			break;
		case TYPE_CALLBACK:
			m_cbfunc = cbfunc;
			m_cbuserptr = cbuserptr;
			break;
		}
	}
	
	Logger::Logger(Logger *pParent, const std::string& strPrefixName) : 
		m_pParent(pParent),
		m_strPrefixName(strPrefixName),
		m_outtype(TYPE_NULL)
	{
	}


	Logger::~Logger()
	{
		switch(m_outtype)
		{
		case TYPE_FILE:
			if(m_fp != NULL)
			{
				fclose(m_fp);
				m_fp = NULL;
			}
		}
	}
	
	void Logger::_child_puts(LogType logtype, const std::string& strPrefixName, const char* szLog)
	{
		if(m_pParent != NULL)
		{
			m_pParent->_child_puts(logtype, m_strPrefixName + ": " + strPrefixName, szLog);
		}else{
			printf(logtype, "%s: %s", strPrefixName.c_str(), szLog);
		}
	}

	void Logger::printf(LogType logtype, const char* format, ...)
	{
		static char strmonths[][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
		
		long buf1size, buf2size;
		char *pbuf1 = NULL;
		char *pbuf2 = NULL;

		va_list args;
	
		time_t rawtime;
		struct tm timeinfo;

		const char *strlogtype = NULL;
		const char *stroutput = NULL;

		do {
			buf1size = buf2size = 4096;
			
			pbuf1 = (char*)malloc(buf1size);
			if(pbuf1 == NULL)
				break;
			
			time(&rawtime);
			localtime_r(&rawtime, &timeinfo);

			va_start(args, format);
#ifdef _JSCUTILS_MSVC_CRT_SECURE
			vsprintf_s(buf, buf1size, format, args);
#else
			vsprintf(pbuf1, format, args);
#endif
			va_end(args);

			
			if(m_pParent != NULL)
			{
				m_pParent->_child_puts(logtype, m_strPrefixName, pbuf1);
			}else{
				switch(logtype)
				{
				case LOGTYPE_EMERG:
					strlogtype = "EMERG";
					break;
				case LOGTYPE_ALERT:
					strlogtype = "ALERT";
					break;
				case LOGTYPE_CRIT:
					strlogtype = "CRIT";
					break;
				case LOGTYPE_ERR:
					strlogtype = "ERR";
					break;
				case LOGTYPE_WARNING:
					strlogtype = "WARN";
					break;
				case LOGTYPE_NOTICE:
					strlogtype = "NOTI";
					break;
				case LOGTYPE_INFO:
					strlogtype = "INFO";
					break;
				case LOGTYPE_DEBUG:
					strlogtype = "DEBUG";
					break;
				default:
					strlogtype = "UNDEFINED";
					break;
				}

				switch(m_outtype)
				{
				case TYPE_STDOUT:
				case TYPE_STDERR:
				case TYPE_FILE:
				case TYPE_CALLBACK:
					pbuf2 = (char*)malloc(buf1size);
					if(pbuf2 == NULL)
						break;
	#ifdef _JSCUTILS_MSVC_CRT_SECURE
					sprintf_s(pbuf2, buf2size, "[%s] %s %02d %02d:%02d:%02d %d] %s\n", strlogtype, strmonths[timeinfo.tm_mon], timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_year + 1900, pbuf1);
	#else
					sprintf(pbuf2, "[%s] %s %02d %02d:%02d:%02d %d] %s\n", strlogtype, strmonths[timeinfo.tm_mon], timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_year + 1900, pbuf1);
	#endif
					stroutput = pbuf2;
					break;
				case TYPE_SYSLOG:
					stroutput = pbuf1;
					break;
				default:
					stroutput = NULL;
					break;
				}

				if(stroutput == NULL)
					break;

				lock();
				switch(m_outtype)
				{
				case TYPE_STDOUT:
				case TYPE_STDERR:
				case TYPE_FILE:
					fputs(stroutput, m_fp);
					break;
				case TYPE_CALLBACK:
					m_cbfunc(m_cbuserptr, stroutput);
					break;
				case TYPE_SYSLOG:
	#ifdef HAS_SYSLOG
					syslog(logtype, "%s", stroutput);
	#endif
					break;
				default:
					break;
				}
				unlock();
			}

		}while(0);
		
		if(pbuf1 != NULL)
		{
			free(pbuf1);
			pbuf1 = NULL;
		}
		if(pbuf2 != NULL)
		{
			free(pbuf2);
			pbuf2 = NULL;
		}
	}
}
