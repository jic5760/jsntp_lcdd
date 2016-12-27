#pragma once

#include <stdarg.h>

#include "JsEmbeddedUtils/I2CDevice.h"

class I2CLCD
{
private:
	JsEmbeddedUtils::I2CDevice m_i2cDevice;

	int m_numOflines;
	int m_numOfcols;
	unsigned char m_displayfunction;
	unsigned char m_displaycontrol;
	unsigned char m_displaymode;
	unsigned char m_backlightval;

public:
	I2CLCD();
	~I2CLCD();
	
	int busOpen(const char *szDevPath);
	void busClose();

	int init(unsigned char slaveaddr, int cols, int lines, int dotsize);

	int getLines();
	int getCols();

	int backlight(bool on);
	int clear();
	int home();
	int display();
	int display(bool on);
	int gotoxy(int y, int x);

	int putchar(char c);
	int printf(const char *format, ...);
	int puts_newline(int y, const char *text);

	int expanderWrite(unsigned char value);
	int write4bits(unsigned char data);
	int pulseEnable(unsigned char data);
	int command(unsigned char value);
	int send(unsigned char value, unsigned char mode);
};

