/*
 * systick.c
 *
 * Created: 11/05/2018 21:56:57
 *  Author: eduardomarossi
 */ 

#include "systick.h"

volatile unsigned long g_systimer = 0;

void SysTick_Handler() {
	g_systimer++;
}

void systick_config() {
	SysTick_Config(sysclk_get_cpu_hz() / 1000); // 1 ms
}

unsigned long millis() {
	return g_systimer;
}