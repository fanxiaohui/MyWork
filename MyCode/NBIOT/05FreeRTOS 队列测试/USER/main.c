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

//�������ȼ�,0�����ã�������������ȼ���0����͵Ĳ����ã���ʱ�������ȼ�31������
#define START_TASK_PRIO		1
//�����ջ��С	
#define START_STK_SIZE 		128  
//������
TaskHandle_t StartTask_Handler;
//������
void start_task(void *pvParameters);


//�������ȼ�
#define LED0_TASK_PRIO		2
//�����ջ��С	
#define LED0_STK_SIZE 		50  
//������
TaskHandle_t LED0Task_Handler;
//������
void led0_task(void *pvParameters);

//�������ȼ�
#define LED1_TASK_PRIO		3
//�����ջ��С	
#define LED1_STK_SIZE 		50  
//������
TaskHandle_t LED1Task_Handler;
//������
void led1_task(void *pvParameters);

//�������ȼ�
#define LED2_TASK_PRIO		4
//�����ջ��С	
#define LED2_STK_SIZE 		50  
//������
TaskHandle_t LED2Task_Handler;
//������
void led2_task(void *pvParameters);

//�������ȼ�
#define LCD_INIT_TASK_PRIO		5
//�����ջ��С	
#define LCD_INIT_STK_SIZE 		150  
//������
TaskHandle_t LCDtask_Handler;
//������
void LCD_task(void *pvParameters);


//�������ȼ�
#define TEMPER_TASK_PRIO		6
//�����ջ��С	
#define TEMPER_STK_SIZE 		256  
//������
TaskHandle_t TEMPERTask_Handler;
//������
void temperature_task(void *pvParameters);

QueueHandle_t Tempter_IIC;

// Tempter_IIC = xQueueCreate(
//							  UBaseType_t uxQueueLength,
//							  UBaseType_t uxItemSize
//						  );


extern void KEY_Init(void); 

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//����ϵͳ�ж����ȼ�����4	 
	delay_init();	    				//��ʱ������ʼ��	  
	uart_init(115200);					//��ʼ������
	LED_Init();		  					//��ʼ��LED
	KEY_Init();               //KEY��ʼ��	
	IIC_Init();

	EXIT_INIT();              //�ⲿ�жϳ�ʼ��
	
//	TIM5_Int_Init(10000-1,7200-1);   //��ʼ����ʱ��5 ����ʱ����1S
//	TIM3_Int_Init(10000-1,7200-1);	//��ʼ����ʱ��5 ����ʱ����1S
	
	LCD_Init();
	
	LCD_Clear(0, 0, 240, 320, BACKGROUND);
	LCD_DispStr(20, 60, (uint8_t *)"Temprature:", RED);
	LCD_DispStr(20, 75, (uint8_t *)"Humi:", RED);
//	LCD_DispChar(60, 60, 'A', RED);
			
	LCD_DispStr(10, 10, (uint8_t *)"This is a lcd demo to display ascii", RED);	

		
	
	//������ʼ����,��̬��������
    xTaskCreate((TaskFunction_t )start_task,            //������
                (const char*    )"start_task",          //��������
                (uint16_t       )START_STK_SIZE,        //�����ջ��С
                (void*          )NULL,                  //���ݸ��������Ĳ���,�õĽ���
                (UBaseType_t    )START_TASK_PRIO,       //�������ȼ�
                (TaskHandle_t*  )&StartTask_Handler);   //������������Ҫ�����񴴽��ɹ��ı�־��
							//����������Ҫ������������ͨ��������              
    vTaskStartScheduler();          //�����������
}

//��ʼ����������
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           //�����ٽ���
	
	 Tempter_IIC = xQueueCreate(2,sizeof(struct IIC_DATA*));   //��������
	 if(Tempter_IIC == NULL)
	  {
		  printf("Creat Queue Failed!\r\n");
	  }

    //����LED0����
    xTaskCreate((TaskFunction_t )led0_task,     	
                (const char*    )"led0_task",   	
                (uint16_t       )LED0_STK_SIZE, 
                (void*          )NULL,				
                (UBaseType_t    )LED0_TASK_PRIO,	
                (TaskHandle_t*  )&LED0Task_Handler);
									
    //����LED1����
    xTaskCreate((TaskFunction_t )led1_task,     
                (const char*    )"led1_task",   
                (uint16_t       )LED1_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )LED1_TASK_PRIO,
                (TaskHandle_t*  )&LED1Task_Handler);
    //����LED2����
    xTaskCreate((TaskFunction_t )led2_task,     
                (const char*    )"led2_task",   
                (uint16_t       )LED2_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )LED2_TASK_PRIO,
                (TaskHandle_t*  )&LED2Task_Handler);
								
								    //����LCD����
    xTaskCreate((TaskFunction_t )LCD_task,     
                (const char*    )"LCD_task",   
                (uint16_t       )LCD_INIT_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )LCD_INIT_TASK_PRIO,
                (TaskHandle_t*  )&LCDtask_Handler);
								
																    //������ʪ������
    xTaskCreate((TaskFunction_t )temperature_task,     
                (const char*    )"temperature_task",   
                (uint16_t       )TEMPER_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )TEMPER_TASK_PRIO,
                (TaskHandle_t*  )&TEMPERTask_Handler);
								
    vTaskDelete(StartTask_Handler); //ɾ����ʼ����
    taskEXIT_CRITICAL();            //�˳��ٽ���
}

//LED0������ 
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
					printf("LED2�������!\r\n");
					vTaskSuspend(LED2Task_Handler);   //��������2
					
				}
    }
}   

//LED1������
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
					num = 0;               //num��ֵҪ���㣬����ָ�������ֻ��ִ��һ�Σ��ָ����num=21
					printf("LED1�������!\r\n");
					vTaskSuspend(NULL);   //���������Լ�
					
				}
    }
}


//LED1������
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
//			vTaskSuspend(NULL);   //���������Լ�
//			  if(i>=5)
//				{
//					i = 0;
//					printf("Delete task3!\r\n");
//					vTaskDelete(LED2Task_Handler); //ɾ���Լ�
					
//				}
    }
}

//LCD���� ������499���Զ���������
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
//								 QueueHandle_t xQueue,    //ȡ���ݵĶ���
//								 void *pvBuffer,          //����ȡ����������
//								 TickType_t xTicksToWait   //����ʱ��
//           			portTICK_PERIOD_MS         //���ȣ�ֱ���ȵ�����
			
			sprintf(a,"%3.1f",Temp->Tempter_data);
			LCD_DispStr(85, 60, (uint8_t *)a, RED);
			sprintf(b,"%3.1f",Temp->Humi_data);
			LCD_DispStr(60, 75, (uint8_t *)b, RED);
//			for( n=0; n<500; n++ )
//			{
//				LCD_DisNum(100, 100, n, RED);
//				vTaskDelay(80);
//			}
			
			
			
//			vTaskSuspend(NULL);     //�����Լ�
//			vTaskDelete(LCDtask_Handler); //ɾ���Լ�
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
			if((Tempter_IIC != NULL)&&(TEM))   //������Ч���¶�ֵ��Ч
			{
				 struct IIC_DATA *Temp;
				
					tempdata.Tempter_data = TEM;
          tempdata.Humi_data = 	HUMI;
				
					Temp = &tempdata;   //���ݵ�ַ
				
				  err = xQueueSend(Tempter_IIC,(void *)&Temp, 10);
				
				vTaskDelay(1000);

//				 err = xQueueSend(Tempter_IIC,&HUMI,10);
//							  QueueHandle_t xQueue,       //����Ҫ���͵��Ķ���
//							  const void * pvItemToQueue,  //Ҫ���͵�����
//							  TickType_t xTicksToWait      //������ʱ��
				
					printf("TEH TEMP IS = %3.1f\r\n",tempdata.Tempter_data);
					printf("TEH HUNI IS = %3.1f\r\n",tempdata.Humi_data);
				
				if(err != pdTRUE)
				{
					printf("���з���ʧ��\r\n");
				}
						
			}
//			HUMI = READ_Humi();

		
	}

}
/*
//�ر��жϺ�ֻ�б��ж���ֵ�ߵ��жϲŻ�ִ�У��������񲻻�ִ��
void interrupt_task(void *pvParameters)
{
	static u32 total_num=0;
    while(1)
    {
		total_num+=1;
		if(total_num==5) 
		{
			printf("�ر��ж�.............\r\n");
			portDISABLE_INTERRUPTS();				//�ر��ж�
			delay_xms(5000);						//��ʱ5s
			delay_xms(5000);
			printf("���ж�.............\r\n");	//���ж�
			portENABLE_INTERRUPTS();
		}
        LED0=~LED0;
        vTaskDelay(1000);  //��Ҫ���ӿ����񣬷���һ������Ҳô�оͻ�����
    }
}

*/
