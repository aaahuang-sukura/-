/*******************************************************
* 文件名：main.h
********************************************************/
#include <reg51.h>
#include <intrins.h>

#ifndef _MAIN_H
#define _MAIN_H

#define uint	unsigned int 
#define uchar	unsigned char 

//自定义时间结构体
typedef struct __system_time{
	uchar year;
	uchar month;
	uchar date;
	uchar hour;
	uchar min;
	uchar sec;
	uchar day;	//星期
}SYSTEM_TIME;



void get_temper(uchar *pBuff);
void get_temper_updown(uchar *pDown, uchar *pUp);
void get_system_time(uchar *pBuff);
void get_moto_stat(uchar *pStat);
void get_heat_stat(uchar *pStat);
void set_temper_updown(uchar down, uchar up);
void set_system_time(uchar *pBuff);
void open_moto(uchar open);
void open_heat(uchar open);


uchar normal_temper(uchar int_temper, uchar down_temper, uchar up_temper);

#endif