#include "Daemon.h"

#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <signal.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>

namespace JsCPPUtils {

	Daemon *Daemon::m_instance = NULL;

	Daemon::Daemon()
	{
		_init();
	}
	
	Daemon::Daemon(void *cbparam, daemonstartup_t fpdaemonstartup, daemonmain_t fpdaemonmain)
	{
		_init();

		m_cbparam = cbparam;
		m_daemonstartup = fpdaemonstartup;
		m_daemonmain = fpdaemonmain;
	}

	void Daemon::_init()
	{
		m_instance = this;

		m_plogger = NULL;

		m_sighandler = NULL;
		m_reloadhandler = NULL;
		m_daemonstartup = NULL;
		m_daemonmain = NULL;
		m_helphandler = NULL;
		m_cbparam = NULL;

		m_isDaemon = false;

		m_runstatus = 0;
	}

	Daemon::~Daemon()
	{
		if(m_plogger != NULL)
		{
			delete m_plogger;
			m_plogger = NULL;
		}
	}

	void Daemon::signalhandler(int sig)
	{
		if(m_instance == NULL)
			return ;
		if(m_instance->m_sighandler != NULL)
			m_instance->m_sighandler(m_instance, m_instance->m_cbparam, sig);
		switch(sig)
		{
		case SIGINT:
		case SIGTERM:
			m_instance->m_runstatus.getifset(2, 1);
			break;
		case SIGHUP:
			if(m_instance->m_reloadhandler != NULL)
				m_instance->m_reloadhandler(m_instance, m_instance->m_cbparam);
			break;
		}
	}

	int Daemon::checkArg(const char *arg, const char *prefix)
	{
		char prefixbuf[64];
		strcpy(prefixbuf, prefix);
		strcat(prefixbuf, "=");
		if(strcmp(arg, prefix) == 0)
			return 0;
		else if(strstr(arg, prefixbuf) == arg)
			return strlen(prefixbuf);
		else
			return -1;
	}

	int Daemon::main(int argc, char *argv[])
	{
		int rc;

		int argtype;
	
		char *arg_pidfile = NULL;
		int arg_uid = -1;
		int arg_gid = -1;
		char *arg_username = NULL;
		char *arg_logfilepath = NULL;
	
		pid_t childpid;
	
		if (argc > 1)
		{
			int argc2 = argc - 1;
			int index = 1;
			for (index = 1; argc2; argc2--, index++)
			{
				if (strcmp(argv[index], "-D") == 0)
				{
					m_isDaemon = true;
				}
				else if ((argtype = checkArg(argv[index], "--pidfile")) != -1)
				{
					if (argtype == 0)
					{
						arg_pidfile = argv[++index]; argc2--;
					}else{
						arg_pidfile = &argv[index][argtype];
					}
				}
				else if ((argtype = checkArg(argv[index], "--user")) != -1)
				{
					int block_retval = 0;
					struct passwd *pwd = NULL;
					size_t buffer_len;
					char *buffer = NULL;

					if (argtype == 0)
					{
						arg_username = argv[++index]; argc2--;
					}else{
						arg_username = &argv[index][argtype];
					}

					do
					{
						pwd = (struct passwd*)calloc(1, sizeof(struct passwd));
						if (pwd == NULL)
						{
							fprintf(stderr, "Failed to allocate struct passwd for getpwnam_r.\n");
							break;
						}
						buffer_len = sysconf(_SC_GETPW_R_SIZE_MAX) * sizeof(char);
						buffer = (char*)malloc(buffer_len);
						if (buffer == NULL)
						{
							fprintf(stderr, "Failed to allocate buffer for getpwnam_r.\n");
							break;
						}
						getpwnam_r(arg_username, pwd, buffer, buffer_len, &pwd);
						if (pwd == NULL)
						{
							fprintf(stderr, "getpwnam_r failed to find requested entry.\n");
							break;
						}
						arg_uid = pwd->pw_uid;
						arg_gid = pwd->pw_gid;
					} while (0);

					if (pwd) free(pwd);
					if (buffer) free(buffer);
				
					if (block_retval != 0) return block_retval;
				}
				else if ((argtype = checkArg(argv[index], "--logfile")) != -1)
				{
					if (argtype == 0)
					{
						arg_logfilepath = argv[++index]; argc2--;
					}else{
						arg_logfilepath = &argv[index][argtype];
					}
				}
				else if (strcmp(argv[index], "--help") == 0)
				{
					printf("%s\n", argv[0]);
					printf("\t-D : Daemon\n");
					printf("\t--pidfile FILEPATH\n");
					printf("\t--user USERNAME\n");
					printf("\t--logfile LOGFILE\n");
					printf("\t--help\n");
					if(m_helphandler != NULL)
						m_helphandler(this, m_cbparam);
					return 0;
				}
			}
		}
	
		if (m_isDaemon)
		{
			childpid = fork();
			if (childpid > 0)
			{
				// parent process
				if (arg_pidfile != NULL)
				{
					FILE *pidfile_fp = fopen(arg_pidfile, "wt");
					if (pidfile_fp == NULL)
					{
						fprintf(stderr, "write pidfile failed: %d\n", errno);
					}
					else {
						fprintf(pidfile_fp, "%d", childpid);
						fclose(pidfile_fp);
					}
				}
				return 0;
			}
			else if (childpid == 0)
			{
				// child process
			}
			else if (childpid == -1)
			{
				// error
				fprintf(stdout, "fork() failed: %d\n", errno);
				return 1;
			}
		}
		
		if (arg_logfilepath != NULL)
		{
			if(m_plogger != NULL)
			{
				delete m_plogger;
				m_plogger = NULL;
			}
			m_plogger = new Logger(Logger::TYPE_FILE, arg_logfilepath, NULL, NULL);
		}
	
		if (arg_uid >= 0)
		{
			FILE *fp_group;
			char strbuf[256];
			int  grouplistcnt = 0;
			gid_t grouplist[64];
		
			fp_group = fopen("/etc/group", "rt");
			if (fp_group == NULL)
			{
				fprintf(stdout, "open /etc/group file failed: %d\n", errno);
				return 1;
			}
		
			while (fgets(strbuf, sizeof(strbuf), fp_group))
			{
				char *pcontext = NULL;
				char *strtoken1 = strtok_r(strbuf, ":", &pcontext);
				char *strtoken2 = strtok_r(NULL, ":", &pcontext);
				char *strtoken3 = strtok_r(NULL, ":", &pcontext);
				char *strtoken4 = strtok_r(NULL, ":", &pcontext);
				char *t_usernames = strtoken4;
				int t_gid = atoi(strtoken3);
				char *t_usernametok;
				size_t tmplen;

				tmplen = strlen(strtoken4);
				while((tmplen > 0) && ((strtoken4[tmplen - 1] == '\r') || (strtoken4[tmplen - 1] == '\n') || (strtoken4[tmplen - 1] == ' ') || (strtoken4[tmplen - 1] == '\t')))
					strtoken4[--tmplen] = 0;

				t_usernametok = strtok_r(t_usernames, ",", &pcontext);
				while (t_usernametok)
				{
					if (strcmp(t_usernametok, arg_username) == 0)
					{
						if (grouplistcnt >= 64) break;
						grouplist[grouplistcnt++] = t_gid;
					}
					t_usernametok = strtok_r(NULL, ",", &pcontext);
				}
			}
		
			if (arg_logfilepath != NULL)
			{
				chown(arg_logfilepath, arg_uid, arg_gid);
			}
		
			setgroups(grouplistcnt, grouplist);
		}

		if(m_plogger == NULL)
		{
			m_plogger = new Logger(Logger::TYPE_STDOUT, NULL, NULL, NULL);
		}
	
		if(m_daemonstartup != NULL)
		{
			rc = m_daemonstartup(this, m_cbparam, argc, argv);
			if (rc != 0)
			{
				goto FUNCEXIT;
			}
		}
	
		if (arg_uid >= 0)
		{
			setgid(arg_gid);
			setuid(arg_uid);
		}
	
		signal(SIGINT, signalhandler);
		signal(SIGTERM, signalhandler);
		signal(SIGHUP, signalhandler);
	
		m_runstatus = 1;
		if(m_daemonmain != NULL)
			rc = m_daemonmain(this, m_cbparam, argc, argv);
		else
			rc = 1;

	FUNCEXIT:
	
		return rc;
	}
		
	bool Daemon::isDaemon()
	{
		return m_isDaemon;
	}

	int Daemon::getRunStatus()
	{
		return m_runstatus.get();
	}
		
	void Daemon::setCallbackParam(void *cbparam)
	{
		m_cbparam = cbparam;
	}

	void Daemon::setSigHandler(sighandler_t fphandler)
	{
		m_sighandler = fphandler;
	}

	void Daemon::setReloadHandler(reloadhandler_t fphandler)
	{
		m_reloadhandler = fphandler;
	}
	
	void Daemon::setDaemonStartup(daemonstartup_t fpdaemonstartup)
	{
		m_daemonstartup = fpdaemonstartup;
	}
	
	void Daemon::setDaemonMain(daemonmain_t fpdaemonmain)
	{
		m_daemonmain = fpdaemonmain;
	}
	
	void Daemon::setHelpHandler(helphandler_t fphandler)
	{
		m_helphandler = fphandler;
	}

	Logger *Daemon::getLogger()
	{
		return m_plogger;
	}
}
