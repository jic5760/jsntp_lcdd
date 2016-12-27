
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>

#include "ntpqmod.h"
#include <ntp.h>
#include <ntp_control.h>

/*
 * Opcodes
 */
#define CTL_OP_UNSPEC           0       /* unspeciffied */
#define CTL_OP_READSTAT         1       /* read status */
#define CTL_OP_READVAR          2       /* read variables */
#define CTL_OP_WRITEVAR         3       /* write variables */
#define CTL_OP_READCLOCK        4       /* read clock variables */
#define CTL_OP_WRITECLOCK       5       /* write clock variables */
#define CTL_OP_SETTRAP          6       /* set trap address */
#define CTL_OP_ASYNCMSG         7       /* asynchronous message */
#define CTL_OP_CONFIGURE        8       /* runtime configuration */
#define CTL_OP_SAVECONFIG       9       /* save config to file */
#define CTL_OP_READ_MRU         10      /* retrieve MRU (mrulist) */
#define CTL_OP_READ_ORDLIST_A   11      /* ordered list req. auth. */
#define CTL_OP_REQ_NONCE        12      /* request a client nonce */
#define CTL_OP_UNSETTRAP        31      /* unset trap */

#define NTPQ_BUFLEN 2048

struct ntpq_varlist my_peervarlist[] = {
	{ "srcadr", 0 },    /* 0 */
	{ "refid",  0 },    /* 1 */
	{ "stratum",    0 },    /* 2 */
	{ "hpoll",  0 },    /* 3 */
	{ "ppoll",  0 },    /* 4 */
	{ "reach",  0 },    /* 5 */
	{ "delay",  0 },    /* 6 */
	{ "offset", 0 },    /* 7 */
	{ "jitter", 0 },    /* 8 */
	{ "dispersion", 0 },    /* 9 */
	{ "rec",    0 },    /* 10 */
	{ "reftime",    0 },    /* 11 */
	{ "srcport",    0 },    /* 12 */
	{ "hmode",      0 },    /* 13 */
	{ "srchost",    0 },    /* 14 */
	{ 0,		0 }
};

static int doreadpeer(
	ntpqmod_peerinfo_t *pinfo,
	const struct ntpq_varlist *pvl,
	int associd,
	int rstatus,
	size_t datalen,
	const char *data,
	int af
	);

static long
when(
        l_fp *ts,
        l_fp *rec,
        l_fp *reftime
        );
static char *
prettyinterval(
        char *buf,
        size_t cb,
        long diff
        );

int init_ntpq()
{
	int nrst;

	nrst = ntpq_openhost("localhost", AF_INET);

	init_systime();

	return 1;
}

/*
int ntpqmod_getpeers()
{
	int i;
	char fullname[LENHOSTNAME];
	sockaddr_u netnum;

	int nrst;
	int numassoc;
	unsigned short associds[NTPQ_BUFLEN] = {0};

	numassoc = ntpq_read_associations(associds, NTPQ_BUFLEN);

	printf("       remote     refid           st t when poll reach   delay   offset    jitter\n");
	printf("===================================================================================\n");
	for (i = 0; i < numassoc; i++) {
		dogetpeers(associds[i]);
	}
}
*/

int ntpqmod_readpeer(ntpqmod_peerinfo_t *pinfo, int associd)
{
	const char *datap;
	int res;
	size_t dsize;
	unsigned short rstatus;

	memset(pinfo, 0, sizeof(ntpqmod_peerinfo_t));

	res = ntpq_doquerylist(my_peervarlist, CTL_OP_READVAR, associd, 0, &rstatus,
			  &dsize, &datap);

	if (res != 0)
		return 0;

	if (dsize == 0) {
		fprintf(stderr, "***No information returned for association %u\n", associd);
		return 0;
	}

	return doreadpeer(pinfo, my_peervarlist, associd, (int)rstatus, dsize, datap, AF_INET);
}

int
doreadpeer(
	ntpqmod_peerinfo_t *pinfo,
	const struct ntpq_varlist *pvl,
	int associd,
	int rstatus,
	size_t datalen,
	const char *data,
	int af
	)
{
	char ntpvalue[NTPQ_BUFLEN];
	size_t len;
	l_fp ts;
	long poll_sec;

	static const char flash2[] = " .+*    "; /* flash decode for version 2 */
	static const char flash3[] = " x.-+#*o"; /* flash decode for peer status version 3 */

	u_char pktversion = NTP_OLDVERSION + 1;
	
	memset(pinfo, 0, sizeof(ntpqmod_peerinfo_t));
	pinfo->c = ' ';

	get_systime(&ts);

	if (pktversion > NTP_OLDVERSION)
		pinfo->c = flash3[CTL_PEER_STATVAL(rstatus) & 0x7];
	else
		pinfo->c = flash2[CTL_PEER_STATVAL(rstatus) & 0x3];

	len = ntpq_getvar(data, datalen, "srcadr", ntpvalue, NTPQ_BUFLEN);
	if(len <= 0)
		len = ntpq_getvar(data, datalen, "peeradr", ntpvalue, NTPQ_BUFLEN);
	if(len > 0)
	{
		if(!decodenetnum(ntpvalue, &pinfo->srcadr))
		{
			printf("ERROR\n");
		}
	}
	len = ntpq_getvar(data, datalen, "srchost", ntpvalue, NTPQ_BUFLEN);
	if(len > 0)
	{
		strncpy(pinfo->srchost, ntpvalue, LENHOSTNAME - 1);
		pinfo->srchost[LENHOSTNAME] = 0;
	}
	len = ntpq_getvar(data, datalen, "dstadr", ntpvalue, NTPQ_BUFLEN);
	if(len > 0)
	{
		if(!decodenetnum(ntpvalue, &pinfo->dstadr))
		{
			printf("ERROR\n");
		}
	}
	len = ntpq_getvar(data, datalen, "hmode", ntpvalue, NTPQ_BUFLEN);
	if(len > 0)
	{
		decodeint(ntpvalue, &pinfo->hmode);

		switch (pinfo->hmode) {
		case MODE_BCLIENT:
			/* broadcastclient or multicastclient */
			pinfo->hmode_c = 'b';
			break;
		case MODE_BROADCAST:
			/* broadcast or multicast server */
			if (IS_MCAST(&pinfo->srcadr))
				pinfo->hmode_c = 'M';
			else
				pinfo->hmode_c = 'B';
			break;
		case MODE_CLIENT:
			if (ISREFCLOCKADR(&pinfo->srcadr))
				pinfo->hmode_c = 'l';     /* local refclock*/
			else if (SOCK_UNSPEC(&pinfo->srcadr))
				pinfo->hmode_c = 'p';     /* pool */
			else if (IS_MCAST(&pinfo->srcadr))
				pinfo->hmode_c = 'a';     /* manycastclient */
			else
				pinfo->hmode_c = 'u';     /* unicast */
			break;
		case MODE_ACTIVE:
			pinfo->hmode_c = 's';             /* symmetric active */
			break;                  /* configured */
		case MODE_PASSIVE:
			pinfo->hmode_c = 'S';             /* symmetric passive */
			break;                  /* ephemeral */
		}
	}
	len = ntpq_getvar(data, datalen, "refid", ntpvalue, NTPQ_BUFLEN);
	if(len > 0)
	{
		if (0 == len) {
			pinfo->refid[0] = 0;
			pinfo->have_refid = 0;
		} else if (len <= 4) {
			strncpy(pinfo->refid, ntpvalue, len);
			pinfo->refid[len] = 0;
			pinfo->have_refid = 1;
		} else if (decodenetnum(ntpvalue, &pinfo->refidadr)) {
			if (SOCK_UNSPEC(&pinfo->refidadr))
				strncpy(pinfo->refid, "0.0.0.0", sizeof(pinfo->refid) - 1);
			else if (ISREFCLOCKADR(&pinfo->refidadr))
			{
				if((pinfo->refidadr.sa.sa_family == PF_INET) || (pinfo->refidadr.sa.sa_family == AF_INET))
					inet_ntop(pinfo->refidadr.sa.sa_family, (void*)&pinfo->refidadr.sa4.sin_addr, pinfo->refid, sizeof(pinfo->refidadr));
				else if((pinfo->refidadr.sa.sa_family == PF_INET6) || (pinfo->refidadr.sa.sa_family == AF_INET6))
					inet_ntop(pinfo->refidadr.sa.sa_family, (void*)&pinfo->refidadr.sa6.sin6_addr, pinfo->refid, sizeof(pinfo->refidadr));
			}
			else
			{
				if((pinfo->refidadr.sa.sa_family == PF_INET) || (pinfo->refidadr.sa.sa_family == AF_INET))
					inet_ntop(pinfo->refidadr.sa.sa_family, (void*)&pinfo->refidadr.sa4.sin_addr, pinfo->refid, sizeof(pinfo->refidadr));
				else if((pinfo->refidadr.sa.sa_family == PF_INET6) || (pinfo->refidadr.sa.sa_family == AF_INET6))
					inet_ntop(pinfo->refidadr.sa.sa_family, (void*)&pinfo->refidadr.sa6.sin6_addr, pinfo->refid, sizeof(pinfo->refidadr));
			}
			pinfo->have_refid = 2;
		} else {
			pinfo->have_refid = 0;
		}
	}
	len = ntpq_getvar(data, datalen, "stratum", ntpvalue, NTPQ_BUFLEN);
	if(len > 0)
	{
		decodeuint(ntpvalue, &pinfo->stratum);
	}
	len = ntpq_getvar(data, datalen, "hpoll", ntpvalue, NTPQ_BUFLEN);
	if(len > 0)
	{
		if(decodeint(ntpvalue, &pinfo->hpoll) && pinfo->hpoll < 0)
			pinfo->hpoll = NTP_MINPOLL;
	}
	len = ntpq_getvar(data, datalen, "ppoll", ntpvalue, NTPQ_BUFLEN);
	if(len > 0)
	{
		if(decodeint(ntpvalue, &pinfo->ppoll) && pinfo->ppoll < 0)
			pinfo->ppoll = NTP_MINPOLL;
	}
	len = ntpq_getvar(data, datalen, "reach", ntpvalue, NTPQ_BUFLEN);
	if(len > 0)
	{
		decodeuint(ntpvalue, &pinfo->reach);
	}
	len = ntpq_getvar(data, datalen, "delay", ntpvalue, NTPQ_BUFLEN);
	if(len > 0)
	{
		decodetime(ntpvalue, &pinfo->estdelay);
	}
	len = ntpq_getvar(data, datalen, "offset", ntpvalue, NTPQ_BUFLEN);
	if(len > 0)
	{
		decodetime(ntpvalue, &pinfo->estoffset);
	}
	len = ntpq_getvar(data, datalen, "jitter", ntpvalue, NTPQ_BUFLEN);
	if(len > 0)
	{
		if (decodetime(ntpvalue, &pinfo->estjitter))
			pinfo->have_jitter = 1;
	}
	len = ntpq_getvar(data, datalen, "rootdisp", ntpvalue, NTPQ_BUFLEN);
	if(len <= 0)
		len = ntpq_getvar(data, datalen, "dispersion", ntpvalue, NTPQ_BUFLEN);
	if(len > 0)
	{
		decodetime(ntpvalue, &pinfo->estdisp);
	}
	len = ntpq_getvar(data, datalen, "rec", ntpvalue, NTPQ_BUFLEN);
	if(len > 0)
	{
		decodets(ntpvalue, &pinfo->rec);
	}
	len = ntpq_getvar(data, datalen, "srcport", ntpvalue, NTPQ_BUFLEN);
	if(len > 0)
	{
		decodeuint(ntpvalue, &pinfo->srcport);
	}
	len = ntpq_getvar(data, datalen, "reftime", ntpvalue, NTPQ_BUFLEN);
	if(len > 0)
	{
		if (!decodets(ntpvalue, &pinfo->reftime))
			L_CLR(&pinfo->reftime);
	}

	poll_sec = 1 << min(pinfo->ppoll, pinfo->hpoll);
	prettyinterval(pinfo->whenbuf, sizeof(pinfo->whenbuf), when(&ts, &pinfo->rec, &pinfo->reftime));
	prettyinterval(pinfo->pollbuf, sizeof(pinfo->pollbuf), (int)poll_sec);

	return 1;
}


// reference by ntp
/*
 * when - print how long its been since his last packet arrived
 */
static long
when(
        l_fp *ts,
        l_fp *rec,
        l_fp *reftime
        )
{
        l_fp *lasttime;

        if (rec->l_ui != 0)
                lasttime = rec;
        else if (reftime->l_ui != 0)
                lasttime = reftime;
        else
                return 0;

        return (ts->l_ui - lasttime->l_ui);
}

/*
 * Pretty-print an interval into the given buffer, in a human-friendly format.
 */
static char *
prettyinterval(
        char *buf,
        size_t cb,
        long diff
        )
{
        if (diff <= 0) {
                buf[0] = '-';
                buf[1] = 0;
                return buf;
        }

        if (diff <= 2048) {
                snprintf(buf, cb, "%ld", diff);
                return buf;
        }

        diff = (diff + 29) / 60;
        if (diff <= 300) {
                snprintf(buf, cb, "%ldm", diff);
                return buf;
        }

        diff = (diff + 29) / 60;
        if (diff <= 96) {
                snprintf(buf, cb, "%ldh", diff);
                return buf;
        }

        diff = (diff + 11) / 24;
        snprintf(buf, cb, "%ldd", diff);
        return buf;
}

