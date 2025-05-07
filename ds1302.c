/*****************************************************************************
* �ļ�����ds1302.c
* ˵  ��������DS1302�Ļ�������
*****************************************************************************/
#include "main.h"
#include "ds1302.h"
 
//DS1302�����Ŷ���
sbit DS_RST = P3^5;
sbit DS_SCLK = P3^6;
sbit DS_IO = P3^4;

#define BCD2DEC(X)	(((X&0x70)>>4)*10 + (X&0x0F))	//���ڽ�BCD��ת��ʮ����
#define DEC2BCD(X)	((X/10)<<4 | (X%10))			//���ڽ�ʮ����ת��BCD��

/******************************************************
* ��  ������ʼ��DS1302������
* ��  ������
******************************************************/
void ds1302_init(void)
{
	DS_RST = 0;
	DS_SCLK = 0;
	DS_IO = 0;
}

/******************************************************
* ��  ����д���ֽڵ�DS1302
* ��  ����datҪд������
******************************************************/
void write_byte(uchar dat)
{
	uchar i = 0;

	for(i=0; i<8; i++)
	{
		DS_SCLK = 0;
		DS_IO = (dat&1);
		_nop_(),_nop_();
		DS_SCLK = 1;		//�����ط�������
		_nop_(),_nop_();
		dat >>= 1;
	}
}

/******************************************************
* ��  ������DS1302�����ֽ�
* ��  ������
* ����ֵ�����ض�ȡ���ֽڣ�10���ƣ�
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
		DS_SCLK = 0;		//�½��ض�������
		_nop_(),_nop_();
		tmp = DS_IO;
		dat >>= 1;			//�ȶ����ǵ�λ����λΪ��λ
		dat |= (tmp<<7);
	}

	dat = BCD2DEC(dat);			//BCDת��
	return(dat);
}

/******************************************************
* ��  ������DS1302��ָ��λ�ö�����
* ��  ����addrҪ��ȡ���ݵĿ����֣���ַ/���
* ����ֵ�����ض�ȡ�����֣�10���ƣ�
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
* ��  ����д���ݵ�DS1302��ָ��λ��
* ��  ����addrҪд�����ݵĿ����֣���ַ/���
*         datҪд������
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
* ��  ������ȡ�ꡢ�¡��ա�ʱ���֡��롢����
* ��  ������
* ����ֵ��str_time��ȡ����ʱ��
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
* ��  ��������DS1302���ꡢ�¡��ա�ʱ���֡��롢����
* ��  ����str_timeҪ�趨��ʱ��
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



