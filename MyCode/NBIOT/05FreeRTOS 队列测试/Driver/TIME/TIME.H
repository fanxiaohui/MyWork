#ifndef _TIME_H
#define	_TIME_H

#include "stm32f10x.h"
#include "usart.h"

void TIM5_Int_Init(u16 arr,u16 psc);
void TIM3_Int_Init(u16 arr,u16 psc);

#endif



