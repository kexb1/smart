/**************************************************************************
*USART1:PA9 PA10  ESP8266
*DHT11: PB12    
*光照传感器：IIC  
*屏幕：IIC  A0 A1
*气体传感器：IIC  B0  B1 
*土壤适度：ADC   A3
**************************************************************************/
#include "stm32f10x.h"                  // Device header
#include "OLED.H"
#include <stdio.h>
#include <stdbool.h>
#include "Delay.h"
#include "MyUSART.H"
#include "esp.h"
#include "DHT11.H"
#include "MySPI.H"
#include "Timer.H"
#include "PWM.H"
#include "sgp30.h"

int Temperature;
int Shidu;
char Switch1=0;
char Switch2=0;
extern char RECS[200];
extern char Data[5];
extern u32 CO2Data;
uint16_t cnt;


void Init(void)
{
	uint8_t Judge=0;

	OLED_Init();
	OLED_ShowString(1,1,"Linking...");
	MyUSART_Init(); //初始化串口
	//Timer_Init();
	PWM_Init();
	do
	{
		Judge = esp_Init();
		OLED_ShowString(1,1,"error code:    ");
		OLED_ShowNum(2,1,Judge,1);
	}while(Judge);  //连接阿里云直到成功
	
	RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOA ,ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
//	GPIO_ResetBits(GPIOA,GPIO_Pin_2);//LED初始化为低电平（关）

}

int main(void)
{
	SGP30_Init();   //初始化SGP30
	Init();
	OLED_ShowString(3,1,"Switch1  Switch2");
	OLED_ShowString(1,1,"Tempera  Humidi");
	
	while(1)
	{
		cnt++;
		if(cnt%150==0)
		{
			DHT11_GetData(); //约每1s执行一次温湿度采集
			SGP30_Data_Get();
		}
			
		if(cnt==600) //约每6s执行一次数据上报
		{
			if(Esp_PUB() == 1)
			{
				OLED_ShowString(1,1,"publish failed");
				Delay_ms(500);
				OLED_Clear();
			}
			cnt=0;
		}
		
		Shidu=Data[0];
		Temperature=Data[2];
		OLED_ShowNum(2,2,Temperature,2);OLED_ShowNum(2,13,Shidu,2);
		OLED_ShowNum(4,2,Switch1,1);OLED_ShowNum(4,13,Switch2,1);
//		if(Switch1==0)
//			PWM_SetCompare(5);
//		else if(Switch1==1)
//			PWM_SetCompare(15);
//		if(Switch2==0)
//			GPIO_ResetBits(GPIOA,GPIO_Pin_2);
//		else if(Switch2==1)
//			GPIO_SetBits(GPIOA,GPIO_Pin_2);
	}
	
}

//void TIM2_IRQHandler(void)
//{
//	if(TIM_GetITStatus(TIM2,TIM_IT_Update) == SET)
//	{

//		TIM_ClearITPendingBit (TIM2,TIM_IT_Update);
//	}
//}





