/*****************************************************************************
* 文件名：ds1302.c
* 说  明：操作DS1302的基本函数
*****************************************************************************/
#include "main.h"
#include "ds1302.h"
 
//DS1302的引脚定义
sbit DS_RST = P3^5;
sbit DS_SCLK = P3^6;
sbit DS_IO = P3^4;

#define BCD2DEC(X)	(((X&0x70)>>4)*10 + (X&0x0F))	//用于将BCD码转成十进制
#define DEC2BCD(X)	((X/10)<<4 | (X%10))			//用于将十进制转成BCD码

/******************************************************
* 函  数：初始化DS1302的引脚
* 参  数：空
******************************************************/
void ds1302_init(void)
{
	DS_RST = 0;
	DS_SCLK = 0;
	DS_IO = 0;
}

/******************************************************
* 函  数：写单字节到DS1302
* 参  数：dat要写的数据
******************************************************/
void write_byte(uchar dat)
{
	uchar i = 0;

	for(i=0; i<8; i++)
	{
		DS_SCLK = 0;
		DS_IO = (dat&1);
		_nop_(),_nop_();
		DS_SCLK = 1;		//上升沿发出数据
		_nop_(),_nop_();
		dat >>= 1;
	}
}

/******************************************************
* 函  数：从DS1302读单字节
* 参  数：空
* 返回值：返回读取的字节（10进制）
******************************************************/
uchar read_byte(void)
{
	uchar i = 0;
	uchar dat = 0;
	uchar tmp = 0;	
//	i = (i++)%10;

	for(i=0; i<8; i++)
	{ 		
		DS_SCLK = 1;   	
		_nop_(),_nop_();			
		DS_SCLK = 0;		//下降沿读出数据
		_nop_(),_nop_();
		tmp = DS_IO;
		dat >>= 1;			//先读的是低位，移位为低位
		dat |= (tmp<<7);
	}

	dat = BCD2DEC(dat);			//BCD转换
	return(dat);
}

/******************************************************
* 函  数：从DS1302的指定位置读数据
* 参  数：addr要读取数据的控制字（地址/命令）
* 返回值：返回读取的数字（10进制）
******************************************************/
uchar read_ds1302(uchar addr)
{
	uchar tmp;

	DS_RST = 0;
	DS_SCLK = 0;
	DS_RST = 1;
	write_byte(addr);
	tmp = read_byte();
	DS_RST = 0;
	DS_SCLK = 0;
	DS_IO = 0;

	return(tmp);
}

/******************************************************
* 函  数：写数据到DS1302的指定位置
* 参  数：addr要写入数据的控制字（地址/命令）
*         dat要写的数据
******************************************************/
void write_ds1302(uchar addr, uchar dat)
{
	uchar tmp = 0;
	tmp = DEC2BCD(dat);

	DS_RST = 0;
	DS_SCLK = 0;
	DS_RST = 1;
	write_byte(addr);
	write_byte(tmp);
	DS_RST = 0;
	DS_SCLK = 0;
}

/******************************************************
* 函  数：读取年、月、日、时、分、秒、星期
* 参  数：无
* 返回值：str_time读取到的时间
******************************************************/
SYSTEM_TIME read_time(void)
{
	SYSTEM_TIME str_time;

	str_time.year = read_ds1302(DS1302_YEAR|0x01);
	str_time.month = read_ds1302(DS1302_MONTH|0x01);
	str_time.date = read_ds1302(DS1302_DATE|0x01);

	str_time.hour = read_ds1302(DS1302_HOUR|0x01);
	str_time.min = read_ds1302(DS1302_MIN|0x01);
	str_time.sec = read_ds1302(DS1302_SEC|0x01);

	str_time.day = read_ds1302(DS1302_DAY|0x01); 
	return(str_time);
}

/******************************************************
* 函  数：设置DS1302的年、月、日、时、分、秒、星期
* 参  数：str_time要设定的时间
******************************************************/
void set_time(SYSTEM_TIME str_time)
{	
	write_ds1302(0x8E, 0x00);
	write_ds1302(DS1302_YEAR, str_time.year);  	
	write_ds1302(DS1302_MONTH, str_time.month);	
	write_ds1302(DS1302_DATE, str_time.date);  
	write_ds1302(DS1302_HOUR, str_time.hour);  
	write_ds1302(DS1302_MIN, str_time.min);	  
	write_ds1302(DS1302_SEC, str_time.sec);	  
	write_ds1302(DS1302_DAY, str_time.day);	
	write_ds1302(0x8E, 0x80);  	
}



