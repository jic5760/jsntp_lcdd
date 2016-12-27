#pragma once

#include <linux/i2c.h>

namespace JsEmbeddedUtils
{

	class I2CDevice
	{
	private:
		int m_dev_fd;
		unsigned char m_slave_addr;

	public:
		I2CDevice();
		virtual ~I2CDevice();

		int busOpen(const char *szDevPath);
		void busClose();

		int setSlaveId(unsigned char addr);
		unsigned char getSlaveId();

		int trasnfer(i2c_msg *pmsgs, int nmsgs, bool autosetslaveaddr = false);
	};

}
