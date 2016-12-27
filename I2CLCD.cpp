#include "I2CLCD.h"

#include <unistd.h>

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00



#define En 0x04  // Enable bit
#define Rw 0x02  // Read/Write bit
#define Rs 0x01  // Register select bit

I2CLCD::I2CLCD()
{
	m_backlightval = LCD_NOBACKLIGHT;
}


I2CLCD::~I2CLCD()
{
}

int I2CLCD::busOpen(const char *szDevPath)
{
	return m_i2cDevice.busOpen(szDevPath);
}

void I2CLCD::busClose()
{
	m_i2cDevice.busClose();
}

int I2CLCD::init(unsigned char slaveaddr, int cols, int lines, int dotsize)
{
	int nrst;
	
	m_displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;

	m_numOfcols = cols;
	m_numOflines = lines;

	if(lines > 1)
		m_displayfunction = (m_displayfunction & (~LCD_1LINE)) | LCD_2LINE;
	if((dotsize != 0) && (lines == 1))
		m_displayfunction = (m_displayfunction & (~LCD_5x8DOTS)) | LCD_5x10DOTS;

	nrst = m_i2cDevice.setSlaveId(slaveaddr);
	if(nrst != 1)
		return nrst;

	usleep(50);

	expanderWrite(0);
	usleep(1000);

	// we start in 8bit mode, try to set 4 bit mode
	write4bits(0x03 << 4);
	usleep(4500); // wait min 4.1ms

	// second try
	write4bits(0x03 << 4);
	usleep(4500); // wait min 4.1ms

	// third go!
	write4bits(0x03 << 4); 
	usleep(150);

	// finally, set to 4-bit interface
	write4bits(0x02 << 4);

	command(LCD_FUNCTIONSET | m_displayfunction);

	// turn the display on with no cursor or blinking default
	m_displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
	display();

	clear();

	// Initialize to default text direction (for roman languages)
	m_displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
	
	// set the entry mode
	command(LCD_ENTRYMODESET | m_displaymode);

	home();

	return 1;
}

int I2CLCD::getLines()
{
	return m_numOflines;
}

int I2CLCD::getCols()
{
	return m_numOfcols;
}

int I2CLCD::backlight(bool on)
{
	m_backlightval = on ? LCD_BACKLIGHT : LCD_NOBACKLIGHT;
	return expanderWrite(m_backlightval);
}

int I2CLCD::clear()
{
	int nrst = command(LCD_CLEARDISPLAY); // clear display, set cursor position to zero
	usleep(2000);
	return nrst;
}

int I2CLCD::home()
{
	int nrst = command(LCD_RETURNHOME);  // set cursor position to zero
	if(nrst == 1)
		usleep(2000);  // this command takes a long time!
	return nrst;
}

int I2CLCD::display()
{
	return command(LCD_DISPLAYCONTROL | m_displaycontrol);
}

int I2CLCD::display(bool on)
{
	if(on)
		m_displaycontrol |= LCD_DISPLAYON;
	else
		m_displaycontrol &= ~LCD_DISPLAYON;
	return command(LCD_DISPLAYCONTROL | m_displaycontrol);
}

int I2CLCD::gotoxy(int y, int x)
{
	unsigned char address;

	switch(y)
	{
	case 0:
		address = 0x00;
		break;
	case 1:
		address = 0x40;
		break;
	case 2:
		address = 0x14;
		break;
	case 3:
		address = 0x54;
		break;
	}

	address += x;

	send(0x80 | address, 0);
}

int I2CLCD::putchar(char c)
{
	return send((unsigned char)c, Rs);
}

int I2CLCD::printf(const char *format, ...)
{
	va_list arg;
	int done;
	char strbuf[128] = {0};
	size_t i;
	size_t len;
	int nrst;

	va_start(arg, format);
	done = vsnprintf(strbuf, sizeof(strbuf), format, arg);
	va_end (arg);

	len = strlen(strbuf);
	for(i=0; i<len; i++)
	{
		nrst = putchar(strbuf[i]);
	}

	return done;
}

int I2CLCD::puts_newline(int y, const char *text)
{
	int i;
	char buf[64];
	int n;
	gotoxy(y, 0);
	n = strlen(text);
	for(i=0; i < n && i < m_numOfcols; i++)
		putchar(text[i]);
	for(; i<m_numOfcols; i++)
		putchar(' ');
}

int I2CLCD::write4bits(unsigned char value)
{
	int nrst = expanderWrite(value);
	if(nrst == 1)
		nrst = pulseEnable(value);
	return nrst;
}

int I2CLCD::expanderWrite(unsigned char value)
{
	i2c_msg msg;
	unsigned char data;

	data = value | m_backlightval;

	msg.addr = 0;
	msg.buf = &data;
	msg.len = 1;
	msg.flags = 0; // write

	return m_i2cDevice.trasnfer(&msg, 1, true);
}

int I2CLCD::pulseEnable(unsigned char data)
{
	int nrst;
	
	do {
		nrst = expanderWrite(data | En);	// En high
		if(nrst != 1)
			break;
		usleep(1);		// enable pulse must be >450ns

		nrst = expanderWrite(data & ~En);	// En low
		if(nrst != 1)
			break;
		usleep(50);		// commands need > 37us to settle
	}while(0);

	return nrst;
}

int I2CLCD::command(unsigned char value)
{
	return send(value, 0);
}

int I2CLCD::send(unsigned char value, unsigned char mode)
{
	int nrst;
	unsigned char highnib=value&0xf0;
	unsigned char lownib=(value<<4)&0xf0;
	nrst = write4bits(highnib | mode);
	if(nrst == 1)
		nrst = write4bits(lownib  | mode); 
	return nrst;
}
