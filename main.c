/*****************************************************************************
* 文件名：main.c
* 说  明：用AT89C51、DS18B20、DS1302、24C02、AMPIRE128X64、电机、灯泡、按键、
*       led指示灯等实现一个简易温度控制自动管理系统。
*		主要功能是可以显示时间（年、月、日、时、分、秒）、当前温度、温度上下限、
*       温度是否在正常范围，可以通过按键控制风机和加热灯，用蜂鸣器报警温度超出
*       上下限、同时用LED指示灯指示，可以通过串口修改时间、温度上限限，控制风机
*       加热灯等。
* 作  者：老杨
* 时  间:2011.6.1起草    2011.6.9完成  2015.9.16完善
*****************************************************************************/
#include "main.h"
#include "delay.h"
#include "ds1302.h"
#include "KS0108.h"
#include "IIC.h"
#include "ds18B20.h"
#include "serial.h"

//端口定义
sbit SPEAK = P1^0;			//报警器控制管脚
sbit LED_NOR = P1^3;		//正常指示灯控制管脚
sbit LED_OVER = P1^4;		//温度上限指示灯控制管脚
sbit LED_LOW = P1^5;		//温度下限指示灯控制管脚
sbit MOTO = P1^2;			//风机控制管脚
sbit HEAT = P1^1;			//加热设备控制管脚
//sbit RXD = P3^0;			//reg51.h中已经定义
//sbit TXD = P3^1;			//reg51.h中已经定义
sbit K_MOTO = P3^2;			//控制风机按键
sbit K_HEAT = P3^3;			//控制加热设备的按键

#define DAT_ADDR	0x03		//IIC EEPROM中保存的温度上、下限的地址

uchar g_pc_cont;		//pc控制，0没控制，1风扇开，2风扇关，4加热开，8加热关；

/******************************************************
* 函  数：外部0中断，设定键中断响应
* 参  数：空
******************************************************/
void int0_inter() interrupt 0
{
	if(K_MOTO == 0)
	{
		MOTO = ~MOTO;
		if(MOTO == 0)
		{
			g_pc_cont &= 0x0C;
		}
	}	
}

/******************************************************
* 函  数：外部1中断，设定键中断响应
* 参  数：空
******************************************************/
void int1_inter() interrupt 2
{
	if(K_HEAT == 0)
	{
		HEAT = ~HEAT;
		g_pc_cont &= 0x03;
	}	
}

/******************************************************
* 函  数：定时器1中断函数
* 参  数：空
* 返回值：无
******************************************************/
void timer0_inter() interrupt 1
{
	//250us*4=1ms
	static uchar timer = 0;
	static uchar speaker = 0;
	if(speaker < 235)			//控制蜂鸣器声音间隔
	{
		if(timer++ == 3)		//控制报警器声音
		{
			SPEAK = ~SPEAK;
			timer = 0;
			speaker++;
		}
	}else
	{
		if(timer++ == 4)
		{
			speaker++;
		}
	}
}

/******************************************************
* 函  数：AT89C51端口初始化
* 参  数：空
* 返回值：无
******************************************************/
void port_init(void)
{
	LED_NOR = 1;
	LED_OVER = 0;
	LED_LOW = 0;
	SPEAK = 0;

	MOTO = 0;
	HEAT = 0;
}

/******************************************************
* 函  数：定时器0初始化
* 参  数：空
* 返回值：无
******************************************************/
void timer0_init(void)
{
	TL0 = 0x06;		//定时器初始值
	TH0 = 0x06;
	TMOD |= 2;		//定时器模式，工作方式2
	ET0 = 1;			//打开T0中断
}

/******************************************************
* 函  数：外部中断初始化
* 参  数：空
* 返回值：无
******************************************************/
void int_init(void)
{
	IT0 = 1;		//下降沿触发
	EX0 = 1;		//外部中断打开
	IT1 = 1;
	EX1 = 1;
}

/******************************************************
* 函  数：控制排风扇（打开、关闭）
* 参  数：open=0关闭，=1打开
* 返回值：无
******************************************************/
void open_moto(uchar open)
{
	if(open == 0)
	{
		MOTO = 0;
		g_pc_cont ^= 0x2;
	}else
	{
		MOTO = 1;
		g_pc_cont ^= 0x1;
	}
}

/******************************************************
* 函  数：控制加热模块（打开、关闭）
* 参  数：open=0关闭，=1打开
* 返回值：无
******************************************************/
void open_heat(uchar open)
{
	if(open == 0)
	{
		HEAT = 0;
		g_pc_cont ^= 0x8;
	}else
	{
		HEAT = 1;
		g_pc_cont ^= 0x4;
	}
}

/******************************************************
* 函  数：设置温度上下限
* 参  数：down温度下限值，up温度上限值
* 返回值：无
******************************************************/
void set_temper_updown(uchar down, uchar up)
{
	write_IIC(DAT_ADDR, down);
	delay_ms(2);
	write_IIC(DAT_ADDR+1, up);
}

/******************************************************
* 函  数：设置系统时间
* 参  数：pBuff保存系统时间：年、月、日、时、分、秒、星期
* 返回值：无
******************************************************/
void set_system_time(uchar *pBuff)
{
	SYSTEM_TIME str_time;
	str_time.year = pBuff[0];
	str_time.month = pBuff[1];
	str_time.date = pBuff[2];
	str_time.hour = pBuff[3];
	str_time.min = pBuff[4];
	str_time.sec = pBuff[5];
	str_time.day = pBuff[6];

	set_time(str_time);
}

/******************************************************
* 函  数：获取当前温度
* 参  数：pBuff保存温度整数、小数值
* 返回值：无
******************************************************/
void get_temper(uchar *pBuff)
{
	uchar pTmpBuff[2] = {0,0};
	read_temper(pTmpBuff);	  //pBuff[0]整数、pBuff[1]小数部分
	*pBuff = pTmpBuff[0];
	*(pBuff+1) = pTmpBuff[1];
}

/******************************************************
* 函  数：获取温度上下限
* 参  数：pDown温度下限值，pUp温度上限值
* 返回值：无
******************************************************/
void get_temper_updown(uchar *pDown, uchar *pUp)
{
	*pDown = read_IIC(DAT_ADDR);
	*pUp = read_IIC(DAT_ADDR+1);
}

/******************************************************
* 函  数：获取系统时间
* 参  数：pBuff保存系统时间：年、月、日、时、分、秒、星期
* 返回值：无
******************************************************/
void get_system_time(uchar *pBuff)
{
	SYSTEM_TIME str_time;
	str_time = read_time();
	*pBuff = str_time.year;
	*(pBuff+1) = str_time.month;
	*(pBuff+2) = str_time.date;
	*(pBuff+3) = str_time.hour;
	*(pBuff+4) = str_time.min;
	*(pBuff+5) = str_time.sec;
	*(pBuff+6) = str_time.day;
}

/******************************************************
* 函  数：获取风机状态
* 参  数：pStat保存风机的状态
* 返回值：无
******************************************************/
void get_moto_stat(uchar *pStat)
{
	*pStat = MOTO;
}

/******************************************************
* 函  数：获取加热设备的状态
* 参  数：pStat保存加热设备的状态
* 返回值：无
******************************************************/
void get_heat_stat(uchar *pStat)
{
	*pStat = HEAT;
}

/******************************************************
* 函  数：检测温度是否在正常范围内
* 参  数：temper当前温度，down_temper温度下限，up_temper温度上限
* 返回值：0正常，1超出下限，2超出上限
******************************************************/
uchar normal_temper(uchar temper, uchar down_temper, uchar up_temper)
{
	uchar state = 0;
	uchar symbol = 0;
	uchar symbol2 = 0, symbol3 = 0;

	if((temper&0x80) != 0)
	{
		symbol = 1;
	}

	if((down_temper&0x80) != 0)
	{
		symbol2 = 1;
	}
	if((temper <= down_temper) && (symbol2 == symbol))
	{
		state = 1;
	}
	if(symbol2 < symbol)
	{
		state = 1;
	}


	if((up_temper&0x80) != 0)
	{
		symbol3 = 1;
	}
	if((temper >= up_temper) && (symbol3 == symbol))
	{
		state = 2;
	}

	return(state);
}

/******************************************************
* 函  数：控制调节温度
* 参  数：state温度范围：0正常，1超出下限，2超出上限
* 返回值：无
******************************************************/
void cont_temper(uchar state)
{
	//指示灯、报警定时器、加热、风机
	if(state == 0)
	{
		LED_NOR = 1;
		LED_OVER = 0;
		LED_LOW = 0;
		if((g_pc_cont&0x0F)==0)
		{
			MOTO = 0;
			HEAT = 0;
		}

		TR0 = 0;	//关闭T0定时器
	}
	else if(state == 1)
	{
		LED_NOR = 0;
		LED_OVER = 0;
		LED_LOW = 1;
		if((g_pc_cont&0x0C)==0)
		{
			MOTO = 0;
			HEAT = 1;
		}

		TR0 = 1;	//启动T0定时器
	}
	else
	{
		LED_NOR = 0;
		LED_OVER = 1;
		LED_LOW = 0;

		if((g_pc_cont&0x03)==0)
		{
			MOTO = 1;
			HEAT = 0;
		}

		TR0 = 1;	//启动T0定时器
	}
}

/******************************************************
* 函  数：主函数：完成温控主系统功能
* 参  数：空
* 返回值：无
******************************************************/
void main(void)
{
	SYSTEM_TIME str_time;
	uchar pbuff[2] = {0,0};		//温度整数、小数部分
	uchar down_temper, up_temper;	//温度上、下限
	uchar state = 0;				//当前温度状态，是否超温

	port_init();			//端口初始化
	int_init();			//外部中断初始化
	timer0_init();		//定时器0初始化

	//外围芯片初始化
	ds1302_init();		//DS1302端口初始化
	ks0108_init();		//LCD初始化
	init_IIC();
	timer1_int();		//uart波特率
	serial_int();			//uart
		
	EA = 1;		//打开全局中断
	//TR0 = 1;		// 在cont_temper函数中控制Timer0
	
	//读取IIC温度上下限
	down_temper = read_IIC(DAT_ADDR);
	up_temper = read_IIC(DAT_ADDR+1);
	//如果读取的值都是0xFF（温度上下限没有被设置过），设定默认温度限值0、35
	if(down_temper == 0xFF && up_temper == 0xFF)
	{
		down_temper = 0;	//默认温度下限值
		up_temper = 35;		//默认温度上限值
		write_IIC(DAT_ADDR, down_temper);
		delay_ms(2);
		write_IIC(DAT_ADDR+1, up_temper);
	}

	temper_convert();	//启动温度转换	
	delay_ms(300);		

	while(1)
	{ 
		str_time = read_time();		//读取时间
		show_time(str_time);		//显示年、月、日、时、分、秒、星期

		delay_ms(500);			//转换温度时间需要750ms
		read_temper(pbuff);		//读取当前温度值

		//判断温度是否正常
		state = normal_temper(pbuff[0], down_temper, up_temper);	//判断温度是否正常
		cont_temper(state);	//控制温度：风扇、加热、报警、指示灯

		temper_convert();	// 启动温度转换，需要750ms

		//显示温度、温度上、下限值
		show_temperature(pbuff[0], pbuff[1], state);		//显示当前温度、状态
		show_area(down_temper, up_temper);			//显示上、下限温度

		deal_protocol();		//串口通信处理
		delay_ms(200);

	 	//上下限有可能已经被修改
		down_temper = read_IIC(DAT_ADDR);
		up_temper = read_IIC(DAT_ADDR+1);
	}
}
