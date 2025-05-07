/*******************************************************
* 文件名：ds1302.h
********************************************************/
#ifndef _DS1302_H
#define _DS1302_H

#include "main.h"

//DS1302控制字（地址/命令）
#define DS1302_SEC		0x80
#define DS1302_MIN		0x82
#define DS1302_HOUR		0x84
#define DS1302_DATE		0x86
#define DS1302_MONTH	0x88
#define DS1302_DAY		0x8A
#define DS1302_YEAR		0x8C
#define DS1302_WRITE	0x8E
#define DS1302_POWER	0x90

//DS1302写保护
#define DS1302_ENABLE	0x00	//写使能
#define DS1302_DISABLE	0x80	//写保护


void ds1302_init(void);
void write_ds1302(uchar addr, uchar dat);
uchar read_ds1302(uchar addr);

SYSTEM_TIME read_time(void);
void set_time(SYSTEM_TIME str_time);

#endif