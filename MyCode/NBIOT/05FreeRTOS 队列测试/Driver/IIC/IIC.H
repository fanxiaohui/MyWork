#ifndef __IIC_H
#define __IIC_H	 

#include "sys.h"
#include "stm32f10x.h"
#include "sys.h"
#include "delay.h"
#include "stdio.h"
 
typedef struct IIC_DATA
{
	float Tempter_data;
	float Humi_data;
}
IIC_DATA;


//IO方向设置
#define SDA_IN()  {GPIOB->CRH&=0XF0FFFFFF;GPIOB->CRH|=0x08000000;}//上下拉输入
#define SDA_OUT() {GPIOB->CRH&=0XF0FFFFFF;GPIOB->CRH|=0x03000000;}//通用推挽输出


//IO操作函数	 
#define IIC_SCL    PBout(13) //SCL输出
#define IIC_SDA    PBout(14) //SDA输出	 
#define READ_SDA   PBin(14)  //SDA输入 
 
 
void IIC_Init(void);
//void sensor_read(void);
float READ_Temprature (void);
float READ_Humi (void);

#endif

