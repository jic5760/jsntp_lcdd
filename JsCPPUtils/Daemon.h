#pragma once

#include "AtomicNum.h"
#include "Logger.h"

namespace JsCPPUtils
{

	class Daemon
	{
	public:
		typedef void (*sighandler_t)(Daemon *pdaemon, void *cbparam, int sig);
		typedef void (*reloadhandler_t)(Daemon *pdaemon, void *cbparam);
		typedef int (*daemonstartup_t)(Daemon *pdaemon, void *cbparam, int argc, char *argv[]);
		typedef int (*daemonmain_t)(Daemon *pdaemon, void *cbparam, int argc, char *argv[]);
		typedef void (*helphandler_t)(Daemon *pdaemon, void *cbparam);

	private:
		static Daemon *m_instance;
		
		/**
		 * 0 : stopped
		 * 1 : running
		 * 2 : stopping
		 */
		AtomicNum<int> m_runstatus;

		Logger *m_plogger;

		void *m_cbparam;
		sighandler_t m_sighandler;
		reloadhandler_t m_reloadhandler;
		daemonstartup_t m_daemonstartup;
		daemonmain_t m_daemonmain;
		helphandler_t m_helphandler;

		bool m_isDaemon;

		int checkArg(const char *arg, const char *prefix);

	public:
		Daemon();
		Daemon(void *cbparam, daemonstartup_t fpdaemonstartup, daemonmain_t fpdaemonmain);
		~Daemon();

		void _init();

		static void signalhandler(int sig);

		int main(int argc, char *argv[]);
		
		bool isDaemon();

		int getRunStatus();
		
		void setCallbackParam(void *cbparam);
		void setSigHandler(sighandler_t fphandler);
		void setReloadHandler(reloadhandler_t fphandler);
		void setDaemonStartup(daemonstartup_t fpdaemonstartup);
		void setDaemonMain(daemonmain_t fpdaemonmain);
		void setHelpHandler(helphandler_t fphandler);

		Logger *getLogger();
	};

}

