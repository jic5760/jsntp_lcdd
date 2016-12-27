#ifndef __NTPQMOD_H__
#define __NTPQMOD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "config_ntp.h"
#include <ntp_fp.h>
#include <ntp_stdlib.h>
#include "ntpq/libntpq.h"

typedef struct _tag_ntpqmod_peerinfo
{
	char c;
	sockaddr_u srcadr;
	char srchost[LENHOSTNAME];
	sockaddr_u dstadr;
	char refid[128];
	sockaddr_u refidadr;
	long hmode;
	char hmode_c;
	u_long stratum;
	long hpoll;
	long ppoll;
	u_long reach;
	l_fp estoffset;
	l_fp estdelay;
	l_fp estjitter;
	l_fp estdisp;
	l_fp reftime;
	l_fp rec;
	u_long srcport;
	
	char whenbuf[8];
	char pollbuf[8];

	/**
	 * 0 : have not
	 * 1 : string
	 * 2 : address
	 */
	int have_refid;
	int have_jitter;
} ntpqmod_peerinfo_t;

extern int init_ntpq();
extern int ntpqmod_getpeers();
extern int ntpqmod_readpeer(ntpqmod_peerinfo_t *pinfo, int associd);

#ifdef __cplusplus
}
#endif

#endif