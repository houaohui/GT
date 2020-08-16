#include "timer.h"
#include "key.h"

float Kp =8.0;
float KI =0.2;
float KD =0;


u16 Moto=0;

extern u8 refreoled;
extern u8 dsopsc;
extern float Frequency_vale;
extern float Targetspeed;

int Pid_Control(float Encoder,float Target)
{
	static float Bias,pwm,Last_bias;
	Bias = Encoder - Target;
	pwm += KI*(Bias-Last_bias)+Kp*Bias;
	Last_bias=Bias;
	return pwm;
}



void TIM4_Int_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); //时钟使能

	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 计数到5000为500ms
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值  10Khz的计数频率  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(  //使能或者失能指定的TIM中断
		TIM4, //TIM2
		TIM_IT_Update ,
		ENABLE  //使能
		);
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;  //TIM4中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器

	TIM_Cmd(TIM4, ENABLE);  //使能TIMx外设
							 
}

void TIM4_IRQHandler(void)   
{
	static  u8 inde=0;
	static  u8 inde1=0;
	
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否:TIM 中断源 
	{
		
		keyscan();
		if(inde==0)  //用于控制pid刷新率和oled刷新率
		{
			Moto = Pid_Control(Frequency_vale,Targetspeed);
			TIM_SetCompare1(TIM1,Moto);
			inde=10;
			refreoled=1;
			
		}
		inde--;
		if(inde1==0)  //用于控制示波器速度
		{
			inde1=30;
			dsopsc=1;
		}
		inde1--;
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update  );  //清除TIMx的中断待处理位
	}
}












