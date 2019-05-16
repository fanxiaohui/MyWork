
#include "exit.h"
#include "key.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"

//外部中断0&13服务程序
void EXIT_INIT(void)
{
 	  EXTI_InitTypeDef EXTI_InitStructure;
 	  NVIC_InitTypeDef NVIC_InitStructure;

    KEY_Init();	 //	按键端口初始化

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//使能复用功能时钟

    //GPIOA.0	  中断线以及中断初始化配置  下降沿触发	//KEY0
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource0);

	  EXTI_InitStructure.EXTI_Line=EXTI_Line0;
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);	 	//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器

	  NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;				//使能按键KEY0所在的外部中断通道
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x06;	//抢占优先级6
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;			//子优先级0 
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;					//使能外部中断通道
  	NVIC_Init(&NVIC_InitStructure);  	  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
		
		    //GPIOC.13	  中断线以及中断初始化配置  下降沿触发	//KEY1
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource13);

	  EXTI_InitStructure.EXTI_Line=EXTI_Line13;
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);	 	//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器

	  NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;				//使能按键KEY0所在的外部中断通道
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x07;	//抢占优先级7
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;			//子优先级0 
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;					//使能外部中断通道
  	NVIC_Init(&NVIC_InitStructure);  	  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
 
}

//任务句柄
extern TaskHandle_t LED2Task_Handler;



//外部中断0服务程序
void EXTI0_IRQHandler(void)
{
	BaseType_t YieldRequired;
	
	delay_xms(20);	//消抖
	if(KEY2==0)	 
	{				 
		YieldRequired=xTaskResumeFromISR(LED2Task_Handler);//恢复led1任务
		printf("恢复任务LED1的运行!\r\n");
		if(YieldRequired==pdTRUE)
		{
			/*如果函数xTaskResumeFromISR()返回值为pdTRUE，那么说明要恢复的这个
			任务的任务优先级等于或者高于正在运行的任务(被中断打断的任务),所以在
			退出中断的时候一定要进行上下文切换！*/
			portYIELD_FROM_ISR(YieldRequired);
		}
	}		 
	 EXTI_ClearITPendingBit(EXTI_Line0);//清除LINE4上的中断标志位  
}


//任务句柄
extern TaskHandle_t LED1Task_Handler;

//外部中断13服务程序
void EXTI15_10_IRQHandler(void)
{
	BaseType_t YieldRequired;
	
	delay_xms(20);	//消抖
	if(KEY2==0)	 
	{				 
		YieldRequired=xTaskResumeFromISR(LED1Task_Handler);//恢复led1任务
		printf("恢复任务LED0的运行!\r\n");
		if(YieldRequired==pdTRUE)
		{
			/*如果函数xTaskResumeFromISR()返回值为pdTRUE，那么说明要恢复的这个
			任务的任务优先级等于或者高于正在运行的任务(被中断打断的任务),所以在
			退出中断的时候一定要进行上下文切换！*/
			portYIELD_FROM_ISR(YieldRequired);
		}
	}		 
	 EXTI_ClearITPendingBit(EXTI_Line13);//清除LINE13上的中断标志位  
}


