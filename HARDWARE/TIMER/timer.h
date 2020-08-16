#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"


extern float Kp;
extern float KI;
extern float KD;

extern u16 Moto;//pwm?

void Init_SysTick(void);
int Pid_Control(float Encoder,float Target);

void TIM4_Int_Init(u16 arr,u16 psc); 
 
#endif
