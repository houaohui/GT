#include "sys.h"
#include "pwm.h"
#include "speedmon.h"
#include "usart.h"
#include "oled.h"
#include "timer.h"
#include "delay.h"
#include "key.h"
#include "dso.h"


float Targetspeed=60;    //Ŀ��ת��
float Frequency_vale;    //ʵʱת��
float cycle=1;           //�趨ת������
u8 max=120,min=0;        //����Ͳ�����ֵ

u8 refreoled=0;          //ˢ��oledʹ��
u8 page=1,yici=1,storbit=0,directionbit=1;   //ҳ����������page��ˢ��һ��ʹ��yici������ˢ��oled����storbit,directionbit

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);   // �����ж����ȼ�����0	 
	delay_init();	    	 
	uart_init(9600);
	EEROM_SpeedPIDRead();           //��ȡeerom�е���Ч��Ϣ
	KEY_Init();                     //����ɨ���ʼ��
	TIM4_Int_Init(9,7199);          //1ms��ʱ���жϳ�ʼ������pid�Ͱ�����ʾ����
	OLED_Init();			              //��ʼ��OLED
	Speedmon_Init();                //���ٺ�����ʼ��
	TIM1_PWM_Init(65535,0);         //����Ƶ��PWMƵ��=72000/(899+1)=80Khz 
	 
 
	
	
	while(1)
	{
		
		
		
		keydriver();   //��������
		
		
		//oledˢ�º���
		if(refreoled==1)  //ʹ��oledˢ��
		{
			
			printf("pwm :%d  ",Moto);
			printf("Currentspeed:%d.2\r\n ",(int)Frequency_vale);
			if(page==1)
			{
				if(yici)              //ˢ��һ��
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
					if(storbit)        //�������Ժ󣬻���±�־λ�������Ͻ���ʾS,˵���ϴ����ݴ�����
					{
						storbit=0;
						OLED_ShowChar(121,0,'S',12,1);     //��ʾ�����־s
					}
					if(directionbit==1)   //��ʾ��ת
					{
						OLED_ShowChar(66,40,'+',12,1);
					}
					else                  //������ʾ��ת
					{
						OLED_ShowChar(66,40,'-',12,1);
					}
				}
				OLED_ShowFloat_Num(80,52,Frequency_vale,12);   //ʵʱ��ʾ��ǰת��
				OLED_Refresh_Gram();  
				refreoled=0;

			}
			else if(page==2)//ת�ٲ���
			{
				//��PID�ص�ͨ����ʱ�趨pwm
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
			else   //ʾ����
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
				DSO_play();               //ʵʱˢ��ʾ����
			}
				
				
		}
		
		
		//usart������
		if(USART_RX_STA&0x8000)
		{
			u8 t;
			u8 len;
			len=USART_RX_STA&0x3fff;          //�õ��˴ν��յ������ݳ���
			printf("\r\n�����͵���ϢΪ:\r\n");
			for(t=0;t<len;t++)
			{
				USART1->DR=USART_RX_BUF[t];
				while((USART1->SR&0X40)==0);    //�ȴ����ͽ���
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
			printf("\r\n\r\n");//���뻻��
			USART_RX_STA=0;
		}
		
		
	} 
}

