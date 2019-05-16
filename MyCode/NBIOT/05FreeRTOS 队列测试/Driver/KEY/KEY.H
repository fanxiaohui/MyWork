
#ifndef __LED_H
#define __LED_H 

#include "sys.h"
#include "delay.h"

#define KEY1  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13)//读取按键0
#define KEY2  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)//读取按键1

#define KEY1_PRES 	1	//KEY0按下
#define KEY2_PRES	  2	//KEY1按下

void KEY_Init(void); //按键初始化
u8 KEY_Scan(u8);  	//按键扫描函数	


#endif

