


#include "ql_type.h"
#include "ql_stdlib.h"
#include "ql_trace.h"
#include "ql_gpio.h"
#include "ql_uart.h"
#include "ql_iic.h"
#include "ql_error.h"
#include "ql_system.h"

#include "custom_gpio.h"
#include "custom_iic.h"

#define I2C_SlaveAddr  0x3A  //8bit address,including the last read-write bit(0)



#define DEBUG_ENABLE 1
#if DEBUG_ENABLE > 0
#define DEBUG_PORT  UART_PORT0
#define DBG_BUF_LEN   512
static char DBG_BUFFER[DBG_BUF_LEN];
#define APP_DEBUG(FORMAT,...) {\
    Ql_memset(DBG_BUFFER, 0, DBG_BUF_LEN);\
    Ql_sprintf(DBG_BUFFER,FORMAT,##__VA_ARGS__); \
    if (UART_PORT1 == (DEBUG_PORT)) \
    {\
        Ql_Debug_Trace(DBG_BUFFER);\
    } else {\
        Ql_UART_Write((Enum_SerialPort)(DEBUG_PORT), (u8*)(DBG_BUFFER), Ql_strlen((const char *)(DBG_BUFFER)));\
    }\
}
#else
#define APP_DEBUG(FORMAT,...) 
#endif

#define I2C_RECVBUF_LENGTH  8
#define I2C_SENDBUF_LENGTH  8

#define delay_us_time 20

u8 I2C_RecvBuf[I2C_RECVBUF_LENGTH] = {0}; 
u8 I2C_SendBuf[I2C_SENDBUF_LENGTH] ={0x0F}; //register addr :0x01


#define READ_BUFFER_LENGTH 1024
static Enum_SerialPort m_myUartPort  = UART_PORT0;
static u8 m_Read_Buffer[READ_BUFFER_LENGTH];
static u8 m_buffer[100];

void Custom_GPIO_Init_IIC(void)
{
   // Specify a GPIO For IIC
   Ql_GPIO_Init(IIC_SCL, PINDIRECTION_OUT, PINLEVEL_HIGH, PINPULLSEL_PULLUP);  

  // Specify a GPIO For SDA
    Ql_GPIO_Init(IIC_SDA,PINDIRECTION_OUT, PINLEVEL_HIGH, PINPULLSEL_PULLUP);  
//    Ql_IIC_Init(0,IIC_SCL,IIC_SDA,0);
//    Ql_IIC_Config(0,TRUE, I2C_SlaveAddr, 0);
     APP_DEBUG("<-- Custom GPIO_Init IIC Fashined!!! -->\r\n");
}

void IIC_Star(void)
{
     Ql_GPIO_Init(IIC_SDA,PINDIRECTION_OUT, PINLEVEL_HIGH, PINPULLSEL_PULLUP);
 //    Ql_GPIO_Init(PINNAME_GPIO4,PINDIRECTION_OUT, PINLEVEL_HIGH, PINPULLSEL_PULLUP);
     Ql_GPIO_SetLevel(IIC_SDA,PINLEVEL_HIGH);    //SDA 1
     Ql_GPIO_SetLevel(IIC_SCL,PINLEVEL_HIGH);    //SCL 1
//     APP_DEBUG("\r\n<-- IIC Star SAD & SCL HIGH!!-->\r\n");  
 //   Ql_GPIO_Init(PINNAME_GPIO4, PINDIRECTION_OUT, PINLEVEL_HIGH, PINPULLSEL_PULLDOWN);
     Ql_Delay_us(1); 
     Ql_GPIO_SetLevel(IIC_SDA,PINLEVEL_LOW);    //SDA 0
 //    APP_DEBUG("\r\n<-- IIC Star SAD LOW!!-->\r\n"); 
 //    Ql_GPIO_SetLevel(PINNAME_GPIO4,PINLEVEL_LOW);    //SCL 0
     Ql_Delay_us(1); 
     Ql_GPIO_SetLevel(IIC_SCL,PINLEVEL_LOW);    //SCL 0
 //    APP_DEBUG("\r\n<-- IIC Star SCL LOW!!-->\r\n"); 
 //    APP_DEBUG("\r\n<--OpenCPU: IIC Star!!-->\r\n");  
}

void IIC_Stop(void)
{
//   Ql_GPIO_Init(IIC_SDA,PINDIRECTION_OUT, PINLEVEL_HIGH, PINPULLSEL_PULLUP);
    Ql_GPIO_SetLevel(IIC_SCL,PINLEVEL_LOW);   //SCL 0 
    Ql_GPIO_SetLevel(IIC_SDA,PINLEVEL_LOW);   //SDA 0
    Ql_GPIO_SetLevel(IIC_SCL,PINLEVEL_HIGH);    //SDA 1
//    Ql_Delay_us(1); 
    Ql_GPIO_SetLevel(IIC_SDA,PINLEVEL_HIGH);    //SCL 1
//    Ql_Delay_us(1); 

   
}


void IIC_Send_Ack(bool ack)
{
    Ql_GPIO_SetLevel(IIC_SCL,PINLEVEL_LOW);
    Ql_GPIO_SetLevel(IIC_SDA,ack);
    Ql_Delay_us(50);
    Ql_GPIO_SetLevel(IIC_SCL,PINLEVEL_HIGH);    //SCL 1
    Ql_Delay_us(50); 
    Ql_GPIO_SetLevel(IIC_SCL,PINLEVEL_LOW);  
}


void IIC_Ack(void)
{
    Ql_GPIO_Init(IIC_SDA,PINDIRECTION_OUT,PINLEVEL_HIGH,PINPULLSEL_PULLUP);
    Ql_GPIO_SetLevel(IIC_SCL,PINLEVEL_LOW);
    Ql_GPIO_SetLevel(IIC_SDA,PINLEVEL_LOW);
    Ql_Delay_us(1);
    Ql_GPIO_SetLevel(IIC_SCL,PINLEVEL_HIGH); 
    Ql_Delay_us(1);
    Ql_GPIO_SetLevel(IIC_SCL,PINLEVEL_LOW);
}
void IIC_Nack(void)
{
   Ql_GPIO_SetLevel(IIC_SCL,PINLEVEL_LOW);
    Ql_GPIO_Init(IIC_SDA,PINDIRECTION_OUT,PINLEVEL_HIGH,PINPULLSEL_PULLUP);
    Ql_GPIO_SetLevel(IIC_SDA,PINLEVEL_HIGH);
    Ql_Delay_us(1);
    Ql_GPIO_SetLevel(IIC_SCL,PINLEVEL_HIGH);
    Ql_Delay_us(1);
    Ql_GPIO_SetLevel(IIC_SCL,PINLEVEL_LOW);
}



void IIC_Send_byte(u8 dat)
{

    u8 i=0;
    Ql_GPIO_Init(IIC_SDA,PINDIRECTION_OUT,PINLEVEL_HIGH,PINPULLSEL_PULLUP);
    Ql_GPIO_SetLevel(IIC_SCL,PINLEVEL_LOW); 
    for(i=0;i<8;i++)
    {
       Ql_GPIO_SetLevel(IIC_SCL,PINLEVEL_LOW); 
      if((dat&0x80)>>7)
      Ql_GPIO_SetLevel(IIC_SDA,PINLEVEL_HIGH);
      else
      Ql_GPIO_SetLevel(IIC_SDA,PINLEVEL_LOW);
      dat<<=1;
      Ql_Delay_us(5);
      Ql_GPIO_SetLevel(IIC_SCL,PINLEVEL_HIGH); 
      Ql_Delay_us(5);
 //     Ql_GPIO_SetLevel(IIC_SCL,PINLEVEL_HIGH); 
 //     Ql_Delay_us(5);
 
    }
//    IIC_Recive_Ack();
    Ql_GPIO_SetLevel(IIC_SCL,PINLEVEL_LOW); 
}

u8 IIC_Wait_Ack(void)
{
    u8 Errortime =0;
    u8 ret = 0;

//    Ql_GPIO_SetLevel(IIC_SCL,PINLEVEL_LOW);   //SCL 0 
//    Ql_GPIO_SetLevel(IIC_SDA,PINLEVEL_LOW);   //SDA 1
    Ql_GPIO_Init(IIC_SDA,PINDIRECTION_IN,PINLEVEL_HIGH,PINPULLSEL_PULLUP);
//    Ql_GPIO_SetLevel(IIC_SDA,PINLEVEL_HIGH);    //SDA 0
//    Ql_GPIO_SetLevel(IIC_SCL,PINLEVEL_LOW);   //SCL 0 
//    Ql_Delay_us(3); 
//   Ql_GPIO_SetLevel(IIC_SCL,PINLEVEL_HIGH);
 //   Ql_Delay_us(3);
    while(Ql_GPIO_GetLevel(IIC_SDA))
    {
        Errortime++;
        if(Errortime>250)
        {
          IIC_Stop();
          APP_DEBUG("\r\n<--Wait Ack Fail!!-->\r\n"); 
         return 1; 
         
        }
    Ql_Delay_us(3); 
    }
    Ql_GPIO_SetLevel(IIC_SCL,PINLEVEL_LOW);   //SCL 0
    Ql_Delay_us(3); 
    Ql_GPIO_SetLevel(IIC_SDA,PINLEVEL_HIGH); 
    Ql_Delay_us(3);
    Ql_GPIO_SetLevel(IIC_SCL,PINLEVEL_LOW);  
    APP_DEBUG("\r\n<--Wait Ack OK!!-->\r\n"); 
    return 0;
 
}

u8 IIC_Read_Byte(u8 ack)
{
    u8 i,recive=0;
    Ql_GPIO_Init(IIC_SDA,PINDIRECTION_IN,PINLEVEL_HIGH,PINPULLSEL_PULLUP);
    for(i=0;i<8;i++)
    {
        Ql_GPIO_SetLevel(IIC_SCL,PINLEVEL_LOW); 
        Ql_Delay_us(50);
        Ql_GPIO_SetLevel(IIC_SCL,PINLEVEL_HIGH); 
        Ql_Delay_us(50);
        recive<<=1; 
        if(Ql_GPIO_GetLevel(IIC_SDA))  
        recive++;   
    }
    Ql_GPIO_SetLevel(IIC_SCL,PINLEVEL_LOW); 
    if(!ack)
    IIC_Nack();
    else
    IIC_Ack(); 
    return recive;
}

u8 DHT12_ReciveByte(void)
{
    u8 i=0;
    u8 dat = 0;
    Ql_GPIO_Init(IIC_SDA,PINDIRECTION_IN,PINLEVEL_HIGH,PINPULLSEL_PULLUP);
//    Ql_GPIO_SetLevel(PINNAME_GPIO5,PINLEVEL_HIGH);
    for(i=0;i<8;i++)
    {
        Ql_GPIO_SetLevel(IIC_SCL,PINLEVEL_LOW); 
        Ql_Delay_us(50);
        Ql_GPIO_SetLevel(IIC_SCL,PINLEVEL_HIGH);
        Ql_Delay_us(50);
        dat<<=1;
        dat|=Ql_GPIO_GetLevel(IIC_SDA);
        Ql_Delay_us(50);
    }

    return dat;

}
float READ_Temprature (void)
{
    u8 i;
   IIC_Send_byte(0Xb8);
    if(!IIC_Wait_Ack())
    {
        APP_DEBUG("\r\n<--Send byte 0xb8 scusseful!-->\r\n");
        i=0;
        IIC_Send_byte(0);
        while(IIC_Wait_Ack())//等待从机应答信号
        {  
            if(++i>=500)
            {
                APP_DEBUG("\r\n<--Send byte 0x00 Fail!-->\r\n");   
                break;
            }        
        }
        APP_DEBUG("\r\n<--Send byte 0x00 OK!-->\r\n"); 
        i=0;
        IIC_Star();
        IIC_Send_byte(0xb9);
        while(IIC_Wait_Ack()) 
        {
            if(++i>=500)
            {
                APP_DEBUG("\r\n<--Send byte 0Xb9 Fail!-->\r\n");   
                break;
            } 
        } 
        APP_DEBUG("\r\n<--Send byte 0Xb9 OK!-->\r\n"); 

    }
    else
    {
        APP_DEBUG("\r\n<--Send byte 0xb8 Fail!-->\r\n");   
    }
   
}

 /*   
float READ_Temprature (void)
{
    float Temprature = 0;
    u16 i;
    u8 Humi_H,Humi_L,Temp_H,Temp_L,Temp_CAL,temp;
 //   Custom_GPIO_Init_IIC();
//    IIC_Star();
    IIC_Send_byte(0Xb8);
   //发送IIC地址
    if(!IIC_Wait_Ack())  //等待从机应答信号（如无应答：考虑IIC通讯频率是否太快，或者传感器接线错误）
    {
        i=0;
        APP_DEBUG("\r\n<--Send byte 0xb8 scusseful!-->\r\n");
        IIC_Send_byte(0);
        while(IIC_Wait_Ack())//等待从机应答信号
        {
            if(++i>=500)
            {
                break;
            }        
        } 
        i=0;
        IIC_Star();       //主机发送起始信号
        IIC_Send_byte(0Xb9);     //发送读指令
        while(IIC_Wait_Ack())    //等待从机应答信号
        {
         APP_DEBUG("\r\n<--IIC_Send_byte 0Xb9 Susseful!!-->\r\n");
            if(++i>=500)
            {
                break;
            }            
        }

        Humi_H=IIC_Read_Byte(1);   //读取湿度高位
        APP_DEBUG("\r\n<--IIC_Read_Byte_Humi_H=%d!-->\r\n",Humi_H);
        Humi_L=IIC_Read_Byte(1);   //读取湿度低位
        APP_DEBUG("\r\n<--IIC_Read_Byte_Humi_L=%d!-->\r\n",Humi_L);
        Temp_H=IIC_Read_Byte(1);   //读取温度高位
        APP_DEBUG("\r\n<--IIC_Read_Byte_Temp_H=%d!-->\r\n",Temp_H);
        Temp_L=IIC_Read_Byte(1);   //读取温度低位
        APP_DEBUG("\r\n<--IIC_Read_Byte_Temp_L=%d!-->\r\n",Temp_L);
        Temp_CAL=IIC_Read_Byte(0); //读取校验位
        APP_DEBUG("\r\n<--IIC_Read_Byte_Temp_CAL=%d!-->\r\n",Temp_CAL);
        IIC_Stop();    //发送停止信号 
        temp = (u8)(Humi_H+Humi_L+Temp_H+Temp_L);//只取低8位
        APP_DEBUG("\r\n<--IIC_Read_Byte_temp=%d!-->\r\n",temp);
        if(Temp_CAL==temp)//如果校验成功，往下运行
        {
//          Humi=Humi_H*10+Humi_L; //湿度
    
            if(Temp_L&0X80) //为负温度
            {
                Temprature =0-(Temp_H*10+((Temp_L&0x7F)));
            }
            else   //为正温度
            {
                Temprature=Temp_H*10+Temp_L;//为正温度
            }   
            //判断温湿度是否超过测量范围（温度：-20℃~60摄氏度；湿度：20%RH~95%RH）       
            if(Temprature>600)
            {
              Temprature=600;
            }
            if(Temprature<-200)
            {
                Temprature = -200;
            }
            Temprature=Temprature/10;//计算为温度值
//          Humi=Humi/10; //计算为湿度值
            APP_DEBUG("\r\n<--temperature is =%0.1f!!-->\r\n",Temprature);
             return Temprature;         
        }
        else //校验失败
        {
           APP_DEBUG("\r\n<--CRC Error !!-->\r\n");
           return 0;
        }
    }
    else
    {
         APP_DEBUG("\r\n<--Sensor Error !!-->\r\n");
        return 0;
    }   
}
*/
void DHT12_READ(u8 *temp, u8 *humi)
{

    u8 i;
    u8 IIC_Read_buffer[5];
    for(i=0;i<5;i++)
    {
        IIC_Read_buffer[i]=DHT12_ReciveByte();
        if((IIC_Read_buffer[0]+IIC_Read_buffer[1]+IIC_Read_buffer[2]+IIC_Read_buffer[3])==IIC_Read_buffer[4])
        {
            *humi =IIC_Read_buffer[0];
            *temp =IIC_Read_buffer[2];
        }
        if(i==4)
            IIC_Send_Ack(1);
        else
            IIC_Send_Ack(0);
    }
    IIC_Stop();
}
void proc_reserved5(s32 taskId)
{
    u8 ret; 
    Custom_GPIO_Init_IIC();
    IIC_Star();
 // float temperature,hmidity;
 //   Ql_Sleep(20);
 ////   IIC_Stop();
  //  Ql_Sleep(20);
 //   IIC_Send_byte(0xB8);
  //  IIC_Send_byte(0X00);
  //  IIC_Send_byte(0XB9);
 //   IIC_Stop();
  //  IIC_Ack();
  //  Ql_Sleep(20);
  //  IIC_Nack();
  //  Ql_Sleep(20);
 //   for(ret=0;ret<8;ret++)
 //   {
 //    IIC_Send_byte(0Xb8);
 //    ret=IIC_Wait_Ack();
 //   }

  //  APP_DEBUG("\r\n<--Wait ack ret=%d!!-->\r\n",ret); 
 //       IIC_Send_byte(0Xb8);
  //      while(!(IIC_Wait_Ack()))
  //      {
  //        APP_DEBUG("\r\n<--Wait ack OK!-->\r\n");   
  //      }
  //      Ql_Sleep(200);
  //      APP_DEBUG("\r\n<--Wait ack Fail!-->\r\n");  
    for(;;)
    {  
    //    IIC_Star();
   //     IIC_Stop();
        IIC_Send_byte(0Xb9);
        IIC_Wait_Ack();
   //   APP_DEBUG("\r\n<--Test the satr !-->\r\n");  
   // APP_DEBUG("\r\n<--Test the satr !-->\r\n");  

   //     APP_DEBUG("\r\n<--Wait ack Fail!-->\r\n");
   //  READ_Temprature ();
     Ql_Sleep(20);
 // 
    }
}
