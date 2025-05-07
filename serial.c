/*****************************************************************************
* �ļ�����serial.c
* ˵  ��������ͨ���йصĺ���
*****************************************************************************/
#include "main.h"

uchar g_pSendBuff[13];		//���ͻ���
uchar g_pRecvBuff[14];		//���ջ���

uchar g_ucRecvNum;			//�����ֽ���

/******************************************************
* ��  ������ʱ��1��ʼ��
* ��  ������
* ����ֵ����
******************************************************/
void timer1_int(void)
{	
	TMOD |= 0x20;		//��ʱ��T1��������ʽ2
	TL1 = 0xFD;			//��ʱ��T1��ʼֵ253��0xFD��
	TH1 = 0xFD;
	TR1 = 1;			//��ʱ��T1����������TCON | 0x40��
} 

/******************************************************
* ��  �������ڳ�ʼ�����趨���ڹ�����ʽ���ж�
* ��  ������
* ����ֵ����
******************************************************/
void serial_int(void)
{
	PCON = 0x80;		//�趨SMODλΪ1
	SCON = 0x51;		//�趨SM0=0,SM1=1,���ڹ�����ʽ1��REN=1��������ʹ��
	ES = 1;				//ʹ�ܴ����ж�
}

/******************************************************
* ��  ���������жϺ���
* ��  ������
* ����ֵ����
******************************************************/
void serial_inter() interrupt 4
{	
	static uchar sSendNum = 0;
	uchar tmp = 0;  
	
	//���ڽ����ж�
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

	//���ڷ����ж�
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
* ��  ����ͨ��Э�鴦����
* ��  ������
* ����ֵ����
******************************************************/
void deal_protocol(void)
{
	uchar chk_sum = 0;
	uchar len = 0;
	uchar cmd = 0;
	uchar i = 0;

	if(g_ucRecvNum >= 6)		//Э�鳤�������6�ֽ�
	{	
		len = g_pRecvBuff[1];	//���ݳ���
		if(g_ucRecvNum < len)
		{
			return;
		} 
		g_ucRecvNum = 0;		//�Ѿ���������һ֡����

		
		if(g_pRecvBuff[len-1] != 0x90)	//����Э��β
		{
			return;
		}
		 
		//����У���
		for(i=1; i<len-2; i++)
		{
			chk_sum += g_pRecvBuff[i];
		}
		if(chk_sum != g_pRecvBuff[len-2])
		{
			return;
		}
		
		if(g_pRecvBuff[3] == 0x01)		//��������		
		{
			cmd = g_pRecvBuff[2];
			switch(cmd)
			{
			case 2:		//�¶�������
				set_temper_updown(g_pRecvBuff[4], g_pRecvBuff[5]);//
				break;
			case 3:		//ʱ��
				set_system_time((uchar*)(g_pRecvBuff+4));
				break;
			case 4:		//����
				open_moto(g_pRecvBuff[4]);
				break;
			case 5:		//���ȵ�
				open_heat(g_pRecvBuff[4]);
				break;
			default :
				break;
			}
			g_pSendBuff[1] = 0x07;
			g_pSendBuff[2] = cmd;
			g_pSendBuff[3] = 0x01;	//����
			g_pSendBuff[4] = 0x00;	//�ɹ�
		}else	 //��ѯ������
		{
			cmd = g_pRecvBuff[2];
			switch(cmd)
			{
			case 1:		//�¶�
				get_temper((uchar*)(g_pSendBuff+5));
				g_pSendBuff[1] = 9;
				break;
			case 2:		//�¶�������
				get_temper_updown((uchar*)(g_pSendBuff+5),(uchar*)(g_pSendBuff+6));//
				g_pSendBuff[1] = 9;
				break;
			case 3:		//ʱ��
				get_system_time((uchar*)(g_pSendBuff+5));
				g_pSendBuff[1] = 14;
				break;
			case 4:		//����
				get_moto_stat((uchar*)(g_pSendBuff+5));
				g_pSendBuff[1] = 8;
				break;
			case 5:		//���ȵ�
				get_heat_stat((uchar*)(g_pSendBuff+5));
				g_pSendBuff[1] = 8;
				break;
			default :
				break;
			}
			g_pSendBuff[2] = cmd;
			g_pSendBuff[3] = 0x00;	//��ȡ
			g_pSendBuff[4] = 0x00;	//�ɹ�
		}

		//Ӧ������
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