/*****************************************************************************
* 文件名：serial.c
* 说  明：串口通信有关的函数
*****************************************************************************/
#include "main.h"

uchar g_pSendBuff[13];		//发送缓存
uchar g_pRecvBuff[14];		//接收缓存

uchar g_ucRecvNum;			//接收字节数

/******************************************************
* 函  数：定时器1初始化
* 参  数：空
* 返回值：无
******************************************************/
void timer1_int(void)
{	
	TMOD |= 0x20;		//定时器T1，工作方式2
	TL1 = 0xFD;			//定时器T1初始值253（0xFD）
	TH1 = 0xFD;
	TR1 = 1;			//定时器T1启动工作（TCON | 0x40）
} 

/******************************************************
* 函  数：串口初始化，设定串口工作方式、中断
* 参  数：空
* 返回值：无
******************************************************/
void serial_int(void)
{
	PCON = 0x80;		//设定SMOD位为1
	SCON = 0x51;		//设定SM0=0,SM1=1,串口工作方式1，REN=1接收数据使能
	ES = 1;				//使能串口中断
}

/******************************************************
* 函  数：串口中断函数
* 参  数：空
* 返回值：无
******************************************************/
void serial_inter() interrupt 4
{	
	static uchar sSendNum = 0;
	uchar tmp = 0;  
	
	//串口接收中断
	if(RI == 1)
	{	
		RI = 0;
		tmp = SBUF;
		if(g_ucRecvNum==0 && tmp == 0xEB)
		{
			g_pRecvBuff[0] = tmp;
			g_ucRecvNum ++;
		}else
		{
			if(g_ucRecvNum > 0)
			{
				 g_pRecvBuff[g_ucRecvNum] = tmp;
				 g_ucRecvNum ++;
			}
		}
	}

	//串口发送中断
	if(TI == 1)
	{
		TI = 0;
		sSendNum ++;
		if(sSendNum < g_pSendBuff[1])
		{
			SBUF = g_pSendBuff[sSendNum];
		}else
		{
			sSendNum = 0;
		}
	}
}  

/******************************************************
* 函  数：通信协议处理函数
* 参  数：空
* 返回值：无
******************************************************/
void deal_protocol(void)
{
	uchar chk_sum = 0;
	uchar len = 0;
	uchar cmd = 0;
	uchar i = 0;

	if(g_ucRecvNum >= 6)		//协议长度最短是6字节
	{	
		len = g_pRecvBuff[1];	//数据长度
		if(g_ucRecvNum < len)
		{
			return;
		} 
		g_ucRecvNum = 0;		//已经接收完整一帧数据

		
		if(g_pRecvBuff[len-1] != 0x90)	//检验协议尾
		{
			return;
		}
		 
		//检验校验和
		for(i=1; i<len-2; i++)
		{
			chk_sum += g_pRecvBuff[i];
		}
		if(chk_sum != g_pRecvBuff[len-2])
		{
			return;
		}
		
		if(g_pRecvBuff[3] == 0x01)		//控制命令		
		{
			cmd = g_pRecvBuff[2];
			switch(cmd)
			{
			case 2:		//温度上下限
				set_temper_updown(g_pRecvBuff[4], g_pRecvBuff[5]);//
				break;
			case 3:		//时间
				set_system_time((uchar*)(g_pRecvBuff+4));
				break;
			case 4:		//风扇
				open_moto(g_pRecvBuff[4]);
				break;
			case 5:		//加热灯
				open_heat(g_pRecvBuff[4]);
				break;
			default :
				break;
			}
			g_pSendBuff[1] = 0x07;
			g_pSendBuff[2] = cmd;
			g_pSendBuff[3] = 0x01;	//控制
			g_pSendBuff[4] = 0x00;	//成功
		}else	 //查询命令字
		{
			cmd = g_pRecvBuff[2];
			switch(cmd)
			{
			case 1:		//温度
				get_temper((uchar*)(g_pSendBuff+5));
				g_pSendBuff[1] = 9;
				break;
			case 2:		//温度上下限
				get_temper_updown((uchar*)(g_pSendBuff+5),(uchar*)(g_pSendBuff+6));//
				g_pSendBuff[1] = 9;
				break;
			case 3:		//时间
				get_system_time((uchar*)(g_pSendBuff+5));
				g_pSendBuff[1] = 14;
				break;
			case 4:		//风扇
				get_moto_stat((uchar*)(g_pSendBuff+5));
				g_pSendBuff[1] = 8;
				break;
			case 5:		//加热灯
				get_heat_stat((uchar*)(g_pSendBuff+5));
				g_pSendBuff[1] = 8;
				break;
			default :
				break;
			}
			g_pSendBuff[2] = cmd;
			g_pSendBuff[3] = 0x00;	//读取
			g_pSendBuff[4] = 0x00;	//成功
		}

		//应答数据
		g_pSendBuff[0] = 0xEB;
		//g_pSendBuff[1] = 0x06;
		len = g_pSendBuff[1];
		chk_sum = 0;
		for(i=1; i<len-2; i++)
		{
			chk_sum += g_pSendBuff[i];
		}
		g_pSendBuff[len-2] = chk_sum;
		g_pSendBuff[len-1] = 0x90;
		SBUF = g_pSendBuff[0];
	}
}