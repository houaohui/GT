#include "sys.h"
#include "pwm.h"
#include "speedmon.h"
#include "usart.h"
#include "oled.h"
#include "timer.h"
#include "delay.h"
#include "key.h"
#include "dso.h"


float Targetspeed=60;    //目标转速
float Frequency_vale;    //实时转速
float cycle=1;           //设定转速周期
u8 max=120,min=0;        //波峰和波谷数值

u8 refreoled=0;          //刷新oled使能
u8 page=1,yici=1,storbit=0,directionbit=1;   //页面索引变量page，刷新一次使能yici，负责刷新oled索引storbit,directionbit

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);   // 设置中断优先级分组0	 
	delay_init();	    	 
	uart_init(9600);
	EEROM_SpeedPIDRead();           //读取eerom中的有效信息
	KEY_Init();                     //按键扫描初始化
	TIM4_Int_Init(9,7199);          //1ms定时器中断初始化用于pid和按键和示波器
	OLED_Init();			              //初始化OLED
	Speedmon_Init();                //测速函数初始化
	TIM1_PWM_Init(65535,0);         //不分频。PWM频率=72000/(899+1)=80Khz 
	 
 
	
	
	while(1)
	{
		
		
		
		keydriver();   //按键驱动
		
		
		//oled刷新函数
		if(refreoled==1)  //使能oled刷新
		{
			
			printf("pwm :%d  ",Moto);
			printf("Currentspeed:%d.2\r\n ",(int)Frequency_vale);
			if(page==1)
			{
				if(yici)              //刷新一次
				{
					yici=0;
					OLED_Clear();
					OLED_ShowString(0,0, "Setspeed:",12);  
					OLED_ShowString(0,13, "KP:",12);  
					OLED_ShowString(64,13, "KI:",12); 
					OLED_ShowString(0,27, "KD:",12);
					OLED_ShowString(0,40, "Direction:",12);				
					OLED_ShowString(0,52, "Currentspeed:",12);

					OLED_ShowNum(80,0,Targetspeed,3,12);
					OLED_ShowFloat_Num(18,13,Kp,12);
					OLED_ShowFloat_Num(82,13,KI,12);
					OLED_ShowFloat_Num(18,27,KD,12);
					if(storbit)        //当储存以后，会更新标志位，在右上角显示S,说明上次数据储存了
					{
						storbit=0;
						OLED_ShowChar(121,0,'S',12,1);     //显示储存标志s
					}
					if(directionbit==1)   //显示正转
					{
						OLED_ShowChar(66,40,'+',12,1);
					}
					else                  //否者显示反转
					{
						OLED_ShowChar(66,40,'-',12,1);
					}
				}
				OLED_ShowFloat_Num(80,52,Frequency_vale,12);   //实时显示当前转速
				OLED_Refresh_Gram();  
				refreoled=0;

			}
			else if(page==2)//转速波形
			{
				//把PID关掉通过延时设定pwm
				if(yici)
				{
					yici=0;
					OLED_Clear();
					OLED_ShowString(0,0, "DSO_SetCycle:",16);
					OLED_ShowString(64,16, "S",16);
					OLED_ShowString(0,32, "Max:",16);
					OLED_ShowString(0,48, "Min:",16);  
					OLED_ShowFloat_Num(0,16,cycle,16);
					OLED_ShowNum(48,32, max,3,16);
					OLED_ShowNum(48,48, min,3,16);
				}
				OLED_Refresh_Gram();          
				
			}
			else   //示波器
			{
				if(yici)
				{
					yici=0;
					OLED_Clear();
					if(directionbit==1)
					{
						OLED_ShowChar(6,40,'+',12,1);
					}
					else
					{
						OLED_ShowChar(6,40,'-',12,1);
					}
					OLED_ShowNum(0,13,Targetspeed,3,12);
					OLED_ShowNum(0,52,Targetspeed,3,12);
				}
				DSO_play();               //实时刷新示波器
			}
				
				
		}
		
		
		//usart处理函数
		if(USART_RX_STA&0x8000)
		{
			u8 t;
			u8 len;
			len=USART_RX_STA&0x3fff;          //得到此次接收到的数据长度
			printf("\r\n您发送的消息为:\r\n");
			for(t=0;t<len;t++)
			{
				USART1->DR=USART_RX_BUF[t];
				while((USART1->SR&0X40)==0);    //等待发送结束
			}
			for(t=0;t<len;t++)
			{
				switch(USART_RX_BUF[t])
				{
					case 'S':Targetspeed = (USART_RX_BUF[t+1]-'0')*100+(USART_RX_BUF[t+2]-'0')*10+(USART_RX_BUF[t+3]-'0');t=t+3;break;
					case 'P':Kp =(USART_RX_BUF[t+1]-'0')+(USART_RX_BUF[t+3]-'0')*0.1;t=t+3;break;
					case 'I':KI =(USART_RX_BUF[t+1]-'0')+(USART_RX_BUF[t+3]-'0')*0.1;t=t+3;break;
					case 'D':KD =(USART_RX_BUF[t+1]-'0')+(USART_RX_BUF[t+3]-'0')*0.1;t=t+3;break;
				}
					
			}
			printf("\r\n\r\n");//插入换行
			USART_RX_STA=0;
		}
		
		
	} 
}

