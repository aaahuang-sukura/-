/*****************************************************************************
* 文件名：KS0108.c
* 说  明：操作KS0108系列芯片驱动液晶的基本函数
*****************************************************************************/
#include "main.h"

#define DATA P0		//LCD12864数据线

sbit RS = P2^7;		// 数据\指令 选择
sbit RW = P2^6;		// 读\写 选择
sbit EN = P2^5;		// 读\写使能
sbit CS1 = P2^3;	// 片选1
sbit CS2 = P2^4;	// 片选2

/*************************************************************
*	定义中文字库 
*   宋体、小四（12），点阵为：宽x高=16x16，纵向取模、字节倒序
**************************************************************/		
uchar code PIC1616[]={
//℃
0x00,0x02,0x07,0xE7,0xFA,0x1C,0x06,0x02,0x02,0x02,0x02,0x02,0x06,0x1E,0x1E,0x00,
0x00,0x00,0x00,0x07,0x1F,0x38,0x60,0x40,0x40,0x40,0x40,0x40,0x60,0x38,0x18,0x00,

//当
0x00,0x00,0x40,0x42,0x5E,0x5C,0x48,0x40,0x7F,0x7F,0x50,0x5E,0x4E,0xC4,0xC0,0x00,
0x00,0x00,0x20,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x7F,0x7F,0x00, 
//前
0x08,0x08,0xE8,0xE8,0xA9,0xAF,0xEE,0xEA,0x08,0xC8,0xCC,0x0F,0xEB,0xEA,0x08,0x08,
0x00,0x00,0x7F,0x7F,0x24,0x64,0x7F,0x3F,0x00,0x1F,0x5F,0xC0,0xFF,0x7F,0x00,0x00,

//正
0x00,0x02,0x02,0xC2,0xC2,0x02,0x02,0x02,0xFE,0xFE,0x82,0x82,0x82,0x82,0x82,0x02,
0x20,0x20,0x20,0x3F,0x3F,0x20,0x20,0x20,0x3F,0x3F,0x20,0x20,0x20,0x20,0x20,0x20,
//常
0x20,0x38,0x18,0x09,0xEF,0xEE,0xAA,0xAF,0xAF,0xA8,0xEC,0xEF,0x2B,0x3A,0x18,0x08,
0x00,0x00,0x3E,0x3E,0x02,0x02,0x02,0xFF,0xFF,0x02,0x12,0x32,0x3E,0x1E,0x00,0x00,

//超
0x40,0x48,0x48,0x48,0xFF,0xFF,0x48,0xCA,0xC2,0xFE,0xBE,0xA2,0xE2,0xFE,0xBE,0x00,
0x60,0x7F,0x3F,0x60,0x7F,0x7F,0x42,0x42,0x5F,0x5F,0x48,0x48,0x48,0x5F,0x5F,0x40, 
//温
0x10,0x31,0xA7,0xF6,0x70,0x7E,0x7E,0x4A,0x4A,0x4A,0x4A,0x7E,0x7E,0x00,0x00,0x00,
0x02,0xFE,0xFF,0x41,0x7F,0x7F,0x41,0x7F,0x7F,0x41,0x7F,0x7F,0x41,0x7F,0x7F,0x40,
};

/*************************************************************
*	定义数字、符号点阵 
*   宋体、小四（12），点阵为：宽x高=8x16；
*   纵向取模、字节倒序（逆向、列行式）
**************************************************************/
uchar code PIC0816[]={
0x00,0xE0,0xF0,0x18,0x08,0x18,0xF0,0xE0,0x00,0x0F,0x1F,0x30,0x20,0x30,0x1F,0x0F,//0
0x00,0x10,0x10,0xF8,0xF8,0x00,0x00,0x00,0x00,0x20,0x20,0x3F,0x3F,0x20,0x20,0x00,//1
0x00,0x70,0x78,0x08,0x08,0x88,0xF8,0x70,0x00,0x30,0x38,0x2C,0x26,0x23,0x31,0x30,//2
0x00,0x30,0x38,0x88,0x88,0xC8,0x78,0x30,0x00,0x18,0x38,0x20,0x20,0x31,0x1F,0x0E,//3	
0x00,0x00,0xC0,0xE0,0x30,0xF8,0xF8,0x00,0x00,0x07,0x07,0x24,0x24,0x3F,0x3F,0x24,//4
0x00,0xF8,0xF8,0x88,0x88,0x88,0x08,0x08,0x00,0x19,0x39,0x21,0x20,0x31,0x1F,0x0E,//5
0x00,0xE0,0xF0,0x98,0x88,0x98,0x18,0x00,0x00,0x0F,0x1F,0x31,0x20,0x31,0x1F,0x0E,//6
0x00,0x38,0x38,0x08,0xC8,0xF8,0x38,0x08,0x00,0x00,0x00,0x3F,0x3F,0x00,0x00,0x00,//7	
0x00,0x70,0xF8,0x88,0x08,0x88,0xF8,0x70,0x00,0x1C,0x3E,0x23,0x21,0x23,0x3E,0x1C,//8	
0x00,0xE0,0xF0,0x18,0x08,0x18,0xF0,0xE0,0x00,0x00,0x31,0x33,0x22,0x33,0x1F,0x0F,//9	

//up
0x40,0x70,0x7C,0xFF,0xFF,0x7C,0x70,0x40,0x00,0x00,0x00,0x7F,0x7F,0x00,0x00,0x00,//10
//down
0x00,0x00,0x00,0xFE,0xFE,0x00,0x00,0x00,0x02,0x0E,0x3E,0xFF,0xFF,0x3E,0x0E,0x02,//11

//-
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,//12
//+
0x00,0x00,0x00,0xF0,0xF0,0x00,0x00,0x00,0x01,0x01,0x01,0x1F,0x1F,0x01,0x01,0x01,//13

//:
0x00,0x00,0x00,0xC0,0xC0,0xC0,0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x30,0x00,0x00,//14 
//.
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x30,0x00,0x00,0x00,0x00,//15
};

/******************************************************
* 函  数：检查LCD状态，是否忙
* 参  数：空
* 返回值：0空闲，非0忙
******************************************************/
uchar check_state(void)		
{
	uchar i = 0;
	uchar tmp = 0;		//状态信息（判断是否忙）

	RS = 0;	//数据\指令选择，D/I（RS）=“L” ，表示 DB7∽DB0 为显示数据 
	RW = 1; //R/W=“H” ，E=“H”数据被读到DB7∽DB0 
	for(i=0; i<6; i++)
	{
		DATA = 0x00;
		EN = 1;		//EN下降源
		_nop_();	//一个时钟延时
		tmp = DATA;
		EN = 0;
		tmp = 0x80 & tmp;	//仅当第7位为0时才可操作(判别busy信号)
		if(tmp == 0)
		{
			break;
		}
	}
	return(tmp);
}

/******************************************************
* 函  数：写命令到LCD
* 参  数：cmd要写的命令
* 返回值：无
******************************************************/
void write_cmd(uchar cmd)
{
	uchar stat = 0;
	stat = check_state();	//状态检查，LCD是否忙
	RS = 0;		//向LCD发送命令。RS=0写指令，RS=1写数据
	RW = 0;		//R/W=“L” ，E=“H→L”数据被写到 IR 或 DR 
	DATA = cmd;	//com :命令
	EN = 1;		//EN下降源
	_nop_();_nop_(); 
	EN = 0;
}

/******************************************************
* 函  数：写数据到LCD
* 参  数：dat要写的数据
* 返回值：无
******************************************************/
void write_data(uchar dat)	
{
	uchar stat = 0;
	stat = check_state();	//状态检查，LCD是否忙
	EN = 0;		//先将EN管脚拉低
	_nop_();_nop_();
	RS = 1;		//RS=0写指令，RS=1写数据
	RW = 0;		//R/W="L" ，E="H→L"数据被写到 IR 或 DR 
	EN = 1;		//拉高EN管脚
	DATA = dat;	//dat:显示数据
	_nop_();_nop_();
	EN = 0;		//数据在EN的下降沿被写入

}  

/******************************************************
* 函  数：设置页地址：0xB8
* 参  数：page页地址
* 返回值：无
******************************************************/
void set_page(uchar page)	
{
	page = 0xB8|page; //1011 1xxx 0<=page<=7 设定页地址--X 0-7,8行为一页64/8=8，共8页
	write_cmd(page);
}

/******************************************************
* 函  数：设定行地址--(X:0-63)：0xC0
* 参  数：line行地址
* 返回值：无
******************************************************/
void set_line(uchar line) 	   
{
	line = 0xC0|line;	//1100 0000
	write_cmd(line);		//设置从哪行开始：0--63，一般从0 行开始显示
}

/******************************************************
* 函  数：设定列地址--(Y:0-63)：0x40
* 参  数：column列地址
* 返回值：无
******************************************************/
void set_column(uchar column)	
{
	column = column&0x3f;	//column最大值为64，越出 0=<column<=63
	column = 0x40|column;	//01xx xxxx
	write_cmd(column);
}

/******************************************************
* 函  数：开关显示：0x3E
* 参  数：onoff开关，0x3E关显示，0x3F开显示
* 返回值：无
******************************************************/
void set_on(uchar onoff)	   
{
	onoff = 0x3E|onoff; //0011 111x,onoff只能为0或者1
	write_cmd(onoff);
}

/******************************************************
* 函  数：选择屏幕
* 参  数：screen: 0-全屏,1-左屏,2-右屏
* 返回值：无
******************************************************/
void select_screen(uchar screen)	  
{ 						 
	switch(screen)
	{ 
	case 0: 
		CS1=0;		//全屏
		_nop_(); _nop_(); _nop_(); 
		CS2=0; 
		_nop_(); _nop_(); _nop_(); 
		break; 
	case 1: 
		CS1=1;		//左屏
		_nop_(); _nop_(); _nop_(); 
		CS2=0;
		_nop_(); _nop_(); _nop_(); 
		break;
	case 2: 
		CS1=0;		//右屏
		_nop_(); _nop_(); _nop_(); 
		CS2=1;
		_nop_(); _nop_(); _nop_(); 
		break;
	}
}

/******************************************************
* 函  数：清屏
* 参  数：screen: 0-全屏,1-左屏,2-右屏
* 返回值：无
******************************************************/
void clear_screen(uchar screen)	  
{ 	
	uchar i,j;

	select_screen(screen); 
	for(i=0; i<8; i++)		//控制页数0-7，共8页
	{
		set_page(i);
		set_column(0);
		for(j=0; j<64; j++)	//控制列数0-63，共64列
		{
			write_data(0x00);	//写点内容，列地址自动加1
		}
	}				 
}

/******************************************************
* 函  数：初始化LCD
* 参  数：空
* 返回值：无
******************************************************/
void ks0108_init(void) 	  
{ 

	check_state();

	select_screen(0);
	set_on(0);		//关显示

	select_screen(0);
	set_on(1);		//开显示

	select_screen(0);
	clear_screen(0);	//清屏

	set_line(0);		//开始行:0	
}

/******************************************************
* 函  数：显示全角汉字（16x16图片）
* 参  数：cs选屏参数，page选页参数，
*         column选列参数，number图片序号（第几个汉字）
* 返回值：无
******************************************************/
void show_1616(uchar cs, uchar page, uchar column, uchar number)
{
	int i;
		
	select_screen(cs);
	column = column&0x3f;

	set_page(page);			//写上半页
	set_column(column);		//控制列
	for(i=0; i<16; i++)		//控制16列的数据输出
	{
		write_data(PIC1616[i+32*number]); //i+32*number汉字的前16个数据输出	 	
	}

	set_page(page+1);		//写下半页
	set_column(column);		//控制列
    for(i=0;i<16;i++)		//控制16列的数据输出
	{
		write_data(PIC1616[i+32*number+16]);	//i+32*number+16汉字的后16个数据输出
	}

} 

/******************************************************
* 函  数：显示半角汉字、数字、字母（08x16图片）
* 参  数：cs选屏参数，page选页参数，
*         column选列参数，number图片序号（第几个数字）
* 返回值：无
******************************************************/
void show_0816(uchar cs,uchar page, uchar column, uchar number)
{
	uint i;
		
	select_screen(cs);
	column = column&0x3f;

	set_page(page);		//写上半页
	set_column(column);
   	for(i=0;i<8;i++)
	{
		write_data(PIC0816[i+16*number]);
	}
	
	set_page(page+1);	//写下半页
	set_column(column);
   	for(i=0;i<8;i++)
	{
		write_data(PIC0816[i+16*number+8]);
	}
}  

/******************************************************
* 函  数：显示当前时间
* 参  数：str_time当前时间
* 返回值：无
******************************************************/
void show_time(SYSTEM_TIME str_time)
{
	uchar tmp = 0;	
	show_0816(2, 0, 24, 2); //年	
	show_0816(2, 0, 32, 0);
	tmp = str_time.year/10;
	show_0816(2, 0, 40, tmp);
	tmp = str_time.year%10;
	show_0816(2, 0, 48, tmp);
	show_0816(2, 0, 56, 12);//-

	tmp = str_time.month/10;
	show_0816(1, 0, 0, tmp);//月
	tmp = str_time.month%10;
	show_0816(1, 0, 8, tmp);
	show_0816(1, 0, 16, 12);//-

	tmp = str_time.date/10;
	show_0816(1, 0, 24, tmp);//日
	tmp = str_time.date%10;
	show_0816(1, 0, 32, tmp);
	
	tmp = str_time.hour/10;
	show_0816(2, 2, 32, tmp);//时
	tmp = str_time.hour%10;
	show_0816(2, 2, 40, tmp);
	show_0816(2, 2, 48, 14);//:

	tmp = str_time.min/10;
	show_0816(2, 2, 56, tmp);//分
	tmp = str_time.min%10;
	show_0816(1, 2, 0, tmp);
	show_0816(1, 2, 8, 14);//:

	tmp = str_time.sec/10;
	show_0816(1, 2, 16, tmp);//秒
	tmp = str_time.sec%10;
	show_0816(1, 2, 24, tmp);
}

/******************************************************
* 函  数：显示当前温度（含符号）
* 参  数：up上限，down下限,state状态0正常，非0超温
* 返回值：无
******************************************************/
void show_temperature(uchar int_temper, uchar dec_temper, uchar state)
{
	uchar tmp = 0;
	uchar symbol = 0;
	uchar int_tmp = 0;
	uint dec_tmp = 0;

	show_1616(2,4,0*16, 1);//当
	show_1616(2,4,1*16, 2);//前

	int_tmp = int_temper;
	if((int_tmp &0x80) != 0)
	{
		symbol = 1;				//符号为负
		int_tmp = ~int_tmp;
		dec_temper = (0x0F-dec_temper) +1;	//负数取反加1
		if(dec_temper == 0x10)			//小数部分进位，则整数进位
		{
			int_tmp += 1;
			dec_temper = 0;
		}
	}

	show_0816(2, 4, 32, 13-symbol);	//+/-
	tmp = int_tmp/10;
	show_0816(2, 4, 40, tmp);
	tmp = int_tmp%10;
	show_0816(2, 4, 48, tmp);
	show_0816(2, 4, 56, 15);  //.

	dec_tmp = dec_temper;
	dec_tmp = dec_tmp * 625 + 500;	//对百分位进行四舍五入
	dec_temper = dec_tmp/1000;		//只显示十分位

	show_0816(1, 4, 0, dec_temper);
	show_1616(1, 4, 8, 0);
	
	//是否正常
	if(state == 0)
	{
		show_1616(1, 4, 32, 3);//正常
		show_1616(1, 4, 48, 4);	
	}else
	{	
		show_1616(1, 4, 32, 5);//超温
		show_1616(1, 4, 48, 6);//
	}
}

/******************************************************
* 函  数：显示温度上、下限（含符号）
* 参  数：up上限，down下限
* 返回值：无
******************************************************/
void show_area(uchar down, uchar up)
{
	uchar tmp = 0; 
	uchar symbol = 0;	
	if((down&0x80) != 0)
	{
		down = 0xFF - down + 1;		//取反，加1
		symbol = 1;
	}
	show_0816(2, 6, 8+0, 11);		//下限显示
	show_0816(2, 6, 8+8, 13-symbol);	//+/-
	tmp = down/10;
	show_0816(2, 6, 8+16, tmp);
	tmp = down%10;
	show_0816(2, 6, 8+24, tmp);
	show_1616(2, 6, 8+32,0);			//℃	
	
	symbol = 0;	
	if((up&0x80) != 0)
	{
		up = 0xFF - up + 1;		//取反，加1
		symbol = 1;
	}
	show_0816(1, 6, 0, 10);		//上限显示
	show_0816(1, 6, 8, 13-symbol);		//+/-
	tmp = up/10;
	show_0816(1, 6, 16, tmp);
	tmp = up%10;
	show_0816(1, 6, 24, tmp);
	show_1616(1, 6, 32,0);		//℃   	 
}
