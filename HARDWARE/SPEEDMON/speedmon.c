#include "speedmon.h"

extern float Frequency_vale;



void TIM3_Init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseSture;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	TIM_TimeBaseSture.TIM_Period = 0xFFFF-1;
	TIM_TimeBaseSture.TIM_Prescaler = 0x00;
	TIM_TimeBaseSture.TIM_ClockDivision = 0x0;
	TIM_TimeBaseSture.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseSture);
	TIM_ETRClockMode2Config(TIM3, TIM_ExtTRGPSC_OFF, TIM_ExtTRGPolarity_NonInverted,0);
	
	TIM_SetCounter(TIM3,0);
	TIM_Cmd(TIM3,ENABLE);
	
}
	
void TIM2_Init(void)    //定时10ms
{
	TIM_TimeBaseInitTypeDef TIM2_TimeBaseSture;
	NVIC_InitTypeDef NVIC_InitSture;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	
	TIM2_TimeBaseSture.TIM_Period = 199;
	TIM2_TimeBaseSture.TIM_Prescaler = 35999;
	TIM2_TimeBaseSture.TIM_ClockDivision = 0x0;
	TIM2_TimeBaseSture.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM2_TimeBaseSture);
	TIM_ClearFlag(TIM2,TIM_FLAG_Update);
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
	
	NVIC_InitSture.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitSture.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitSture.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitSture.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitSture);
	
	TIM_Cmd(TIM2,ENABLE);

}

void GPIO_D2_Init(void)  //PD2
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);	
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;				 
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 		 
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_Init(GPIOD, &GPIO_InitStructure);					
 GPIO_SetBits(GPIOD,GPIO_Pin_2);					
}

void Speedmon_Init(void)
{
  TIM3_Init();
	TIM2_Init();
	GPIO_D2_Init();
}


void TIM2_IRQHandler(void)   //每20ms进入中断读TIM3数值，计算出转速
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update)!= RESET) 
		{
			TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
			Frequency_vale = TIM_GetCounter(TIM3)/0.2/150.0;    //求出每秒转速
			TIM_SetCounter(TIM3,0);        //把tim3归零
			
		}
}
 




