
#include "iic.h"

//float Temprature,Humi;//定义温湿度变量 ，此变量为全局变量

 //初始化IIC
void IIC_Init(void)
{					     
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );//使能GPIOB	
	   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_14;//PB13,PB14
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD ;   //开漏输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_13|GPIO_Pin_14); 	//PB13,PB14 输出高
}

//产生IIC起始信号
void IIC_Start(void)
{
	SDA_OUT();     //sda线输出
	IIC_SDA=1;	  	  
	IIC_SCL=1;
	delay_us(14);
 	IIC_SDA=0;//开始：当SCL为高时，数据从高到低变化
	delay_us(14);
	IIC_SCL=0;//钳住I2C总线，准备发送或接收数据 
}	  
//产生IIC停止信号
void IIC_Stop(void)
{
	SDA_OUT();//sda线输出
	IIC_SCL=0;
	IIC_SDA=0;//停止:当SCL为高时，数据从低到高变化
 	delay_us(14);
	IIC_SCL=1;
	delay_us(14);	 
	IIC_SDA=1;//发送I2C总线结束信号
							   	
}
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
	IIC_SCL=0;
	IIC_SDA=1;
	SDA_IN();      //SDA设置为输入  
	delay_us(14);	   
	IIC_SCL=1;
	delay_us(14);	 
	while(READ_SDA)//等到SDA变低
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_Stop();
			return 1;
		}
		delay_us(1);
	}
	IIC_SCL=0;//时钟输出0
	delay_us(10); 
	return 0;  
} 
//产生ACK应答
void IIC_Ack(void)
{
	IIC_SCL=0;
	SDA_OUT();
	IIC_SDA=0;
	delay_us(15);
	IIC_SCL=1;
	delay_us(15);
	IIC_SCL=0;
}
//产生非ACK应答		    
void IIC_NAck(void)
{
	IIC_SCL=0;
	SDA_OUT();
	IIC_SDA=1;
	delay_us(15);
	IIC_SCL=1;
	delay_us(15);
	IIC_SCL=0;
}					 				     
//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
void IIC_Send_Byte(u8 txd)
{                        
    u8 t;   
	SDA_OUT(); 	    
    IIC_SCL=0;//拉低时钟开始数据传输
    for(t=0;t<8;t++)
    {              
		IIC_SCL=0;
		if((txd&0x80)>>7)
			IIC_SDA=1;
		else
			IIC_SDA=0;
		txd<<=1; 	  
		delay_us(15);   //延时
		IIC_SCL=1;
		delay_us(15); 	
    }
	IIC_SCL=0;	 
} 	    
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
u8 IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_IN();//SDA设置为输入
    for(i=0;i<8;i++ )
	{
        IIC_SCL=0; 
        delay_us(50);
		IIC_SCL=1;
		delay_us(50);
        receive<<=1;
        if(READ_SDA)receive++;   
    }
        IIC_SCL=0;						 
    if (!ack)
        IIC_NAck();//发送nACK
    else
        IIC_Ack(); //发送ACK   
    return receive;
}


/**********************************************/
/*
功能：读取温度的值
返回：温度值
*/

float READ_Temprature (void)
{
	float Temprature = 0;
	u16 i;
	u8 Humi_H,Humi_L,Temp_H,Temp_L,Temp_CAL,temp;

	IIC_Start();    //主机发送起始信号
	IIC_Send_Byte(0Xb8);    //发送IIC地址
	if(!IIC_Wait_Ack())  //等待从机应答信号（如无应答：考虑IIC通讯频率是否太快，或者传感器接线错误）
	{
		i=0;
		IIC_Send_Byte(0);
		while(IIC_Wait_Ack())//等待从机应答信号
		{
		 	if(++i>=500)
			{
			 	break;
			}		 
		} 
		i=0;
		IIC_Start();       //主机发送起始信号
		IIC_Send_Byte(0Xb9);     //发送读指令
		while(IIC_Wait_Ack())    //等待从机应答信号
		{
		 	if(++i>=500)
			{
			 	break;
			}			 
		}
	
		Humi_H=IIC_Read_Byte(1);   //读取湿度高位
		Humi_L=IIC_Read_Byte(1);   //读取湿度低位
		Temp_H=IIC_Read_Byte(1);   //读取温度高位
		Temp_L=IIC_Read_Byte(1);   //读取温度低位
		Temp_CAL=IIC_Read_Byte(0); //读取校验位
		IIC_Stop();	   //发送停止信号	
		temp = (u8)(Humi_H+Humi_L+Temp_H+Temp_L);//只取低8位
		if(Temp_CAL==temp)//如果校验成功，往下运行
		{
//			Humi=Humi_H*10+Humi_L; //湿度
	
			if(Temp_L&0X80)	//为负温度
			{
				Temprature =0-(Temp_H*10+((Temp_L&0x7F)));
			}
			else   //为正温度
			{
				Temprature=Temp_H*10+Temp_L;//为正温度
			} 	
			//判断温湿度是否超过测量范围（温度：-20℃~60摄氏度；湿度：20%RH~95%RH）		
//			if(Humi>950) 
//			{
//			  Humi=950;
//			}
//			if(Humi<200)
//			{
//				Humi =200;
//			}
			if(Temprature>600)
			{
			  Temprature=600;
			}
			if(Temprature<-200)
			{
				Temprature = -200;
			}
			Temprature=Temprature/10;//计算为温度值
//			Humi=Humi/10; //计算为湿度值
			printf("\r\n温度为:  %.1f  ℃\r\n",Temprature); //显示温度
//			printf("湿度为:  %.1f  %%RH\r\n",Humi);//显示湿度	
     return Temprature;			
		}
		else //校验失败
		{
			printf("CRC Error !!\r\n");
			return 0;
		}
	}
	else
	{
	   printf("Sensor Error !!\r\n");
		return 0;
	}	
}


float READ_Humi (void)
{
	float Humi = 0;
	u16 i;
	u8 Humi_H,Humi_L,Temp_H,Temp_L,Temp_CAL,temp;

	IIC_Start();    //主机发送起始信号
	IIC_Send_Byte(0Xb8);    //发送IIC地址
	if(!IIC_Wait_Ack())  //等待从机应答信号（如无应答：考虑IIC通讯频率是否太快，或者传感器接线错误）
	{
		i=0;
		IIC_Send_Byte(0);
		while(IIC_Wait_Ack())//等待从机应答信号
		{
		 	if(++i>=500)
			{
			 	break;
			}		 
		} 
		i=0;
		IIC_Start();       //主机发送起始信号
		IIC_Send_Byte(0Xb9);     //发送读指令
		while(IIC_Wait_Ack())    //等待从机应答信号
		{
		 	if(++i>=500)
			{
			 	break;
			}			 
		}
	
		Humi_H=IIC_Read_Byte(1);   //读取湿度高位
		Humi_L=IIC_Read_Byte(1);   //读取湿度低位
		Temp_H=IIC_Read_Byte(1);   //读取温度高位
		Temp_L=IIC_Read_Byte(1);   //读取温度低位
		Temp_CAL=IIC_Read_Byte(0); //读取校验位
		IIC_Stop();	   //发送停止信号	
		temp = (u8)(Humi_H+Humi_L+Temp_H+Temp_L);//只取低8位
		if(Temp_CAL==temp)//如果校验成功，往下运行
		{
			Humi=Humi_H*10+Humi_L; //湿度
	
//			if(Temp_L&0X80)	//为负温度
//			{
//				Temprature =0-(Temp_H*10+((Temp_L&0x7F)));
//			}
//			else   //为正温度
//			{
//				Temprature=Temp_H*10+Temp_L;//为正温度
//			} 	
			//判断温湿度是否超过测量范围（温度：-20℃~60摄氏度；湿度：20%RH~95%RH）		
			if(Humi>950) 
			{
			  Humi=950;
			}
			if(Humi<200)
			{
				Humi =200;
			}
//			if(Temprature>600)
//			{
//			  Temprature=600;
//			}
//			if(Temprature<-200)
//			{
//				Temprature = -200;
//			}
//			Temprature=Temprature/10;//计算为温度值
			Humi=Humi/10; //计算为湿度值
//			printf("\r\n温度为:  %.1f  ℃\r\n",Temprature); //显示温度
			printf("湿度为:  %.1f  %%RH\r\n",Humi);//显示湿度	
     return Humi;			
		}
		else //校验失败
		{
			printf("CRC Error !!\r\n");
			return 0;
		}
	}
	else
	{
	   printf("Sensor Error !!\r\n");
		return 0;
	}
}
/********************************************\
|* 功能：DHT12读取温湿度函数       *|
\********************************************/
//变量：Humi_H：湿度高位；Humi_L：湿度低位；Temp_H：温度高位；Temp_L：温度低位；Temp_CAL：校验位
//数据格式为：湿度高位（湿度整数）+湿度低位（湿度小数）+温度高位（温度整数）+温度低位（温度小数）+ 校验位
//校验：校验位=湿度高位+湿度低位+温度高位+温度低位
/*
void sensor_read(void)
{	
	u16 i;
	u8 Humi_H,Humi_L,Temp_H,Temp_L,Temp_CAL,temp;

	IIC_Start();    //主机发送起始信号
	IIC_Send_Byte(0Xb8);    //发送IIC地址
	if(!IIC_Wait_Ack())  //等待从机应答信号（如无应答：考虑IIC通讯频率是否太快，或者传感器接线错误）
	{
		i=0;
		IIC_Send_Byte(0);
		while(IIC_Wait_Ack())//等待从机应答信号
		{
		 	if(++i>=500)
			{
			 	break;
			}		 
		} 
		i=0;
		IIC_Start();       //主机发送起始信号
		IIC_Send_Byte(0Xb9);     //发送读指令
		while(IIC_Wait_Ack())    //等待从机应答信号
		{
		 	if(++i>=500)
			{
			 	break;
			}			 
		}
	
		Humi_H=IIC_Read_Byte(1);   //读取湿度高位
		Humi_L=IIC_Read_Byte(1);   //读取湿度低位
		Temp_H=IIC_Read_Byte(1);   //读取温度高位
		Temp_L=IIC_Read_Byte(1);   //读取温度低位
		Temp_CAL=IIC_Read_Byte(0); //读取校验位
		IIC_Stop();	   //发送停止信号	
		temp = (u8)(Humi_H+Humi_L+Temp_H+Temp_L);//只取低8位
		if(Temp_CAL==temp)//如果校验成功，往下运行
		{
			Humi=Humi_H*10+Humi_L; //湿度
	
			if(Temp_L&0X80)	//为负温度
			{
				Temprature =0-(Temp_H*10+((Temp_L&0x7F)));
			}
			else   //为正温度
			{
				Temprature=Temp_H*10+Temp_L;//为正温度
			} 	
			//判断温湿度是否超过测量范围（温度：-20℃~60摄氏度；湿度：20%RH~95%RH）		
			if(Humi>950) 
			{
			  Humi=950;
			}
			if(Humi<200)
			{
				Humi =200;
			}
			if(Temprature>600)
			{
			  Temprature=600;
			}
			if(Temprature<-200)
			{
				Temprature = -200;
			}
			Temprature=Temprature/10;//计算为温度值
			Humi=Humi/10; //计算为湿度值
			printf("\r\n温度为:  %.1f  ℃\r\n",Temprature); //显示温度
			printf("湿度为:  %.1f  %%RH\r\n",Humi);//显示湿度		   
		}
		else //校验失败
		{
			printf("CRC Error !!\r\n");
		}
	}
	else
	{
	   printf("Sensor Error !!\r\n");
	}	
	
}
*/



