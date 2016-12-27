#include "GPSWork.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

GPSWork::GPSWork()
{
	int nrst;

	m_gpsconn_status = 0;
	
	m_worker_run = 1;
	nrst = pthread_create(&m_worker_thread, NULL, workerThreadProc, this);
}

GPSWork::~GPSWork()
{
	int cnt;

	m_worker_run = 2;
	for(cnt = 0; cnt < 75 && m_worker_run != 0; cnt++) // 750ms
	{
		usleep(10000); // 10ms
	}
}

void *GPSWork::workerThreadProc(void *param)
{
	GPSWork *pthis = (GPSWork*)param;

	int nrst;
	struct gps_data_t gpsctx;

	while(pthis->m_worker_run == 1)
	{
		int bErr = 0;

		usleep(100000);

		switch(pthis->m_gpsconn_status)
		{
		case 0:
			// Not connected
			nrst = gps_open("localhost", "2947", &gpsctx);
			if(nrst == -1)
			{
				break;
			}
			pthis->m_gpsconn_status = 1;
			nrst = gps_stream(&gpsctx, WATCH_ENABLE, NULL);
			break;
		case 1:
			// Connected
			if(!gps_waiting(&gpsctx, 500000))
			{
				bErr = 1;
				break;
			}
			if((nrst = gps_read(&gpsctx)) == -1)
			{
				break;
			}
			pthis->m_last_data.setData(&gpsctx);
			break;
		}
		if(bErr)
		{
			if(pthis->m_gpsconn_status == 1)
			{
				gps_close(&gpsctx);
				pthis->m_gpsconn_status = 0;
			}
		}
	}

	if(pthis->m_gpsconn_status == 1)
	{
		gps_close(&gpsctx);
		pthis->m_gpsconn_status = 0;
	}

	pthis->m_worker_run = 0;
}

void GPSWork::Data::setData(const struct gps_data_t *pdata)
{
	lock();
	memcpy(&gpsdata, pdata, sizeof(struct gps_data_t));
	unlock();
}

		
int GPSWork::Data::getUsedSatellitesCount()
{
	int i;
	int value = 0;

	lock();
	for(i=0; i<MAXCHANNELS; i++)
	{
		if(gpsdata.used[i] != 0)
			value++;
	}
	unlock();

	return value;
}

int GPSWork::Data::getVisibleSatellitesCount()
{
	int i;
	int value = 0;

	lock();
	for(i=0; i<MAXCHANNELS; i++)
	{
		if(gpsdata.PRN[i] != 0)
			value++;
	}
	unlock();

	return value;
}
