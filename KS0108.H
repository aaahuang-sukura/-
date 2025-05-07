/*******************************************************
* ÎÄ¼þÃû£ºks0108.h
********************************************************/
#ifndef _KS0108_H
#define _KS0108_H

void ks0108_init(void);
void show_time(SYSTEM_TIME str_time);
void show_temperature(uchar int_temper, uchar dec_temper, uchar state);
void show_area(uchar down_temper, uchar up_temper);

#endif