/*
 * systick.h
 *
 * Created: 11/05/2018 21:57:13
 *  Author: eduardomarossi
 */ 


#ifndef SYSTICK_H_
#define SYSTICK_H_

#include <asf.h>
unsigned long millis();
void systick_config();
volatile unsigned long g_systimer;


#endif /* SYSTICK_H_ */