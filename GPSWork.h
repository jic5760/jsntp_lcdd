#pragma once

#include <stdlib.h>
#include <pthread.h>
#include <gps.h>

#include "JsCPPUtils/AtomicNum.h"
#include "JsCPPUtils/LockableEx.h"

class GPSWork
{
public:
	class Data : public JsCPPUtils::LockableEx {
	public:
		struct gps_data_t gpsdata;

		void setData(const struct gps_data_t *pdata);
		
		int getUsedSatellitesCount();
		int getVisibleSatellitesCount();
	};

private:
	pthread_t                  m_worker_thread;
	JsCPPUtils::AtomicNum<int> m_worker_run;
	int m_gpsconn_status;

public:
	Data m_last_data;

public:
	GPSWork();
	~GPSWork();

	static void *workerThreadProc(void *param);
};

