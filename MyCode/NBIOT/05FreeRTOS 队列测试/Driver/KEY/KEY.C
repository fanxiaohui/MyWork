
#include "key.h"


//初始化PA0和PC13为输入口.并使能这两个口的时钟		    
//KEY IO初始化
void KEY_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC, ENABLE);	 //使能PA,PC端口时钟
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;				 //KEY1-->PC13 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 		 //下拉输入
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_Init(GPIOC, &GPIO_InitStructure);					 //根据设定参数初始化GPIOC.13
 GPIO_SetBits(GPIOC,GPIO_Pin_13);						 //PC.13 输出高

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;	    		 //KEY2->PA0 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 		 //下拉输入
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_Init(GPIOA, &GPIO_InitStructure);	  				 //推挽输出 ，IO口速度为50MHz
 GPIO_SetBits(GPIOA,GPIO_Pin_0); 						 //PA0 输入 
	
}

//按键处理函数
//返回按键值
//mode:0,不支持连续按;1,支持连续按;
//0，没有任何按键按下
//1，KEY1按下
//2，KEY2按下

//注意此函数有响应优先级,KEY1>KEY2!!
u8 KEY_Scan(u8 mode)
{	 
	static u8 key_up=1;//按键按松开标志
	
	if(mode)key_up=1;  //支持连按		  
	if(key_up&&(KEY1==0||KEY2==0))
	{
		delay_ms(10);//去抖动 
		key_up=0;
		if(KEY1==0)return KEY1_PRES;
		else if(KEY2==0)return KEY2_PRES;

	}else if(KEY1==1&&KEY2)key_up=1;
	
 	return 0;// 无按键按下
}



