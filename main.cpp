#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "ntpqmod.h"
#include "I2CLCD.h"
#include "JsCPPUtils/AtomicNum.h"
#include "JsCPPUtils/Daemon.h"

#include "GPSWork.h"

static int daemon_startup(JsCPPUtils::Daemon *pdaemon, void *cbparam, int argc, char *argv[]);
static int daemon_main(JsCPPUtils::Daemon *pdaemon, void *cbparam, int argc, char *argv[]);

JsCPPUtils::Daemon m_daemon(NULL, daemon_startup, daemon_main);

int main(int argc, char *argv[])
{
	return m_daemon.main(argc, argv);
}

int daemon_startup(JsCPPUtils::Daemon *pdaemon, void *cbparam, int argc, char *argv[])
{
	return 0;
}

int daemon_main(JsCPPUtils::Daemon *pdaemon, void *cbparam, int argc, char *argv[])
{
	I2CLCD lcd;
	int nrst;
	char straddrbuf[256];
	volatile int64_t timers[4] = {0};

	GPSWork work_gps;

	nrst = lcd.busOpen("/dev/i2c-1");

	nrst = init_ntpq();
	
	while(pdaemon->getRunStatus() == 1)
	{
		volatile int64_t curtcnt = JsCPPUtils::Common::getTickCount();

		int i;
		int n;
		struct timeval now_tv;
		tm now_tm;
		long now_microsec;
		
		gettimeofday(&now_tv, NULL);
		now_microsec = now_tv.tv_usec;
		gmtime_r(&now_tv.tv_sec, &now_tm);
		
		if((curtcnt - timers[3]) >= 250)
		{
			int numassoc;
			unsigned short associds[128] = {0};
			ntpqmod_peerinfo_t peerinfo = {0};
			
			char strbuf[64];
			char strstatus1[64] = {0};

			int t_isfirst = 0;

			timers[3] = curtcnt;

			numassoc = ntpq_read_associations(associds, sizeof(associds)/sizeof(associds[0]));

			if(!pdaemon->isDaemon())
			{
				printf("\033[H\033[J");
				printf("       remote     refid            st t  when poll reach delay     offset    jitter    \n");
				printf("=======================================================================================\n");
			}

			for (i = 0; i < numassoc; i++)
			{
				ntpqmod_readpeer(&peerinfo, associds[i]);
				straddrbuf[0] = 0;
				if((peerinfo.srcadr.sa.sa_family == PF_INET) || (peerinfo.srcadr.sa.sa_family == AF_INET))
					inet_ntop(peerinfo.srcadr.sa.sa_family, (void*)&peerinfo.srcadr.sa4.sin_addr, straddrbuf, sizeof(peerinfo.srcadr));
				else if((peerinfo.srcadr.sa.sa_family == PF_INET6) || (peerinfo.srcadr.sa.sa_family == AF_INET6))
					inet_ntop(peerinfo.srcadr.sa.sa_family, (void*)&peerinfo.srcadr.sa6.sin6_addr, straddrbuf, sizeof(peerinfo.dstadr));
				if(!pdaemon->isDaemon())
				{
					printf("%c ", peerinfo.c);
					printf("%-15s ", straddrbuf);
					printf("%c%-15s ", (peerinfo.have_refid == 1) ? '.' : ' ', peerinfo.refid);
					printf("%2d ", peerinfo.stratum);
					printf("%c  ", peerinfo.hmode_c);
					printf("%-4s ", peerinfo.whenbuf);
					printf("%-4s ", peerinfo.pollbuf);
					printf("%-5d ", peerinfo.reach); // reach
					printf("%-9s ", lfptoms(&peerinfo.estdelay, 3)); // delay
					printf("%-9s ", lfptoms(&peerinfo.estoffset, 3)); // offset
					printf("%-9s\n", lfptoms(&peerinfo.estjitter, 3)); // jitter
				}

				if(peerinfo.c != ' ')
				{
					if(t_isfirst == 0)
					{
						t_isfirst = 1;
					}else{
						strcat(strstatus1, " ");
					}
					if(peerinfo.have_refid == 2)
					{
						straddrbuf[0] = 0;
						if((peerinfo.srcadr.sa.sa_family == PF_INET) || (peerinfo.srcadr.sa.sa_family == AF_INET))
							inet_ntop(peerinfo.srcadr.sa.sa_family, (void*)&peerinfo.srcadr.sa4.sin_addr, straddrbuf, sizeof(peerinfo.srcadr));
						else if((peerinfo.srcadr.sa.sa_family == PF_INET6) || (peerinfo.srcadr.sa.sa_family == AF_INET6))
							inet_ntop(peerinfo.srcadr.sa.sa_family, (void*)&peerinfo.srcadr.sa6.sin6_addr, straddrbuf, sizeof(peerinfo.dstadr));
						sprintf(strbuf, "%c%.8s", peerinfo.c, straddrbuf);
					}else{
						sprintf(strbuf, "%c%.4s", peerinfo.c, peerinfo.refid);
					}
					strcat(strstatus1, strbuf);
				}
			}

			if(!pdaemon->isDaemon())
			{
				strftime(strbuf, sizeof(strbuf), "%04Y-%02m-%02d %02H:%02M:%02S", &now_tm);
				printf("\nSYSTEM TIME : %s.%06ld\n", strbuf, now_microsec);
				work_gps.m_last_data.lock();
				printf("GPS : %d/%d\n", work_gps.m_last_data.getUsedSatellitesCount(), work_gps.m_last_data.getVisibleSatellitesCount());
				work_gps.m_last_data.unlock();
			}

			if((timers[2] == 0) || ((curtcnt - timers[2]) >= 60000))
			{
				timers[2] = curtcnt;
				lcd.init(0x27, 20, 4, 0);
				lcd.backlight(true);
			}

			for(i=0, n=(lcd.getCols() - 7) / 2; i<n; i++)
			{
				strbuf[i] = '=';
			}
			strbuf[i] = 0;
			strcat(strbuf, " JsNTP ");
			i += 7;
			for(n=lcd.getCols(); i<n; i++)
			{
				strbuf[i] = '=';
			}
			strbuf[i] = 0;
			lcd.puts_newline(0, strbuf);

			work_gps.m_last_data.lock();
			sprintf(strbuf, "GPS:%d/%d", work_gps.m_last_data.getUsedSatellitesCount(), work_gps.m_last_data.getVisibleSatellitesCount());
			work_gps.m_last_data.unlock();
			lcd.puts_newline(1, strbuf);

			lcd.puts_newline(2, strstatus1);

			strftime(strbuf, sizeof(strbuf), "%04Y-%02m-%02d %02H:%02M:%02S", &now_tm);
			lcd.puts_newline(3, strbuf);
		}

		usleep(250000);
	}

	return 0;
}
