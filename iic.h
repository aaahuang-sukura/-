/*******************************************************
* ÎÄ¼þÃû£ºiic.h
********************************************************/

#ifndef _IIC_H
#define _IIC_H

#include "main.h"

void init_IIC(void);
void write_IIC(uchar add,uchar dat);
uchar read_IIC(uchar add);

#endif