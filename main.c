/*****************************************************************************
* �ļ�����main.c
* ˵  ������AT89C51��DS18B20��DS1302��24C02��AMPIRE128X64����������ݡ�������
*       ledָʾ�Ƶ�ʵ��һ�������¶ȿ����Զ�����ϵͳ��
*		��Ҫ�����ǿ�����ʾʱ�䣨�ꡢ�¡��ա�ʱ���֡��룩����ǰ�¶ȡ��¶������ޡ�
*       �¶��Ƿ���������Χ������ͨ���������Ʒ���ͼ��ȵƣ��÷����������¶ȳ���
*       �����ޡ�ͬʱ��LEDָʾ��ָʾ������ͨ�������޸�ʱ�䡢�¶������ޣ����Ʒ��
*       ���ȵƵȡ�
* ��  �ߣ�����
* ʱ  ��:2011.6.1���    2011.6.9���  2015.9.16����
*****************************************************************************/
#include "main.h"
#include "delay.h"
#include "ds1302.h"
#include "KS0108.h"
#include "IIC.h"
#include "ds18B20.h"
#include "serial.h"

//�˿ڶ���
sbit SPEAK = P1^0;			//���������ƹܽ�
sbit LED_NOR = P1^3;		//����ָʾ�ƿ��ƹܽ�
sbit LED_OVER = P1^4;		//�¶�����ָʾ�ƿ��ƹܽ�
sbit LED_LOW = P1^5;		//�¶�����ָʾ�ƿ��ƹܽ�
sbit MOTO = P1^2;			//������ƹܽ�
sbit HEAT = P1^1;			//�����豸���ƹܽ�
//sbit RXD = P3^0;			//reg51.h���Ѿ�����
//sbit TXD = P3^1;			//reg51.h���Ѿ�����
sbit K_MOTO = P3^2;			//���Ʒ������
sbit K_HEAT = P3^3;			//���Ƽ����豸�İ���

#define DAT_ADDR	0x03		//IIC EEPROM�б�����¶��ϡ����޵ĵ�ַ

uchar g_pc_cont;		//pc���ƣ�0û���ƣ�1���ȿ���2���ȹأ�4���ȿ���8���ȹأ�

/******************************************************
* ��  �����ⲿ0�жϣ��趨���ж���Ӧ
* ��  ������
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
* ��  �����ⲿ1�жϣ��趨���ж���Ӧ
* ��  ������
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
* ��  ������ʱ��1�жϺ���
* ��  ������
* ����ֵ����
******************************************************/
void timer0_inter() interrupt 1
{
	//250us*4=1ms
	static uchar timer = 0;
	static uchar speaker = 0;
	if(speaker < 235)			//���Ʒ������������
	{
		if(timer++ == 3)		//���Ʊ���������
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
* ��  ����AT89C51�˿ڳ�ʼ��
* ��  ������
* ����ֵ����
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
* ��  ������ʱ��0��ʼ��
* ��  ������
* ����ֵ����
******************************************************/
void timer0_init(void)
{
	TL0 = 0x06;		//��ʱ����ʼֵ
	TH0 = 0x06;
	TMOD |= 2;		//��ʱ��ģʽ��������ʽ2
	ET0 = 1;			//��T0�ж�
}

/******************************************************
* ��  �����ⲿ�жϳ�ʼ��
* ��  ������
* ����ֵ����
******************************************************/
void int_init(void)
{
	IT0 = 1;		//�½��ش���
	EX0 = 1;		//�ⲿ�жϴ�
	IT1 = 1;
	EX1 = 1;
}

/******************************************************
* ��  ���������ŷ��ȣ��򿪡��رգ�
* ��  ����open=0�رգ�=1��
* ����ֵ����
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
* ��  �������Ƽ���ģ�飨�򿪡��رգ�
* ��  ����open=0�رգ�=1��
* ����ֵ����
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
* ��  ���������¶�������
* ��  ����down�¶�����ֵ��up�¶�����ֵ
* ����ֵ����
******************************************************/
void set_temper_updown(uchar down, uchar up)
{
	write_IIC(DAT_ADDR, down);
	delay_ms(2);
	write_IIC(DAT_ADDR+1, up);
}

/******************************************************
* ��  ��������ϵͳʱ��
* ��  ����pBuff����ϵͳʱ�䣺�ꡢ�¡��ա�ʱ���֡��롢����
* ����ֵ����
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
* ��  ������ȡ��ǰ�¶�
* ��  ����pBuff�����¶�������С��ֵ
* ����ֵ����
******************************************************/
void get_temper(uchar *pBuff)
{
	uchar pTmpBuff[2] = {0,0};
	read_temper(pTmpBuff);	  //pBuff[0]������pBuff[1]С������
	*pBuff = pTmpBuff[0];
	*(pBuff+1) = pTmpBuff[1];
}

/******************************************************
* ��  ������ȡ�¶�������
* ��  ����pDown�¶�����ֵ��pUp�¶�����ֵ
* ����ֵ����
******************************************************/
void get_temper_updown(uchar *pDown, uchar *pUp)
{
	*pDown = read_IIC(DAT_ADDR);
	*pUp = read_IIC(DAT_ADDR+1);
}

/******************************************************
* ��  ������ȡϵͳʱ��
* ��  ����pBuff����ϵͳʱ�䣺�ꡢ�¡��ա�ʱ���֡��롢����
* ����ֵ����
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
* ��  ������ȡ���״̬
* ��  ����pStat��������״̬
* ����ֵ����
******************************************************/
void get_moto_stat(uchar *pStat)
{
	*pStat = MOTO;
}

/******************************************************
* ��  ������ȡ�����豸��״̬
* ��  ����pStat��������豸��״̬
* ����ֵ����
******************************************************/
void get_heat_stat(uchar *pStat)
{
	*pStat = HEAT;
}

/******************************************************
* ��  ��������¶��Ƿ���������Χ��
* ��  ����temper��ǰ�¶ȣ�down_temper�¶����ޣ�up_temper�¶�����
* ����ֵ��0������1�������ޣ�2��������
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
* ��  �������Ƶ����¶�
* ��  ����state�¶ȷ�Χ��0������1�������ޣ�2��������
* ����ֵ����
******************************************************/
void cont_temper(uchar state)
{
	//ָʾ�ơ�������ʱ�������ȡ����
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

		TR0 = 0;	//�ر�T0��ʱ��
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

		TR0 = 1;	//����T0��ʱ��
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

		TR0 = 1;	//����T0��ʱ��
	}
}

/******************************************************
* ��  ����������������¿���ϵͳ����
* ��  ������
* ����ֵ����
******************************************************/
void main(void)
{
	SYSTEM_TIME str_time;
	uchar pbuff[2] = {0,0};		//�¶�������С������
	uchar down_temper, up_temper;	//�¶��ϡ�����
	uchar state = 0;				//��ǰ�¶�״̬���Ƿ���

	port_init();			//�˿ڳ�ʼ��
	int_init();			//�ⲿ�жϳ�ʼ��
	timer0_init();		//��ʱ��0��ʼ��

	//��ΧоƬ��ʼ��
	ds1302_init();		//DS1302�˿ڳ�ʼ��
	ks0108_init();		//LCD��ʼ��
	init_IIC();
	timer1_int();		//uart������
	serial_int();			//uart
		
	EA = 1;		//��ȫ���ж�
	//TR0 = 1;		// ��cont_temper�����п���Timer0
	
	//��ȡIIC�¶�������
	down_temper = read_IIC(DAT_ADDR);
	up_temper = read_IIC(DAT_ADDR+1);
	//�����ȡ��ֵ����0xFF���¶�������û�б����ù������趨Ĭ���¶���ֵ0��35
	if(down_temper == 0xFF && up_temper == 0xFF)
	{
		down_temper = 0;	//Ĭ���¶�����ֵ
		up_temper = 35;		//Ĭ���¶�����ֵ
		write_IIC(DAT_ADDR, down_temper);
		delay_ms(2);
		write_IIC(DAT_ADDR+1, up_temper);
	}

	temper_convert();	//�����¶�ת��	
	delay_ms(300);		

	while(1)
	{ 
		str_time = read_time();		//��ȡʱ��
		show_time(str_time);		//��ʾ�ꡢ�¡��ա�ʱ���֡��롢����

		delay_ms(500);			//ת���¶�ʱ����Ҫ750ms
		read_temper(pbuff);		//��ȡ��ǰ�¶�ֵ

		//�ж��¶��Ƿ�����
		state = normal_temper(pbuff[0], down_temper, up_temper);	//�ж��¶��Ƿ�����
		cont_temper(state);	//�����¶ȣ����ȡ����ȡ�������ָʾ��

		temper_convert();	// �����¶�ת������Ҫ750ms

		//��ʾ�¶ȡ��¶��ϡ�����ֵ
		show_temperature(pbuff[0], pbuff[1], state);		//��ʾ��ǰ�¶ȡ�״̬
		show_area(down_temper, up_temper);			//��ʾ�ϡ������¶�

		deal_protocol();		//����ͨ�Ŵ���
		delay_ms(200);

	 	//�������п����Ѿ����޸�
		down_temper = read_IIC(DAT_ADDR);
		up_temper = read_IIC(DAT_ADDR+1);
	}
}
