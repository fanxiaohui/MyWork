#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "FreeRTOS.h"
#include "task.h"
#include "key.h"
#include "exit.h"
#include "lcd.h"
#include "time.h"
#include "iic.h"
#include "queue.h"
#include "string.h"


/************************************************

************************************************/

//任务优先级,0不能用，调度任务的优先级是0，最低的不能用，软定时器的优先级31不能用
#define START_TASK_PRIO		1
//任务堆栈大小	
#define START_STK_SIZE 		128  
//任务句柄
TaskHandle_t StartTask_Handler;
//任务函数
void start_task(void *pvParameters);


//任务优先级
#define LED0_TASK_PRIO		2
//任务堆栈大小	
#define LED0_STK_SIZE 		50  
//任务句柄
TaskHandle_t LED0Task_Handler;
//任务函数
void led0_task(void *pvParameters);

//任务优先级
#define LED1_TASK_PRIO		3
//任务堆栈大小	
#define LED1_STK_SIZE 		50  
//任务句柄
TaskHandle_t LED1Task_Handler;
//任务函数
void led1_task(void *pvParameters);

//任务优先级
#define LED2_TASK_PRIO		4
//任务堆栈大小	
#define LED2_STK_SIZE 		50  
//任务句柄
TaskHandle_t LED2Task_Handler;
//任务函数
void led2_task(void *pvParameters);

//任务优先级
#define LCD_INIT_TASK_PRIO		5
//任务堆栈大小	
#define LCD_INIT_STK_SIZE 		150  
//任务句柄
TaskHandle_t LCDtask_Handler;
//任务函数
void LCD_task(void *pvParameters);


//任务优先级
#define TEMPER_TASK_PRIO		6
//任务堆栈大小	
#define TEMPER_STK_SIZE 		256  
//任务句柄
TaskHandle_t TEMPERTask_Handler;
//任务函数
void temperature_task(void *pvParameters);

QueueHandle_t Tempter_IIC;

// Tempter_IIC = xQueueCreate(
//							  UBaseType_t uxQueueLength,
//							  UBaseType_t uxItemSize
//						  );


extern void KEY_Init(void); 

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//设置系统中断优先级分组4	 
	delay_init();	    				//延时函数初始化	  
	uart_init(115200);					//初始化串口
	LED_Init();		  					//初始化LED
	KEY_Init();               //KEY初始化	
	IIC_Init();

	EXIT_INIT();              //外部中断初始化
	
//	TIM5_Int_Init(10000-1,7200-1);   //初始化定时器5 ，定时周期1S
//	TIM3_Int_Init(10000-1,7200-1);	//初始化定时器5 ，定时周期1S
	
	LCD_Init();
	
	LCD_Clear(0, 0, 240, 320, BACKGROUND);
	LCD_DispStr(20, 60, (uint8_t *)"Temprature:", RED);
	LCD_DispStr(20, 75, (uint8_t *)"Humi:", RED);
//	LCD_DispChar(60, 60, 'A', RED);
			
	LCD_DispStr(10, 10, (uint8_t *)"This is a lcd demo to display ascii", RED);	

		
	
	//创建开始任务,动态创建任务
    xTaskCreate((TaskFunction_t )start_task,            //任务函数
                (const char*    )"start_task",          //任务名称
                (uint16_t       )START_STK_SIZE,        //任务堆栈大小
                (void*          )NULL,                  //传递给任务函数的参数,用的较少
                (UBaseType_t    )START_TASK_PRIO,       //任务优先级
                (TaskHandle_t*  )&StartTask_Handler);   //任务句柄，很重要，任务创建成功的标志，
							//其他的任务要操作这个任务就通过任务句柄              
    vTaskStartScheduler();          //开启任务调度
}

//开始任务任务函数
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           //进入临界区
	
	 Tempter_IIC = xQueueCreate(2,sizeof(struct IIC_DATA*));   //创建队列
	 if(Tempter_IIC == NULL)
	  {
		  printf("Creat Queue Failed!\r\n");
	  }

    //创建LED0任务
    xTaskCreate((TaskFunction_t )led0_task,     	
                (const char*    )"led0_task",   	
                (uint16_t       )LED0_STK_SIZE, 
                (void*          )NULL,				
                (UBaseType_t    )LED0_TASK_PRIO,	
                (TaskHandle_t*  )&LED0Task_Handler);
									
    //创建LED1任务
    xTaskCreate((TaskFunction_t )led1_task,     
                (const char*    )"led1_task",   
                (uint16_t       )LED1_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )LED1_TASK_PRIO,
                (TaskHandle_t*  )&LED1Task_Handler);
    //创建LED2任务
    xTaskCreate((TaskFunction_t )led2_task,     
                (const char*    )"led2_task",   
                (uint16_t       )LED2_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )LED2_TASK_PRIO,
                (TaskHandle_t*  )&LED2Task_Handler);
								
								    //创建LCD任务
    xTaskCreate((TaskFunction_t )LCD_task,     
                (const char*    )"LCD_task",   
                (uint16_t       )LCD_INIT_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )LCD_INIT_TASK_PRIO,
                (TaskHandle_t*  )&LCDtask_Handler);
								
																    //创建温湿度任务
    xTaskCreate((TaskFunction_t )temperature_task,     
                (const char*    )"temperature_task",   
                (uint16_t       )TEMPER_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )TEMPER_TASK_PRIO,
                (TaskHandle_t*  )&TEMPERTask_Handler);
								
    vTaskDelete(StartTask_Handler); //删除开始任务
    taskEXIT_CRITICAL();            //退出临界区
}

//LED0任务函数 
void led0_task(void *pvParameters)
{
    int i = 0;
	  while(1)
    {
				LED0=~LED0;
        vTaskDelay(20);
				printf("this is led0 task!\r\n");
				i++;
				if(i>=200)
				{
					i = 0;
					printf("LED2任务挂起!\r\n");
					vTaskSuspend(LED2Task_Handler);   //挂起任务2
					
				}
    }
}   

//LED1任务函数
void led1_task(void *pvParameters)
{
		int num = 0;
	   while(1)
      {
        LED1=0;
        vTaskDelay(10);
        LED1=1;
        vTaskDelay(30);
				printf("this is led1 task!\r\n");
				num++;
				if(num>=60)
				{	
					num = 0;               //num的值要清零，否则恢复后任务只会执行一次，恢复后的num=21
					printf("LED1任务挂起!\r\n");
					vTaskSuspend(NULL);   //挂起任务自己
					
				}
    }
}


//LED1任务函数
void led2_task(void *pvParameters)
{
	  while(1)
    {
        LED2=0;
        vTaskDelay(20);
        LED2=1;
        vTaskDelay(40);
				printf("this is led2 task!\r\n");
//				sensor_read();
//			vTaskSuspend(NULL);   //挂起任务自己
//			  if(i>=5)
//				{
//					i = 0;
//					printf("Delete task3!\r\n");
//					vTaskDelete(LED2Task_Handler); //删除自己
					
//				}
    }
}

//LCD任务 计数到499后自动挂起任务
void LCD_task(void *pvParameters)
{
//	int n = 0;
//	float Tempter_data=10.1;
//	float Humi_data=10.1;
	char a[8];
	char b[8];
	BaseType_t err;	
	while(1)
		{				
//	    LCD_DispStr(40, 100, (uint8_t *)"count:", RED);
			struct IIC_DATA *Temp;
			
			if(Tempter_IIC != NULL)
			{
			  err = xQueueReceive(Tempter_IIC,&Temp,portTICK_PERIOD_MS);
				vTaskDelay(100);
//				err = xQueueReceive(Tempter_IIC,&Humi_data,portTICK_PERIOD_MS);
				if(err == pdTRUE)
				{
					printf("Get The Massage Successful!\r\n");
					printf("The Tempter_Value Is =%3.1f!\r\n",Temp->Tempter_data);
					printf("The Humi_Value Is =%3.1f!\r\n",Temp->Humi_data);
				}
			}
			else
			{
				vTaskDelay(80);
				printf("Get The Massage Failed!\r\n");
			}
//								 QueueHandle_t xQueue,    //取数据的队列
//								 void *pvBuffer,          //数据取来放在哪里
//								 TickType_t xTicksToWait   //阻塞时间
//           			portTICK_PERIOD_MS         //死等，直到等到数据
			
			sprintf(a,"%3.1f",Temp->Tempter_data);
			LCD_DispStr(85, 60, (uint8_t *)a, RED);
			sprintf(b,"%3.1f",Temp->Humi_data);
			LCD_DispStr(60, 75, (uint8_t *)b, RED);
//			for( n=0; n<500; n++ )
//			{
//				LCD_DisNum(100, 100, n, RED);
//				vTaskDelay(80);
//			}
			
			
			
//			vTaskSuspend(NULL);     //挂起自己
//			vTaskDelete(LCDtask_Handler); //删除自己
		}

}

void temperature_task(void *pvParameters)

{
	IIC_DATA tempdata;
	float TEM,HUMI;
	BaseType_t err;
	while(1)
	{
		
//		vTaskDelay(1000);
			TEM = READ_Temprature();
			HUMI = READ_Humi();
			if((Tempter_IIC != NULL)&&(TEM))   //队列有效，温度值有效
			{
				 struct IIC_DATA *Temp;
				
					tempdata.Tempter_data = TEM;
          tempdata.Humi_data = 	HUMI;
				
					Temp = &tempdata;   //传递地址
				
				  err = xQueueSend(Tempter_IIC,(void *)&Temp, 10);
				
				vTaskDelay(1000);

//				 err = xQueueSend(Tempter_IIC,&HUMI,10);
//							  QueueHandle_t xQueue,       //数据要发送到的队列
//							  const void * pvItemToQueue,  //要发送的数据
//							  TickType_t xTicksToWait      //阻塞的时间
				
					printf("TEH TEMP IS = %3.1f\r\n",tempdata.Tempter_data);
					printf("TEH HUNI IS = %3.1f\r\n",tempdata.Humi_data);
				
				if(err != pdTRUE)
				{
					printf("队列发送失败\r\n");
				}
						
			}
//			HUMI = READ_Humi();

		
	}

}
/*
//关闭中断后只有比中断阈值高的中断才会执行，但是任务不会执行
void interrupt_task(void *pvParameters)
{
	static u32 total_num=0;
    while(1)
    {
		total_num+=1;
		if(total_num==5) 
		{
			printf("关闭中断.............\r\n");
			portDISABLE_INTERRUPTS();				//关闭中断
			delay_xms(5000);						//延时5s
			delay_xms(5000);
			printf("打开中断.............\r\n");	//打开中断
			portENABLE_INTERRUPTS();
		}
        LED0=~LED0;
        vTaskDelay(1000);  //需要增加空任务，否则一个任务也么有就会死掉
    }
}

*/
