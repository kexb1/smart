#include "stm32f10x.h"                  // Device header

void PWM_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 ,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA ,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed  = GPIO_Speed_50MHz ;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	TIM_InternalClockConfig(TIM3);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = 100-1;  //ARR
	TIM_TimeBaseInitStructure.TIM_Prescaler = 7200 - 1; //PSC
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);
	
	TIM_OCInitTypeDef TIM_OC1InitStructure;
	TIM_OCStructInit(&TIM_OC1InitStructure);
	TIM_OC1InitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OC1InitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC1InitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OC1InitStructure.TIM_Pulse = 10; //CCR
	TIM_OC1Init(TIM3,&TIM_OC1InitStructure);
	TIM_Cmd (TIM3,ENABLE);
	
}

void PWM_SetCompare(uint8_t Compare)
{
	TIM_SetCompare1 (TIM3,Compare);
}
