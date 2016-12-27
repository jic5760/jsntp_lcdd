#include "I2CDevice.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/poll.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/i2c-dev.h>

namespace JsEmbeddedUtils
{

	I2CDevice::I2CDevice()
	{
		m_dev_fd = -1;
		m_slave_addr = 0;
	}


	I2CDevice::~I2CDevice()
	{
		busClose();
	}

	int I2CDevice::busOpen(const char *szDevPath)
	{
		m_dev_fd = ::open(szDevPath, O_RDWR);
		if(m_dev_fd == -1)
		{
			return -errno;
		}
		return 1;
	}

	void I2CDevice::busClose()
	{
		if(m_dev_fd != -1)
		{
			close(m_dev_fd);
			m_dev_fd = -1;
		}
	}

	int I2CDevice::setSlaveId(unsigned char addr)
	{
		int nrst = ioctl(m_dev_fd, I2C_SLAVE, addr);
		if(nrst < 0)
			return -errno;
		m_slave_addr = addr;
		return 1;
	}
	
	unsigned char I2CDevice::getSlaveId()
	{
		return m_slave_addr;
	}

	int I2CDevice::trasnfer(i2c_msg *pmsgs, int nmsgs, bool autosetslaveaddr)
	{
		int i;
		int nrst;

		i2c_rdwr_ioctl_data data;
		data.msgs = pmsgs;
		data.nmsgs = nmsgs;
		for(i=0; i<nmsgs; i++)
		{
			pmsgs[i].addr = m_slave_addr;
		}
		nrst = ioctl(m_dev_fd, I2C_RDWR, (struct i2c_rdwr_ioctl_data *)&data);
		if(nrst < 0)
		{
			return -errno;
		}
		return 1;
	}

}
