#ifndef __LED_H
#define __LED_H	 
#include "sys.h"
/*	 
LED �궨��
*/ 
#define LED0 PCout(3)// PC3
#define LED1 PCout(4)// PC4
#define LED2 PBout(0)// PC4

void LED_Init(void);//��ʼ��

		 				    
#endif
